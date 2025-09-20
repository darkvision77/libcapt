#include "StreamPacket.hpp"
#include <cassert>

namespace Capt {
    StreamPacket::StreamPacket(std::istream& stream, PacketHeader header) noexcept
        : stream(&stream), remain(header.PayloadSize), Header(header) {}

    StreamPacket::~StreamPacket() {
        this->Discard();
    }

    StreamPacket::StreamPacket(StreamPacket&& other) noexcept
        : stream(other.stream), remain(other.remain), Header(std::move(other.Header)) {
        other.stream = nullptr;
        other.remain = 0;
    }

    StreamPacket& StreamPacket::operator=(StreamPacket&& other) noexcept {
        this->stream = other.stream;
        this->remain = other.remain;
        this->Header = std::move(other.Header);
        other.stream = nullptr;
        other.remain = 0;
        return *this;
    }

    uint8_t StreamPacket::ReadByte() {
        assert(this->stream != nullptr);
        if (this->remain == 0) {
            throw std::out_of_range("packet payload EOF");
        }
        this->remain--;
        return this->stream->get();
    }

    uint16_t StreamPacket::ReadUint16() {
        uint16_t lo = this->ReadByte();
        uint16_t hi = this->ReadByte();
        return (hi << 8) | lo;
    }

    uint32_t StreamPacket::ReadUint32() {
        uint32_t value = 0;
        value |= this->ReadByte();
        value |= this->ReadByte() << 8;
        value |= this->ReadByte() << 16;
        value |= this->ReadByte() << 24;
        return value;
    }

    void StreamPacket::ReadBytes(std::span<uint8_t> dest) {
        assert(this->stream != nullptr);
        if (dest.size() > this->remain) {
            throw std::out_of_range("packet payload EOF");
        }
        this->stream->read(reinterpret_cast<char*>(dest.data()), dest.size());
        this->remain -= dest.size();
    }

    void StreamPacket::Discard() {
        if (this->remain == 0) {
            return;
        }
        if (this->stream != nullptr && this->stream->good()) {
            this->stream->ignore(this->remain);
        }
        this->remain = 0;
    }

    std::istream& operator>>(std::istream& stream, StreamPacket& reader) {
        reader.Discard();
        stream >> reader.Header;
        reader.remain = reader.Header.PayloadSize;
        reader.stream = &stream;
        return stream;
    }
}
