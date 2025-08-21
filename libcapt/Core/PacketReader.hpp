#ifndef _LIBCAPT_CORE_PACKET_READER_HPP_
#define _LIBCAPT_CORE_PACKET_READER_HPP_

#include "CaptPacket.hpp"
#include <cstdint>
#include <span>

namespace Capt {
    class PacketReader {
    private:
        const CaptPacket& packet;
        std::vector<uint8_t>::const_iterator iter;

    public:
        explicit PacketReader(const CaptPacket& packet) noexcept;

        uint8_t ReadByte();
        uint16_t ReadUint16();
        uint32_t ReadUint32();
        std::span<const uint8_t> ReadBytes(std::size_t count);
    };
}

#endif
