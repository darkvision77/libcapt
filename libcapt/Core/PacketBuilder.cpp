#include "PacketBuilder.hpp"
#include <stdexcept>

namespace Capt {
    PacketBuilder::PacketBuilder(uint16_t opcode) noexcept : Packet(opcode) {}

    void PacketBuilder::AppendByte(uint8_t value) {
        this->AppendBytes({&value, 1});
    }

    void PacketBuilder::AppendUint16(uint16_t value) {
        uint8_t buffer[2];
        buffer[0] = value & 0xff;
        buffer[1] = (value >> 8) & 0xff;
        this->AppendBytes(buffer);
    }

    void PacketBuilder::AppendUint32(uint32_t value) {
        uint8_t buffer[4];
        buffer[0] = value & 0xff;
        buffer[1] = (value >> 8) & 0xff;
        buffer[2] = (value >> 16) & 0xff;
        buffer[3] = (value >> 24) & 0xff;
        this->AppendBytes(buffer);
    }

    void PacketBuilder::AppendBytes(std::span<const uint8_t> data) {
        if ((this->Packet.Size() + data.size()) > UINT16_MAX) {
            throw std::overflow_error("Packet size overflow");
        }
        this->Packet.Payload.insert(this->Packet.Payload.end(), data.begin(), data.end());
    }
}
