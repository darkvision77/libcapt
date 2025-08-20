#ifndef _LIBCAPT_CORE_CAPT_PACKET_HPP_
#define _LIBCAPT_CORE_CAPT_PACKET_HPP_

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

namespace Capt {
    class CaptPacket {
    public:
        uint16_t Opcode;
        std::vector<uint8_t> Payload;

        explicit CaptPacket() noexcept;
        explicit CaptPacket(uint16_t opcode) noexcept;
        explicit CaptPacket(uint16_t opcode, const std::vector<uint8_t>& payload);

        inline std::size_t Size() const noexcept {
            return this->Payload.size() + 4;
        }

        static std::ostream& WriteTo(std::ostream& stream, uint16_t opcode, std::span<const uint8_t> payload = {});
    };

    std::ostream& operator<<(std::ostream& stream, const CaptPacket& packet);
    std::istream& operator>>(std::istream& stream, CaptPacket& packet);
}

#endif
