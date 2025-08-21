#ifndef _LIBCAPT_CORE_PACKET_BUILDER_HPP_
#define _LIBCAPT_CORE_PACKET_BUILDER_HPP_

#include <cstdint>
#include <span>
#include "CaptPacket.hpp"

namespace Capt {
    class PacketBuilder {
    public:
        CaptPacket Packet;

        explicit PacketBuilder(uint16_t opcode) noexcept;

        void AppendByte(uint8_t value);
        void AppendUint16(uint16_t value);
        void AppendUint32(uint32_t value);
        void AppendBytes(std::span<const uint8_t> data);
    };
}

#endif
