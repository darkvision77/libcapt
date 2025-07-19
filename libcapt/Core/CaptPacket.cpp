#include "CaptPacket.hpp"
#include <stdexcept>

namespace Capt {
    CaptPacket::CaptPacket() noexcept {}
    CaptPacket::CaptPacket(uint16_t opcode) noexcept : Opcode(opcode) {}
    CaptPacket::CaptPacket(uint16_t opcode, std::vector<uint8_t> payload) noexcept : Opcode(opcode), Payload(payload) {}

    static void writeUint16(std::ostream& stream, uint16_t value) {
        stream.put(value & 0xff);
        stream.put((value >> 8) & 0xff);
    }

    static uint16_t readUint16(std::istream& stream) {
        uint8_t buff[2];
        stream.read(reinterpret_cast<char*>(buff), sizeof(buff));
        uint16_t lo = buff[0];
        uint16_t hi = buff[1];
        return (hi << 8) | lo;
    }

    std::size_t CaptPacket::WriteTo(std::ostream& stream) {
        std::size_t size = this->Size();
        if (size > UINT16_MAX) {
            throw std::overflow_error("Packet size overflow");
        }
        writeUint16(stream, this->Opcode);
        writeUint16(stream, size);
        stream.write(reinterpret_cast<char*>(this->Payload.data()), this->Payload.size());
        return size;
    }

    CaptPacket CaptPacket::ReadFrom(std::istream& stream) {
        uint16_t opcode = readUint16(stream);
        uint16_t size = readUint16(stream);
        std::vector<uint8_t> payload(size - 4);
        stream.read(reinterpret_cast<char*>(payload.data()), payload.size());
        return CaptPacket(opcode, payload);
    }
}
