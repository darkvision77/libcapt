#include "libcapt/CaptPrinter.hpp"
#include "libcapt/Compression/ScoaStreambuf.hpp"
#include "libcapt/Protocol/Enums.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include <chrono>
#include <print>
#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <thread>
#include <vector>

using namespace Capt;

static void printBasicStatus(Protocol::BasicStatus status) {
    std::println("BasicStatus = 0x{:02X}", (int)status);
    int check = 0;
    if (((int)status & 1) != 0) { // 1
        std::println("  UNKNOWN 1");
        check |= 1;
    }
    if ((status & Protocol::BasicStatus::NOT_READY) != 0) { // 2
        std::println("  NOT_READY");
        check |= 2;
    }
    if (((int)status & 4) != 0) { // 4
        std::println("  *RCF_CMD_BUSY");
        check |= 4;
    }
    if (((int)status & 8) != 0) { // 8
        std::println("  *RCF_IM_DATA_BUSY");
        check |= 8;
    }
    if ((status & Protocol::BasicStatus::OFFLINE) != 0) { // 16
        std::println("  OFFLINE");
        check |= 16;
    }
    if (((int)status & 32) != 0) { // 32
        std::println("  UNKNOWN 32");
        check |= 32;
    }
    if ((status & Protocol::BasicStatus::UNIT_FREE) != 0) { // 64
        std::println("  UNIT_FREE");
        check |= 64;
    }
    if ((status & Protocol::BasicStatus::ERROR_BIT) != 0) { // 128
        std::println("  ERROR_BIT");
        check |= 128;
    }
    if (status != check) {
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", (int)status, check);
    }
}

static void printAuxStatus(Protocol::AuxStatus status) {
    std::println("AuxStatus = 0x{:02X}", (int)status);
    int check = 0;
    if (((int)status & 1) != 0) { // 1
        std::println("  UNKNOWN 1");
        check |= 1;
    }
    if ((status & Protocol::AuxStatus::PRINTER_BUSY) != 0) { // 2
        std::println("  PRINTER_BUSY");
        check |= 2;
    }
    if ((status & Protocol::AuxStatus::PAPER_DELIVERY) != 0) { // 4
        std::println("  PAPER_DELIVERY");
        check |= 4;
    }
    if (((int)status & 8) != 0) { // 8
        std::println("  UNKNOWN 8");
        check |= 8;
    }
    if (((int)status & 16) != 0) { // 16
        std::println("  UNKNOWN 16");
        check |= 16;
    }
    if (((int)status & 32) != 0) { // 32
        std::println("  UNKNOWN 32");
        check |= 32;
    }
    if (((int)status & 64) != 0) { // 64
        std::println("  UNKNOWN 64");
        check |= 64;
    }
    if ((status & Protocol::AuxStatus::SAFE_TIMER) != 0) { // 128
        std::println("  SAFE_TIMER");
        check |= 128;
    }
    if (status != check) {
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", (int)status, check);
    }
}

static void printControllerStatus(Protocol::ControllerStatus status) {
    std::println("ControllerStatus = 0x{:02X}", (int)status);
    int check = 0;
    if ((status & Protocol::ControllerStatus::OVERRUN) != 0) { // 1
        std::println("  OVERRUN");
        check |= 1;
    }
    if ((status & Protocol::ControllerStatus::UNDERRUN) != 0) { // 2
        std::println("  UNDERRUN");
        check |= 2;
    }
    if ((status & Protocol::ControllerStatus::MISSING_EOP) != 0) { // 4
        std::println("  MISSING_EOP");
        check |= 4;
    }
    if ((status & Protocol::ControllerStatus::INVALID_DATA) != 0) { // 8
        std::println("  INVALID_DATA");
        check |= 8;
    }
    if ((status & Protocol::ControllerStatus::ENGINE_COMM_ERROR) != 0) { // 16
        std::println("  ENGINE_COMM_ERROR");
        check |= 16;
    }
    if ((status & Protocol::ControllerStatus::ENGINE_RESET_IN_PROGRESS) != 0) { // 32
        std::println("  ENGINE_RESET_IN_PROGRESS");
        check |= 32;
    }
    if ((status & Protocol::ControllerStatus::PRINT_REJECTED) != 0) { // 64
        std::println("  PRINT_REJECTED");
        check |= 64;
    }
    if (status != check) {
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", (int)status, check);
    }
}

static void printEngineStatus(Protocol::EngineReadyStatus status) {
    std::println("EngineReadyStatus = 0x{:02X}", (int)status);
    int check = 0;
    if ((status & Protocol::EngineReadyStatus::DOOR_OPEN) != 0) { // 0x4000
        std::println("  DOOR_OPEN");
        check |= 0x4000;
    }
    if ((status & Protocol::EngineReadyStatus::NO_CARTRIDGE) != 0) { // 0x2000
        std::println("  NO_CARTRIDGE");
        check |= 0x2000;
    }
    if ((status & Protocol::EngineReadyStatus::WAITING) != 0) { // 0x1000
        std::println("  WAITING");
        check |= 0x1000;
    }
    if ((status & Protocol::EngineReadyStatus::TEST_PRINTING) != 0) { // 0x400
        std::println("  TEST_PRINTING");
        check |= 0x400;
    }
    if ((status & Protocol::EngineReadyStatus::NO_PRINT_PAPER) != 0) { // 0x200
        std::println("  NO_PRINT_PAPER");
        check |= 0x200;
    }
    if ((status & Protocol::EngineReadyStatus::JAM) != 0) { // 0x100
        std::println("  JAM");
        check |= 0x100;
    }
    if ((status & Protocol::EngineReadyStatus::CLEANING) != 0) { // 4
        std::println("  CLEANING");
        check |= 4;
    }
    if ((status & Protocol::EngineReadyStatus::SERVICE_CALL) != 0) { // 2
        std::println("  SERVICE_CALL");
        check |= 2;
    }
    if ((status & Protocol::EngineReadyStatus::MIS_PRINT) != 0) { // 0x80
        std::println("  MIS_PRINT");
        check |= 0x80;
    }
    if ((status & Protocol::EngineReadyStatus::MIS_PRINT_2) != 0) { // 0x40
        std::println("  MIS_PRINT_2");
        check |= 0x40;
    }
    if (status != check) {
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", (int)status, check);
    }
}

