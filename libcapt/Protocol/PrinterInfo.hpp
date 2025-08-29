#ifndef _LIBCAPT_PROTOCOL_PRINTER_INFO_HPP_
#define _LIBCAPT_PROTOCOL_PRINTER_INFO_HPP_

#include <cstdint>

namespace Capt::Protocol {
    struct PrinterInfo {
        uint8_t DeviceId;
        uint8_t Type;
        uint8_t VersionMajor;
        uint8_t VersionMinor;
        uint16_t BlockSize;
        uint16_t Buffers;
    };
}

#endif
