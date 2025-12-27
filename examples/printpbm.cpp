#include "FileStreambuf.hpp"
#include "libcapt/Utility/BufferedPage.hpp"
#include "libcapt/BasicCaptPrinter.hpp"
#include "libcapt/Compression/ScoaStreambuf.hpp"
#include "libcapt/Protocol/Enums.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include "libcapt/Protocol/PageParams.hpp"
#include "libcapt/Protocol/ReprintStatus.hpp"
#include <atomic>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <cstdint>
#include <cctype>
#include <optional>
#include <utility>

using namespace Capt;
using namespace std::literals::chrono_literals;

class DummyStopSource {
private:
    std::atomic<bool> stopReq;
public:
    DummyStopSource() noexcept : stopReq(false) {}

    bool stop_requested() const noexcept {
        return this->stopReq.load();
    }

    bool request_stop() noexcept {
        if (!this->stopReq.exchange(true)) {
            return true;
        }
        return false;
    }
};

class DummyStopToken {
private:
    DummyStopSource* src = nullptr;
public:
    DummyStopToken() noexcept = default;
    explicit DummyStopToken(DummyStopSource& src) noexcept : src(&src) {}

    bool stop_requested() const noexcept {
        if (this->src == nullptr) {
            return false;
        }
        return this->src->stop_requested();
    }
};

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

    std::optional<PageParams> ReadHeader() {
        unsigned width;
        unsigned lines;
        if (!readPbmHeader(this->pbmStream, width, lines)) {
            return std::nullopt;
        }
        if (width % 8 != 0) {
            throw std::runtime_error("PBM width must be a multiple of 8");
        }
        return PageParams{
            .PaperSize = 0x09,
            .TonerDensity = 0x3f,
            .Mode = 0,
            .Resolution = ResolutionIdx::RES_600,
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

template<StopToken Token>
static ExtendedStatus waitReady(Token stopToken, BasicCaptPrinter<Token>& printer) {
    ExtendedStatus status = printer.GetStatus();
    if (status.Ready()) {
        return status;
    }
    while (!status.Ready() && !stopToken.stop_requested()) {
        std::printf("\033[2K\r");
        if ((status.Engine & EngineReadyStatus::DOOR_OPEN) != 0) {
            std::printf("Not ready: DOOR_OPEN");
        } else if ((status.Engine & EngineReadyStatus::NO_CARTRIDGE) != 0) {
            std::printf("Not ready: NO_CARTRIDGE");
        } else if ((status.Engine & EngineReadyStatus::WAITING) != 0) {
            std::printf("Not ready: WAITING");
        } else if ((status.Engine & EngineReadyStatus::TEST_PRINTING) != 0) {
            std::printf("Not ready: TEST_PRINTING");
        } else if ((status.Engine & EngineReadyStatus::NO_PRINT_PAPER) != 0) {
            std::printf("Not ready: NO_PRINT_PAPER");
        } else if ((status.Engine & EngineReadyStatus::JAM) != 0) {
            std::printf("Not ready: JAM");
        } else if ((status.Engine & EngineReadyStatus::CLEANING) != 0) {
            std::printf("Not ready: CLEANING");
        } else if ((status.Engine & EngineReadyStatus::SERVICE_CALL) != 0) {
            throw std::runtime_error("Unrecoverable error: SERVICE_CALL");
        } else if (status.ClearErrorNeeded()) {
            std::printf("Sending clear error...");
            printer.ClearError(&status);
        } else {
            std::printf("Not ready: unknown error");
            break;
        }
        std::this_thread::sleep_for(1s);
        status = printer.GetStatus();
    }
    std::putchar('\n');
    return status;
}

template<StopToken Token>
static void prepareBeforePrint(Token stopToken, BasicCaptPrinter<Token>& printer, unsigned page) {
    while (true) {
        ExtendedStatus status = waitReady(stopToken, printer);
        if (stopToken.stop_requested()) {
            return;
        }
        assert(status.Ready());
        if (!status.Online() || status.Start != page) {
            if (!printer.GoOnline(page)) {
                std::puts("Failed to go online, retrying...");
                std::this_thread::sleep_for(1s);
                continue;
            }
        }
        break;
    }
}

template<StopToken Token>
static bool writePage(Token stopToken, BasicCaptPrinter<Token>& printer, Utility::BufferedPage& page, Utility::BufferedPage* prev) {
    ReprintStatus reprint = ReprintStatus::None;
    while (!stopToken.stop_requested()) {
        Utility::BufferedPage& p = (prev && reprint == ReprintStatus::Prev) ? *prev : page;
        p.pubseekpos(0);
        prepareBeforePrint(stopToken, printer, p.PageNumber);
        if (stopToken.stop_requested()) {
            return true;
        }
        if (reprint != ReprintStatus::None) {
            std::printf("Retrying page %u...\n", p.PageNumber);
        } else {
            std::printf("Writing page %u...\n", p.PageNumber);
        }
        if (printer.WriteVideoData(p.Params, p)) {
            if (prev && reprint == ReprintStatus::Prev) {
                reprint = ReprintStatus::None;
                continue;
            }
            break;
        }
        auto status = printer.WaitPrintEnd(stopToken);
        if (!status) {
            return true;
        }
        if (status->VideoDataError()) {
            return false;
        }
        reprint = status->GetReprintStatus();
        assert(!status->Ready());
        std::this_thread::sleep_for(1s);
    }
    return true;
}

template<StopToken Token>
static bool waitLastPage(Token stopToken, BasicCaptPrinter<Token>& printer, Utility::BufferedPage& page) {
    while (!stopToken.stop_requested()) {
        std::this_thread::sleep_for(1s);
        auto status = printer.WaitPrintEnd(stopToken);
        if (!status) {
            return true;
        }
        if (status->VideoDataError()) {
            return false;
        } else if (status->GetReprintStatus() == ReprintStatus::None) {
            break;
        }
        if (!writePage(stopToken, printer, page, nullptr)) {
            return false;
        }
    }
    return true;
}

DummyStopSource stopSrc;

int main(int argc, char* argv[]) {
    std::setbuf(stdout, nullptr);
    if (argc != 3) {
        std::printf("Usage: %s printerdev pbmfile\n", argv[0]);
        return 1;
    }
    FileStreambuf fs;
    if (!fs.Open(argv[1], "r+")) {
        std::puts("Failed to open printer stream");
        return 1;
    }
    std::iostream printerStream(&fs);

    std::fstream pbmStream(argv[2], std::ios_base::in | std::ios_base::binary);
    if (!pbmStream.is_open()) {
        std::puts("Failed to open PBM stream");
        return 1;
    }

    std::signal(SIGINT, +[](int) {
        std::puts("Stopping...");
        stopSrc.request_stop();
    });
    DummyStopToken stopToken(stopSrc);

    PbmPageProvider prov(pbmStream);
    BasicCaptPrinter<DummyStopToken> printer(printerStream);

    printer.ReserveUnit();
    printer.ClearError();

    unsigned page = 0;
    Utility::BufferedPage prevPage;
    while (!stopToken.stop_requested()) {
        auto params = prov.ReadHeader();
        if (!params) {
            break;
        }
        Compression::ScoaStreambuf ss(*pbmStream.rdbuf(), params->ImageLineSize, params->ImageLines);
        Utility::BufferedPage currPage(page, *params, &ss);

        if (!writePage(stopToken, printer, currPage, page == 0 ? nullptr : &prevPage)) {
            std::puts("Error: WritePage failed");
            return 1;
        }
        prevPage = std::move(currPage);
        page++;
    }

    if (page != 0 && !waitLastPage(stopToken, printer, prevPage)) {
        std::puts("Error: waitLastPage failed");
        return 1;
    }

    printer.GoOffline();
    printer.ReleaseUnit();
    return 0;
}
