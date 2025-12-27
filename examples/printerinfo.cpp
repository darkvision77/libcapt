#include "FileStreambuf.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include "libcapt/Protocol/PageParams.hpp"
#include "libcapt/Protocol/Protocol.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdint>
#include <span>

using namespace Capt;

static void printPrinterInfo(PrinterInfo info) {
    std::puts("Printer Info:");
    std::printf("    Type = %u\n", info.Type);
    std::printf("    Version = %u.%02u\n", info.VersionMajor, info.VersionMinor);
    std::printf("    BlockSize = %u\n", info.BlockSize);
    std::printf("    Buffers = %u\n", info.Buffers);
    std::printf("    RAM Total = %.02f MB\n", (static_cast<double>(info.BlockSize * info.Buffers) / 1024.0) / 1024.0);
}

static std::size_t fillBuffer(std::iostream& printerStream, std::size_t batchSize, std::size_t thresh = 0) {
    std::size_t count = 0;
    std::vector<uint8_t> buffer(batchSize, 0xff);
    while (true) {
        if (thresh != 0 && count >= thresh && batchSize != 1) {
            batchSize = 1;
            buffer.resize(batchSize);
        }
        BasicStatus bs = Protocol::PCR_GET_BASIC_STATUS(printerStream);
        if ((bs & BasicStatus::IM_DATA_BUSY) != 0) {
            break;
        }
        if ((bs & BasicStatus::CMD_BUSY) != 0) {
            std::puts("\nfillBuffer error: CMD_BUSY");
            break;
        }
        if ((bs & BasicStatus::ERROR_BIT) != 0) {
            std::puts("\nfillBuffer error: ERROR_BIT");
            break;
        }
        Protocol::IC_VIDEO_DATA(printerStream, buffer);
        count += buffer.size();
        std::printf("\033[2K\rFilling: %zu", count);
        std::fflush(stdout);
    }
    std::putchar('\n');
    return count;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::printf("Usage: %s printerdev\n", argv[0]);
        return 1;
    }
    FileStreambuf fs;
    if (!fs.Open(argv[1], "r+")) {
        std::puts("Failed to open printer stream");
        return 1;
    }
    std::iostream printerStream(&fs);

    PrinterInfo info = Protocol::PC_GET_PRINTER_INFO(printerStream);
    printPrinterInfo(info);

    if (Protocol::PC_RESERVE_UNIT(printerStream) != 0) {
        std::puts("Failed to reserve unit");
        return 1;
    }
    std::puts("Unit reserved");

    Protocol::PCR_CLEAR_ERROR(printerStream);
    Protocol::PCR_DISCARD_DATA(printerStream);
    Protocol::PCR_GO_ONLINE(printerStream, 0);

    ExtendedStatus status = Protocol::PC_GET_EXTENDED_STATUS(printerStream);
    if (status.PaperAvailableBits != 0) {
        std::puts("Video buffer size detection is not available: remove paper first");
        return 0;
    }

    Protocol::IC_BEGIN_DATA(printerStream);

    if ((status.Basic & BasicStatus::IM_DATA_BUSY) != 0) {
        std::puts("Video buffer size detection error: IM_DATA_BUSY");
        return 1;
    }

    PageParams dummyParams{
        .PaperSize = 0x09,
        .TonerDensity = 0x3f,
        .Mode = 0,
        .Resolution = ResolutionIdx::RES_600,
        .SmoothEnable = true,
        .TonerSaving = false,
        .MarginLeft = 1,
        .MarginTop = 1,
        .ImageLineSize = 620,
        .ImageLines = 7014,
        .PaperWidth = 4960,
        .PaperHeight = 7014,
    };
    Protocol::IC_BEGIN_PAGE(printerStream, dummyParams);

    std::puts("Filling video buffer: pass 1...");
    const std::size_t batchSize = 6144;
    const std::size_t thresh = 16 * 1024;
    std::size_t buffSize = fillBuffer(printerStream, batchSize);
    assert(buffSize > thresh);

    std::puts("Filling video buffer: pass 2...");
    Protocol::PCR_DISCARD_DATA(printerStream);
    buffSize = fillBuffer(printerStream, batchSize, buffSize - thresh);

    std::printf("Video buffer size: %zu\n", buffSize);
    return 0;
}
