#include "libcapt/BufferedPage.hpp"
#include "libcapt/CaptPrinter.hpp"
#include "libcapt/Compression/ScoaStreambuf.hpp"
#include "libcapt/Protocol/Enums.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include "libcapt/Protocol/PageParams.hpp"
#include "libcapt/Protocol/ReprintStatus.hpp"
#include <cassert>
#include <iostream>
#include <print>
#include <fstream>
#include <stdexcept>
#include <thread>

using namespace Capt;
using namespace std::literals::chrono_literals;

static bool readPbmHeader(std::istream& stream, unsigned& width, unsigned& height) {
    char buffer[3];
    stream.read(buffer, sizeof(buffer));
    if (stream.eof()) {
        return false;
    }
    if (buffer[0] != 'P' || buffer[1] != '4' || !std::isspace(buffer[2])) {
        throw std::runtime_error("PBM: invalid magic");
    }
    while (stream.peek() == '#') {
        while (stream.get() != '\n') {
            if (stream.eof()) {
                throw std::runtime_error("PBM: unexpected EOF");
            }
        }
    }
    if (!(stream >> width)) {
        throw std::runtime_error("PBM: failed to read width");
    }
    if (!std::isspace(stream.get())) {
        throw std::runtime_error("PBM: unexpected char");
    }
    if (!(stream >> height)) {
        throw std::runtime_error("PBM: failed to read height");
    }
    while (!std::isspace(stream.get()) && stream.good());
    if (stream.eof()) {
        throw std::runtime_error("PBM: unexpected EOF");
    }
    return true;
}

class PbmPageProvider {
private:
    std::istream& pbmStream;
public:
    explicit PbmPageProvider(std::istream& pbmStream) : pbmStream(pbmStream) {}

    std::optional<Protocol::PageParams> ReadHeader() {
        unsigned width;
        unsigned lines;
        if (!readPbmHeader(this->pbmStream, width, lines)) {
            return std::nullopt;
        }
        if (width % 8 != 0) {
            throw std::runtime_error("PBM width must be a multiple of 8");
        }
        return Protocol::PageParams{
            .PaperSize = 0x09,
            .TonerDensity = 0x3f,
            .Mode = 0,
            .Resolution = Protocol::ResolutionIdx::RES_600,
            .SmoothEnable = true,
            .TonerSaving = false,
            .MarginLeft = 1,
            .MarginTop = 1,
            .ImageLineSize = static_cast<uint16_t>(width / 8),
            .ImageLines = static_cast<uint16_t>(lines),
            .PaperWidth = 4960,
            .PaperHeight = 7014,
        };
    }
};

static Protocol::ExtendedStatus waitReady(CaptPrinter& printer) {
    Protocol::ExtendedStatus status = printer.GetStatus();
    if (status.Ready()) {
        return status;
    }
    while (!status.Ready()) {
        std::print("\033[2K\r");
        if ((status.Engine & Protocol::EngineReadyStatus::DOOR_OPEN) != 0) {
            std::print("Not ready: DOOR_OPEN");
        } else if ((status.Engine & Protocol::EngineReadyStatus::NO_CARTRIDGE) != 0) {
            std::print("Not ready: NO_CARTRIDGE");
        } else if ((status.Engine & Protocol::EngineReadyStatus::WAITING) != 0) {
            std::print("Not ready: WAITING");
        } else if ((status.Engine & Protocol::EngineReadyStatus::TEST_PRINTING) != 0) {
            std::print("Not ready: TEST_PRINTING");
        } else if ((status.Engine & Protocol::EngineReadyStatus::NO_PRINT_PAPER) != 0) {
            std::print("Not ready: NO_PRINT_PAPER");
        } else if ((status.Engine & Protocol::EngineReadyStatus::JAM) != 0) {
            std::print("Not ready: JAM");
        } else if ((status.Engine & Protocol::EngineReadyStatus::CLEANING) != 0) {
            std::print("Not ready: CLEANING");
        } else if ((status.Engine & Protocol::EngineReadyStatus::SERVICE_CALL) != 0) {
            throw std::runtime_error("Unrecoverable error: SERVICE_CALL");
        } else if (status.ClearErrorNeeded()) {
            std::print("Sending clear error...");
            printer.ClearError(&status);
        } else {
            std::print("Not ready: unknown error");
            break;
        }
        std::this_thread::sleep_for(1s);
        status = printer.GetStatus();
    }
    std::println();
    return status;
}

