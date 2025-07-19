#include "DecoderStreambuf.hpp"
#include "DecoderError.hpp"
#include <cassert>
#include <format>
#include <streambuf>

namespace Capt::Compression {
    #define CLOG(...) if (this->commandLog) *this->commandLog << std::format(__VA_ARGS__) << std::flush
    #define CHECK_BOUNDS(PARAM, MIN, MAX) checkBounds(PARAM, MIN, MAX, #PARAM)
    inline static void checkBounds(int value, int min, int max, std::string paramName) {
        if (value < min || value > max) {
            throw DecoderError(std::format("{} must be in range [{};{}], got {}", paramName, min, max, value));
        }
    }

    inline static uint8_t mustGet(std::streambuf& stream) {
        std::streambuf::int_type v = stream.sbumpc();
        if (v == std::streambuf::traits_type::eof()) {
            throw DecoderError("unexpected EOF");
        }
        return std::streambuf::traits_type::to_char_type(v);
    }

    inline static std::streamsize mustRead(std::streambuf& stream, std::streambuf::char_type* s, std::streamsize n) {
        std::streamsize read = stream.sgetn(s, n);
        if (read != n) {
            throw DecoderError("unexpected EOF");
        }
        return read;
    }

    DecoderStreambuf::DecoderStreambuf(std::streambuf& reader, unsigned lineSize, std::ostream* commandLog)
        : reader(reader), commandLog(commandLog), lineSize(lineSize), videoSize(0) {}

    void DecoderStreambuf::repeat(unsigned count, uint8_t repeatByte) {
        if (count == 0) {
            return;
        }
        this->buffer.insert(this->buffer.end(), count, repeatByte);
    }

    void DecoderStreambuf::repeatX(unsigned count) {
        this->repeat(count, 0x43);
    }

    void DecoderStreambuf::copy(unsigned count) {
        if (count == 0) {
            return;
        }
        if (this->buffer.size() < this->lineSize) {
            throw DecoderError("insufficient buffer size for copying");
        }
        auto start = this->buffer.end() - this->lineSize;
        this->buffer.insert(this->buffer.end(), start, start + count);
    }

    void DecoderStreambuf::raw(const std::vector<uint8_t>& data) {
        if (data.size() == 0) {
            return;
        }
        this->buffer.insert(this->buffer.end(), data.begin(), data.end());
    }

    void DecoderStreambuf::endLine() {
        unsigned count = this->lineSize - (this->buffer.size() % this->lineSize);
        this->copy(count);
    }

