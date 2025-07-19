#ifndef _LIBCAPT_CORE_PACKET_BUILDER_HPP_
#define _LIBCAPT_CORE_PACKET_BUILDER_HPP_

#include <cstdint>
#include "CaptPacket.hpp"

namespace Capt {
    class PacketBuilder {
    public:
        CaptPacket Packet;

        explicit PacketBuilder(uint16_t opcode);

        void AppendByte(uint8_t value);
        void AppendUint16(uint16_t value);
        void AppendUint32(uint32_t value);
        void AppendBytes(uint8_t* data, int count);
    };
}

#endif
