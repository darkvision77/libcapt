#include "CaptPacket.hpp"
#include <stdexcept>

namespace Capt {
    CaptPacket::CaptPacket() noexcept : CaptPacket(0) {}
    CaptPacket::CaptPacket(uint16_t opcode) noexcept : Opcode(opcode) {}
    CaptPacket::CaptPacket(uint16_t opcode, const std::vector<uint8_t>& payload) : Opcode(opcode), Payload(payload) {}

    static void writeUint16(std::ostream& stream, uint16_t value) {
        if (!stream.good()) {
            return;
        }
        stream.put(value & 0xff);
        stream.put((value >> 8) & 0xff);
    }

    static uint16_t readUint16(std::istream& stream) {
        if (!stream.good()) {
            return 0;
        }
        uint8_t buff[2];
        stream.read(reinterpret_cast<char*>(buff), sizeof(buff));
        uint16_t lo = buff[0];
        uint16_t hi = buff[1];
        return (hi << 8) | lo;
    }

    std::ostream& CaptPacket::WriteTo(std::ostream& stream, uint16_t opcode, std::span<const uint8_t> payload) {
        if (!stream.good()) {
            return stream;
        }
        std::size_t size = 4 + payload.size();
        if (size > UINT16_MAX) {
            throw std::overflow_error("Packet size overflow");
        }
        writeUint16(stream, opcode);
        writeUint16(stream, size);
        stream.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const CaptPacket& packet) {
        if (!stream.good()) {
            return stream;
        }
        std::size_t size = packet.Size();
        if (size > UINT16_MAX) {
            throw std::overflow_error("Packet size overflow");
        }
        writeUint16(stream, packet.Opcode);
        writeUint16(stream, size);
        stream.write(reinterpret_cast<const char*>(packet.Payload.data()), packet.Payload.size());
        return stream;
    }

    std::istream& operator>>(std::istream& stream, CaptPacket& packet) {
        if (!stream.good()) {
            return stream;
        }
        packet.Opcode = readUint16(stream);
        uint16_t size = readUint16(stream);
        if (size < 4) {
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        packet.Payload = std::vector<uint8_t>(size - 4);
        stream.read(reinterpret_cast<char*>(packet.Payload.data()), packet.Payload.size());
        return stream;
    }
}
