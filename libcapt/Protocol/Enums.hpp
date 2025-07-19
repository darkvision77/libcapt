#ifndef _LIBCAPT_PROTOCOL_ENUMS_HPP_
#define _LIBCAPT_PROTOCOL_ENUMS_HPP_

#include <cstdint>

namespace Capt::Protocol {
    enum ResolutionIdx : uint8_t {
        RES_300 = 0,
        RES_600 = 0x11,
    };

    enum BasicStatus : uint8_t {
        NOT_READY = 2,
        CMD_BUSY = 4,
        IM_DATA_BUSY = 8,
        OFFLINE = 16,
        UNIT_FREE = 64,
        ERROR_BIT = 128,
    };

    enum AuxStatus : uint8_t {
        PRINTER_BUSY = 2,
        PAPER_DELIVERY = 4,
        SAFE_TIMER = 128
    };

    enum ControllerStatus : uint8_t {
        ENGINE_RESET_IN_PROGRESS = 0x20,
        INVALID_DATA = 8,
        MISSING_EOP = 4,
        UNDERRUN = 2,
        OVERRUN = 1,
        ENGINE_COMM_ERROR = 0x10, // highly likely ENGINE_COMMAND_ERROR
        PRINT_REJECTED = 0x40,
    };

    enum EngineReadyStatus : uint16_t {
        DOOR_OPEN = 0x4000,
        NO_CARTRIDGE = 0x2000,
        WAITING = 0x1000,
        TEST_PRINTING = 0x400,
        NO_PRINT_PAPER = 0x200,
        JAM = 256,
        CLEANING = 4,
        SERVICE_CALL = 2,

        MIS_PRINT = 128,
        MIS_PRINT_2 = 64,
    };
}

#endif