static void prepareBeforePrint(CaptPrinter& printer, unsigned page) {
    Protocol::ExtendedStatus status = waitReady(printer);
    assert(status.Ready());
    if (!status.Online() || status.Start != page) {
        if (!printer.GoOnline(page)) {
            std::println("Failed to go online, retrying...");
            std::this_thread::sleep_for(1s);
            return prepareBeforePrint(printer, page);
        }
    }
}

static bool writePage(CaptPrinter& printer, BufferedPage& page, BufferedPage* prev) {
    Protocol::ReprintStatus reprint = Protocol::ReprintStatus::None;
    while (true) {
        BufferedPage& p = (prev && reprint == Protocol::ReprintStatus::Prev) ? *prev : page;
        p.ResetPos();
        prepareBeforePrint(printer, p.PageNumber);
        if (reprint != Protocol::ReprintStatus::None) {
            std::println("Retrying page {}...", p.PageNumber);
        } else {
            std::println("Writing page {}...", p.PageNumber);
        }
        if (printer.WriteVideoData(p.Params, p)) {
            if (prev && reprint == Protocol::ReprintStatus::Prev) {
                reprint = Protocol::ReprintStatus::None;
                continue;
            }
            break;
        }
        printer.WaitPrintEnd();
        Protocol::ExtendedStatus status = printer.GetStatus();
        if (status.VideoDataError()) {
            return false;
        }
        reprint = status.GetReprintStatus();
        assert(!status.Ready());
        std::this_thread::sleep_for(1s);
    }
    return true;
}

static bool waitLastPage(CaptPrinter& printer, BufferedPage& page) {
    while (true) {
        std::this_thread::sleep_for(1s);
        printer.WaitPrintEnd();
        Protocol::ExtendedStatus status = printer.GetStatus();
        if (status.VideoDataError()) {
            return false;
        } else if (status.GetReprintStatus() == Protocol::ReprintStatus::None) {
            break;
        }
        if (!writePage(printer, page, nullptr)) {
            std::println("WritePage fatal error");
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::setbuf(stdout, nullptr);
    if (argc != 3) {
        std::println("Usage: {} printerdev pbmfile", argv[0]);
        return 1;
    }
    std::fstream printerStream(argv[1], std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (!printerStream.is_open()) {
        std::println("Failed to open printer stream");
        return 1;
    }

    std::fstream pbmStream(argv[2], std::ios_base::in | std::ios_base::binary);
    if (!pbmStream.is_open()) {
        std::println("Failed to open PBM stream");
        return 1;
    }

    PbmPageProvider prov(pbmStream);
    CaptPrinter printer(printerStream);

    printer.ReserveUnit();
    printer.ClearError();

    unsigned page = 0;
    BufferedPage prevPage;
    while (true) {
        auto params = prov.ReadHeader();
        if (!params) {
            break;
        }
        Compression::ScoaStreambuf ss(*pbmStream.rdbuf(), params->ImageLineSize, params->ImageLines);
        BufferedPage currPage(page, *params, &ss);

        if (!writePage(printer, currPage, page == 0 ? nullptr : &prevPage)) {
            std::println("Error: WritePage failed");
            return 1;
        }
        prevPage = std::move(currPage);
        page++;
    }

    if (page != 0 && !waitLastPage(printer, prevPage)) {
        std::println("Error: waitLastPage failed");
        return 1;
    }

    printer.GoOffline();
    printer.ReleaseUnit();
    return 0;
}
