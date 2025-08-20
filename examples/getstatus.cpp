#include "libcapt/Protocol/Enums.hpp"
#include "libcapt/Protocol/ExtendedStatus.hpp"
#include "libcapt/Protocol/Protocol.hpp"
#include <print>
#include <fstream>

using namespace Capt;

static void printBasicStatus(Protocol::BasicStatus status) {
    std::println("BasicStatus = 0x{:02X}", static_cast<int>(status));
    int check = 0;
    if ((static_cast<int>(status) & 1) != 0) { // 1
        std::println("  UNKNOWN 1");
        check |= 1;
    }
    if ((status & Protocol::BasicStatus::NOT_READY) != 0) { // 2
        std::println("  NOT_READY");
        check |= 2;
    }
    if ((static_cast<int>(status) & 4) != 0) { // 4
        std::println("  *RCF_CMD_BUSY");
        check |= 4;
    }
    if ((static_cast<int>(status) & 8) != 0) { // 8
        std::println("  *RCF_IM_DATA_BUSY");
        check |= 8;
    }
    if ((status & Protocol::BasicStatus::OFFLINE) != 0) { // 16
        std::println("  OFFLINE");
        check |= 16;
    }
    if ((static_cast<int>(status) & 32) != 0) { // 32
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
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", static_cast<int>(status), check);
    }
}

static void printAuxStatus(Protocol::AuxStatus status) {
    std::println("AuxStatus = 0x{:02X}", static_cast<int>(status));
    int check = 0;
    if ((static_cast<int>(status) & 1) != 0) { // 1
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
    if ((static_cast<int>(status) & 8) != 0) { // 8
        std::println("  UNKNOWN 8");
        check |= 8;
    }
    if ((static_cast<int>(status) & 16) != 0) { // 16
        std::println("  UNKNOWN 16");
        check |= 16;
    }
    if ((static_cast<int>(status) & 32) != 0) { // 32
        std::println("  UNKNOWN 32");
        check |= 32;
    }
    if ((static_cast<int>(status) & 64) != 0) { // 64
        std::println("  UNKNOWN 64");
        check |= 64;
    }
    if ((status & Protocol::AuxStatus::SAFE_TIMER) != 0) { // 128
        std::println("  SAFE_TIMER");
        check |= 128;
    }
    if (status != check) {
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", static_cast<int>(status), check);
    }
}

static void printControllerStatus(Protocol::ControllerStatus status) {
    std::println("ControllerStatus = 0x{:02X}", static_cast<int>(status));
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
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", static_cast<int>(status), check);
    }
}

static void printEngineStatus(Protocol::EngineReadyStatus status) {
    std::println("EngineReadyStatus = 0x{:02X}", static_cast<int>(status));
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
        std::println("Check mismatch: 0x{:02X} != 0x{:02X}", static_cast<int>(status), check);
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::println("Usage: {} printerdev", argv[0]);
        return 1;
    }
    std::fstream printerStream(argv[1], std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (!printerStream.is_open()) {
        std::println("Failed to open printer stream");
        return 1;
    }

    printStatus(Protocol::PC_GET_EXTENDED_STATUS(printerStream));
    return 0;
}
