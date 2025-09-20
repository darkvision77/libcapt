#include "PacketHeader.hpp"
#include <stdexcept>

namespace Capt {
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

    std::ostream& PacketHeader::WriteTo(std::ostream& stream, uint16_t opcode, std::span<const uint8_t> payload) {
        if (!stream.good()) {
            return stream;
        }
        std::size_t size = 4 + payload.size();
        if (size > UINT16_MAX) {
            throw std::overflow_error("packet size overflow");
        }
        writeUint16(stream, opcode);
        writeUint16(stream, size);
        stream.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        return stream;
    }

    std::istream& operator>>(std::istream& stream, PacketHeader& header) {
        header.Opcode = readUint16(stream);
        uint16_t size = readUint16(stream);
        if (!stream.good()) {
            return stream;
        }
        if (size < 4) {
            throw std::runtime_error("impossible packet size");
        }
        header.PayloadSize = size - 4;
        return stream;
    }
}
