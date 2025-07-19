#include "PacketReader.hpp"

namespace Capt {
    PacketReader::PacketReader(const CaptPacket& packet) : packet(packet), iter(packet.Payload.cbegin()) {}

    uint8_t PacketReader::ReadByte() {
        if (this->iter == packet.Payload.end()) {
            throw std::out_of_range("Packet payload EOF");
        }
        uint8_t value = *this->iter;
        this->iter++;
        return value;
    }

    uint16_t PacketReader::ReadUint16() {
        uint16_t lo = this->ReadByte();
        uint16_t hi = this->ReadByte();
        return (hi << 8) | lo;
    }

    uint32_t PacketReader::ReadUint32() {
        uint32_t value = 0;
        value |= this->ReadByte();
        value |= this->ReadByte() << 8;
        value |= this->ReadByte() << 16;
        value |= this->ReadByte() << 24;
        return value;
    }
}
