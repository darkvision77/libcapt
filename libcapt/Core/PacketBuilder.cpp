#include "PacketBuilder.hpp"
#include <stdexcept>

namespace Capt {
    static const int MaxPacketSize = UINT16_MAX;

    PacketBuilder::PacketBuilder(uint16_t opcode) : Packet(CaptPacket(opcode)) {}

    void PacketBuilder::AppendByte(uint8_t value) {
        this->AppendBytes(&value, 1);
    }

    void PacketBuilder::AppendUint16(uint16_t value) {
        uint8_t buffer[2];
        buffer[0] = value & 0xff;
        buffer[1] = (value >> 8) & 0xff;
        this->AppendBytes(buffer, sizeof(buffer));
    }

    void PacketBuilder::AppendUint32(uint32_t value) {
        uint8_t buffer[4];
        buffer[0] = value & 0xff;
        buffer[1] = (value >> 8) & 0xff;
        buffer[2] = (value >> 16) & 0xff;
        buffer[3] = (value >> 24) & 0xff;
        this->AppendBytes(buffer, sizeof(buffer));
    }

    void PacketBuilder::AppendBytes(uint8_t* data, std::size_t count) {
        if ((this->Packet.Size() + count) > MaxPacketSize) {
            throw std::overflow_error("Packet size overflow");
        }
        this->Packet.Payload.insert(this->Packet.Payload.end(), data, data + count);
    }
}
