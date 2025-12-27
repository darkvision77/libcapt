#include "FileStreambuf.hpp"
#include "libcapt/Protocol/Enums.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include "libcapt/Protocol/Protocol.hpp"
#include <iostream>
#include <cstdio>

using namespace Capt;

static void printBasicStatus(BasicStatus status) {
    std::printf("BasicStatus = 0x%02X\n", static_cast<int>(status));
    int check = 0;
    if ((static_cast<int>(status) & 1) != 0) { // 1
        std::puts("  UNKNOWN 1");
        check |= 1;
    }
    if ((status & BasicStatus::NOT_READY) != 0) { // 2
        std::puts("  NOT_READY");
        check |= 2;
    }
    if ((static_cast<int>(status) & 4) != 0) { // 4
        std::puts("  *RCF_CMD_BUSY");
        check |= 4;
    }
    if ((static_cast<int>(status) & 8) != 0) { // 8
        std::puts("  *RCF_IM_DATA_BUSY");
        check |= 8;
    }
    if ((status & BasicStatus::OFFLINE) != 0) { // 16
        std::puts("  OFFLINE");
        check |= 16;
    }
    if ((static_cast<int>(status) & 32) != 0) { // 32
        std::puts("  UNKNOWN 32");
        check |= 32;
    }
    if ((status & BasicStatus::UNIT_FREE) != 0) { // 64
        std::puts("  UNIT_FREE");
        check |= 64;
    }
    if ((status & BasicStatus::ERROR_BIT) != 0) { // 128
        std::puts("  ERROR_BIT");
        check |= 128;
    }
    if (status != check) {
        std::printf("Check mismatch: 0x%02X != 0x%02X\n", static_cast<int>(status), check);
    }
}

static void printAuxStatus(AuxStatus status) {
    std::printf("AuxStatus = 0x%02X\n", static_cast<int>(status));
    int check = 0;
    if ((static_cast<int>(status) & 1) != 0) { // 1
        std::puts("  UNKNOWN 1");
        check |= 1;
    }
    if ((status & AuxStatus::PRINTER_BUSY) != 0) { // 2
        std::puts("  PRINTER_BUSY");
        check |= 2;
    }
    if ((status & AuxStatus::PAPER_DELIVERY) != 0) { // 4
        std::puts("  PAPER_DELIVERY");
        check |= 4;
    }
    if ((static_cast<int>(status) & 8) != 0) { // 8
        std::puts("  UNKNOWN 8");
        check |= 8;
    }
    if ((static_cast<int>(status) & 16) != 0) { // 16
        std::puts("  UNKNOWN 16");
        check |= 16;
    }
    if ((static_cast<int>(status) & 32) != 0) { // 32
        std::puts("  UNKNOWN 32");
        check |= 32;
    }
    if ((static_cast<int>(status) & 64) != 0) { // 64
        std::puts("  UNKNOWN 64");
        check |= 64;
    }
    if ((status & AuxStatus::SAFE_TIMER) != 0) { // 128
        std::puts("  SAFE_TIMER");
        check |= 128;
    }
    if (status != check) {
        std::printf("Check mismatch: 0x%02X != 0x%02X\n", static_cast<int>(status), check);
    }
}

static void printControllerStatus(ControllerStatus status) {
    std::printf("ControllerStatus = 0x%02X\n", static_cast<int>(status));
    int check = 0;
    if ((status & ControllerStatus::OVERRUN) != 0) { // 1
        std::puts("  OVERRUN");
        check |= 1;
    }
    if ((status & ControllerStatus::UNDERRUN) != 0) { // 2
        std::puts("  UNDERRUN");
        check |= 2;
    }
    if ((status & ControllerStatus::MISSING_EOP) != 0) { // 4
        std::puts("  MISSING_EOP");
        check |= 4;
    }
    if ((status & ControllerStatus::INVALID_DATA) != 0) { // 8
        std::puts("  INVALID_DATA");
        check |= 8;
    }
    if ((status & ControllerStatus::ENGINE_COMM_ERROR) != 0) { // 16
        std::puts("  ENGINE_COMM_ERROR");
        check |= 16;
    }
    if ((status & ControllerStatus::ENGINE_RESET_IN_PROGRESS) != 0) { // 32
        std::puts("  ENGINE_RESET_IN_PROGRESS");
        check |= 32;
    }
    if ((status & ControllerStatus::PRINT_REJECTED) != 0) { // 64
        std::puts("  PRINT_REJECTED");
        check |= 64;
    }
    if (status != check) {
        std::printf("Check mismatch: 0x%02X != 0x%02X\n", static_cast<int>(status), check);
    }
}

static void printEngineStatus(EngineReadyStatus status) {
    std::printf("EngineReadyStatus = 0x%02X\n", static_cast<int>(status));
    int check = 0;
    if ((status & EngineReadyStatus::DOOR_OPEN) != 0) { // 0x4000
        std::puts("  DOOR_OPEN");
        check |= 0x4000;
    }
    if ((status & EngineReadyStatus::NO_CARTRIDGE) != 0) { // 0x2000
        std::puts("  NO_CARTRIDGE");
        check |= 0x2000;
    }
    if ((status & EngineReadyStatus::WAITING) != 0) { // 0x1000
        std::puts("  WAITING");
        check |= 0x1000;
    }
    if ((status & EngineReadyStatus::TEST_PRINTING) != 0) { // 0x400
        std::puts("  TEST_PRINTING");
        check |= 0x400;
    }
    if ((status & EngineReadyStatus::NO_PRINT_PAPER) != 0) { // 0x200
        std::puts("  NO_PRINT_PAPER");
        check |= 0x200;
    }
    if ((status & EngineReadyStatus::JAM) != 0) { // 0x100
        std::puts("  JAM");
        check |= 0x100;
    }
    if ((status & EngineReadyStatus::CLEANING) != 0) { // 4
        std::puts("  CLEANING");
        check |= 4;
    }
    if ((status & EngineReadyStatus::SERVICE_CALL) != 0) { // 2
        std::puts("  SERVICE_CALL");
        check |= 2;
    }
    if ((status & EngineReadyStatus::MIS_PRINT) != 0) { // 0x80
        std::puts("  MIS_PRINT");
        check |= 0x80;
    }
    if ((status & EngineReadyStatus::MIS_PRINT_2) != 0) { // 0x40
        std::puts("  MIS_PRINT_2");
        check |= 0x40;
    }
    if (status != check) {
        std::printf("Check mismatch: 0x%02X != 0x:%02X\n", static_cast<int>(status), check);
    }
}

static void printStatus(ExtendedStatus ex) {
    printBasicStatus(ex.Basic);
    printAuxStatus(ex.Aux);
    printControllerStatus(ex.Controller);
    printEngineStatus(ex.Engine);
    std::printf("Start = %u\n", ex.Start);
    std::printf("Printing = %u\n", ex.Printing);
    std::printf("Shipped = %u\n", ex.Shipped);
    std::printf("Printed = %u\n", ex.Printed);
    std::printf("Paper = 0x%x\n", ex.PaperAvailableBits);
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

    printStatus(Protocol::PC_GET_EXTENDED_STATUS(printerStream));
    return 0;
}
