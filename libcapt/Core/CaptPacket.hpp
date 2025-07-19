#ifndef _LIBCAPT_CORE_CAPT_PACKET_HPP_
#define _LIBCAPT_CORE_CAPT_PACKET_HPP_

#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <vector>

namespace Capt {
    class CaptPacket {
    public:
        uint16_t Opcode;
        std::vector<uint8_t> Payload;

        explicit CaptPacket() noexcept;
        explicit CaptPacket(uint16_t opcode) noexcept;
        explicit CaptPacket(uint16_t opcode, std::vector<uint8_t> payload) noexcept;

        inline std::size_t Size() const noexcept {
            return this->Payload.size() + 4;
        }

        std::size_t WriteTo(std::ostream& stream);
        static CaptPacket ReadFrom(std::istream& stream);
    };
}

#endif
