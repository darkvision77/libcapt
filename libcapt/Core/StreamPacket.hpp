#ifndef _LIBCAPT_CORE_STREAM_PACKET_HPP_
#define _LIBCAPT_CORE_STREAM_PACKET_HPP_

#include "PacketHeader.hpp"
#include <istream>

namespace Capt {
    class StreamPacket {
    private:
        std::istream* stream = nullptr;
        std::size_t remain = 0;
    public:
        PacketHeader Header;

        StreamPacket() noexcept = default;
        explicit StreamPacket(std::istream& stream, PacketHeader header) noexcept;
        ~StreamPacket();

        StreamPacket(const StreamPacket&) = delete;
        StreamPacket& operator=(const StreamPacket&) = delete;

        StreamPacket(StreamPacket&& other) noexcept;
        StreamPacket& operator=(StreamPacket&& other) noexcept;

        inline std::size_t Remain() const noexcept {
            return this->remain;
        }

        uint8_t ReadByte();
        uint16_t ReadUint16();
        uint32_t ReadUint32();
        void ReadBytes(std::span<uint8_t> dest);

        void Discard();

        friend std::istream& operator>>(std::istream& stream, StreamPacket& reader);
    };
};

#endif