    int DecoderStreambuf::decodeNext() {
        uint8_t cmd = mustGet(this->reader);
        if (cmd == 0x42) { // EOP
            CLOG("EOP\n");
            return 0;
        } else if (cmd == 0x41) { // EOL
            CLOG("EOL\n");
            this->endLine();
            return 1;
        } else if (cmd == 0x40) { // NOP
            CLOG("NOP\n");
            return 1;
        } else if ((cmd & 0xe0) == 0xa0) { // Extend
            CLOG("Extend\n");
            uint8_t extendCount = (cmd & 0x1f) << 3;
            cmd = mustGet(this->reader);
            if ((cmd & 0xc0) == 0) { // RepeatXLong
                CLOG("RepeatXLong\n");
                uint8_t repeatCount = extendCount + ((cmd >> 3) & 0x07);
                CHECK_BOUNDS(repeatCount, 8, 255);
                this->repeatX(repeatCount);
                return 2;
            } else if ((cmd & 0xc0) == 0x80) { // CopyThenRepeatLong
                CLOG("CopyThenRepeatLong\n");
                uint8_t copyCount = (cmd & 0x07);
                uint8_t repeatCount = extendCount + ((cmd >> 3) & 0x07);
                uint8_t repeatByte = mustGet(this->reader);
                CHECK_BOUNDS(copyCount, 0, 7);
                CHECK_BOUNDS(repeatCount, 8, 255);
                this->copy(copyCount);
                this->repeat(repeatCount, repeatByte);
                return 3;
            } else if ((cmd & 0xc0) == 0x40) { // RepeatThenRawLong
                CLOG("RepeatThenRawLong\n");
                uint8_t rawCount = extendCount + (cmd & 0x07);
                uint8_t repeatCount = ((cmd >> 3) & 0x07);
                uint8_t repeatByte = mustGet(this->reader);
                std::vector<uint8_t> rawData(rawCount);
                mustRead(this->reader, reinterpret_cast<char*>(rawData.data()), rawCount);
                CHECK_BOUNDS(rawCount, 8, 255);
                CHECK_BOUNDS(repeatCount, 2, 7);
                this->repeat(repeatCount, repeatByte);
                this->raw(rawData);
                return 3 + rawCount;
            } else if ((cmd & 0xc0) == 0xc0) { // CopyThenRawLong
                CLOG("CopyThenRawLong\n");
                uint8_t copyCount = (cmd & 0x07);
                uint8_t rawCount = extendCount + ((cmd >> 3) & 0x07);
                std::vector<uint8_t> rawData(rawCount);
                mustRead(this->reader, reinterpret_cast<char*>(rawData.data()), rawCount);
                CHECK_BOUNDS(rawCount, 8, 255);
                CHECK_BOUNDS(copyCount, 0, 7);
                this->copy(copyCount);
                this->raw(rawData);
                return 2 + rawCount;
            }
        } else if ((cmd & 0xe0) == 0x80) { // CopyLong
            CLOG("CopyLong\n");
            uint8_t copyCount = (cmd & 0x1f) << 3;
            CHECK_BOUNDS(copyCount, 8, 248);
            this->copy(copyCount);
            return 1;
        } else if ((cmd & 0xc0) == 0x40) { // CopyThenRepeat
            CLOG("CopyThenRepeat\n");
            uint8_t copyCount = (cmd & 0x07);
            uint8_t repeatCount = ((cmd >> 3) & 0x07);
            uint8_t repeatByte = mustGet(this->reader);
            CHECK_BOUNDS(copyCount, 0, 7);
            CHECK_BOUNDS(repeatCount, 0, 7);
            if (repeatCount == 0 && copyCount <= 2) {
                // actually unreachable
                throw DecoderError("command collision");
            }
            this->copy(copyCount);
            this->repeat(repeatCount, repeatByte);
            return 2;
        } else if ((cmd & 0xc0) == 0) { // CopyThenRaw
            CLOG("CopyThenRaw\n");
            uint8_t copyCount = (cmd & 0x07);
            uint8_t rawCount = ((cmd >> 3) & 0x07);
            std::vector<uint8_t> rawData(rawCount);
            mustRead(this->reader, reinterpret_cast<char*>(rawData.data()), rawCount);
            CHECK_BOUNDS(copyCount, 0, 7);
            CHECK_BOUNDS(rawCount, 1, 7);
            this->copy(copyCount);
            this->raw(rawData);
            return 1 + rawCount;
        } else if ((cmd & 0xc0) == 0xc0) { // RepeatX | CopyShort | RepeatThenRaw
            uint8_t copyCount = (cmd & 0x07);
            uint8_t repeatCount = ((cmd >> 3) & 0x07);
            if (copyCount != 0 && repeatCount == 0) { // CopyShort
                CLOG("CopyShort\n");
                this->copy(copyCount);
                return 1;
            } else if (copyCount == 0 && repeatCount != 0) { // RepeatX
                CLOG("RepeatX\n");
                CHECK_BOUNDS(repeatCount, 1, 7);
                this->repeatX(repeatCount);
                return 1;
            } else if (copyCount != 0 && repeatCount != 0) { // RepeatThenRaw
                CLOG("RepeatThenRaw\n");
                uint8_t rawCount = copyCount;
                uint8_t repeatByte = mustGet(this->reader);
                std::vector<uint8_t> rawData(rawCount);
                mustRead(this->reader, reinterpret_cast<char*>(rawData.data()), rawCount);
                CHECK_BOUNDS(repeatCount, 2, 7);
                CHECK_BOUNDS(rawCount, 1, 7);
                this->repeat(repeatCount, repeatByte);
                this->raw(rawData);
                return 2 + rawCount;
            }
        }
        throw DecoderError(std::format("unknown command: {:#x}", cmd));
    }

    bool DecoderStreambuf::decodeLine() {
        unsigned start = this->buffer.size();
        while (this->buffer.size() < start + this->lineSize) {
            int c = this->decodeNext();
            this->videoSize += c;
            if (c == 0) {
                if (((this->videoSize + 1) % 2) != 0) {
                    throw DecoderError("video size must be even");
                }
                this->videoSize = 0;
                return false;
            }
        }
        return true;
    }

    DecoderStreambuf::int_type DecoderStreambuf::underflow() {
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }

        if (!this->decodeLine()) {
            return traits_type::eof();
        }

        assert(this->buffer.size() >= this->lineSize);
        char_type* start = reinterpret_cast<char_type*>(this->buffer.data() + this->buffer.size() - this->lineSize);
        this->setg(start, start, start + this->lineSize);
        return traits_type::to_int_type(*this->gptr());
    }
}