static void printStatus(Protocol::ExtendedStatus ex) {
    printBasicStatus(ex.Basic);
    printAuxStatus(ex.Aux);
    printControllerStatus(ex.Controller);
    printEngineStatus(ex.Engine);
    std::println("Start = {}", ex.Start);
    std::println("Printing = {}", ex.Printing);
    std::println("Shipped = {}", ex.Shipped);
    std::println("Printed = {}", ex.Printed);
}

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

    bool NextPage(Protocol::PageParams& params, std::streambuf*& stream) {
        unsigned width;
        unsigned lines;
        if (!readPbmHeader(this->pbmStream, width, lines)) {
            return false;
        }
        if (width % 8 != 0) {
            throw std::runtime_error("PBM width must be a multiple of 8");
        }
        params = Protocol::PageParams{
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
        stream = this->pbmStream.rdbuf();
        return true;
    }
};

class BufferedStreambuf : public std::streambuf {
private:
    std::streambuf* source;
    std::vector<char_type>& dest;
    std::size_t blockSize;
protected:
    int_type underflow() override {
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }
        if (source == nullptr) {
            return traits_type::eof();
        }
        std::vector<char_type> buffer(this->blockSize);
        std::streamsize read = source->sgetn(buffer.data(), buffer.size());
        if (read == 0) {
            return traits_type::eof();
        }
        std::size_t offset = dest.size();
        dest.reserve(dest.size() + read);
        dest.insert(dest.end(), buffer.data(), buffer.data() + read);

        char_type* start = dest.data();
        char_type* end = start + dest.size();
        this->setg(start, start + offset, end);
        return traits_type::to_int_type(*this->gptr());
    }

public:
    explicit BufferedStreambuf(std::streambuf* source, std::vector<char_type>& dest, std::size_t blockSize = 4096)
        : source(source), dest(dest), blockSize(blockSize) {
        char_type* start = dest.data();
        char_type* end = start + dest.size();
        this->setg(start, start, end);
    }
};

Protocol::ExtendedStatus waitReady(CaptPrinter& printer) {
    Protocol::ExtendedStatus status = printer.GetStatus();
    if (status.ReadyToPrint()) {
        return status;
    }
    while (!status.ReadyToPrint()) {
        std::print("\33[2K\r");
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
        } else if ((status.Controller & Protocol::ControllerStatus::ENGINE_COMM_ERROR) != 0) {
            printer.ClearError();
        } else if ((status.Controller & Protocol::ControllerStatus::PRINT_REJECTED) != 0) {
            printer.ClearError();
        } else {
            printStatus(status);
            std::print("Not ready: unknown error");
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        status = printer.GetStatus();
    }
    std::println();
    return status;
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

    std::vector<char> prevPageBuffer;
    std::vector<char> currPageBuffer;
    unsigned page = 0;
    while (true) {
        Protocol::PageParams params;
        std::streambuf* videoStream;
        if (!prov.NextPage(params, videoStream)) {
            break;
        }
        Compression::ScoaStreambuf ss(*videoStream, params.ImageLineSize, params.ImageLines);
        bool reprintNeeded = false;
        while (true) {
            Protocol::ExtendedStatus status = printer.GetStatus();
            if (status.VideoDataError()) {
                std::println("Fatal error: page {} cannot be printed (video data error)", status.Shipped);
                return 1;
            }
            if (status.Rejected()) {
                reprintNeeded |= status.Printed + 1 != status.Start;
                printer.ClearError();
            }
            if (!status.ReadyToPrint()) {
                status = waitReady(printer);
            }
            if (reprintNeeded || !status.IsOnline()) {
                unsigned onlinePage = page == 0 ? 0 : (reprintNeeded ? (page - 1) : page);
                if (!printer.GoOnline(onlinePage)) {
                    std::println("GoOnline failed");
                    return 1;
                }
            }

            if (reprintNeeded) {
                std::print("Retrying to write page {}...", page);
                BufferedStreambuf cstr(nullptr, prevPageBuffer);
                if (printer.WritePage(params, cstr)) {
                    std::println(" OK");
                    reprintNeeded = false;
                } else {
                    std::println(" failed");
                }
            } else {
                std::print("Writing page {}...", page + 1);
                BufferedStreambuf cstr(&ss, currPageBuffer);
                if (printer.WritePage(params, cstr)) {
                    std::println(" OK");
                    break;
                }
                std::println(" failed");
                printer.WaitPrintEnd();
                status = printer.GetStatus();
                printStatus(status);
            }
        }
        prevPageBuffer.swap(currPageBuffer);
        currPageBuffer.clear();
        page++;
    }

    printer.WaitPrintEnd();
    printer.GoOffline();
    printer.ReleaseUnit();
    return 0;
}
