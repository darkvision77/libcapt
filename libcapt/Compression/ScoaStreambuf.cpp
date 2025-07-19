#include "ScoaStreambuf.hpp"
#include "ScoaCmd.hpp"
#include <cassert>
#include <iostream>

namespace Capt::Compression {
    using int_type = ScoaStreambuf::int_type;

    ScoaStreambuf::ScoaStreambuf(std::streambuf& rasterStream, unsigned lineSize, unsigned lines)
        : rasterStream(&rasterStream), state(lineSize), linesRemain(lines) {
        char_type* start = reinterpret_cast<char_type*>(this->buffer.data());
        char_type* end = start + this->buffer.size();
        this->setg(start, end, end);
    }

    std::size_t cmd_Copy(std::vector<uint8_t>& buffer, int copyCount) {
        if (copyCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        while (copyCount >= 8) {
            int copy = std::min(copyCount, 255);
            copy = copy - (copy % 8);
            vsize += ScoaCmd::CopyLong(buffer, copy);
            copyCount -= copy;
        }
        assert(copyCount <= 7);
        if (copyCount != 0) {
            vsize += ScoaCmd::CopyShort(buffer, copyCount);
        }
        return vsize;
    }

    std::size_t cmd_CopyThenRaw(std::vector<uint8_t>& buffer, int copyCount, const uint8_t* rawData, int rawCount) {
        if (copyCount == 0 && rawCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (copyCount <= 7) {
            while (rawCount >= 8) {
                int count = std::min(rawCount, 255);
                vsize += ScoaCmd::CopyThenRawLong(buffer, copyCount, rawData, count);
                copyCount = 0;
                rawData += count;
                rawCount -= count;
            }
            if (copyCount != 0 || rawCount != 0) {
                vsize += ScoaCmd::CopyThenRaw(buffer, copyCount, rawData, rawCount);
            }
        } else {
            while (copyCount >= 8) {
                int copy = std::min(copyCount, 248);
                copy = copy - (copy % 8);
                vsize += ScoaCmd::CopyLong(buffer, copy);
                copyCount -= copy;
            }
            return vsize + cmd_CopyThenRaw(buffer, copyCount, rawData, rawCount);
        }
        return vsize;
    }

    std::size_t cmd_WriteRaw(std::vector<uint8_t>& buffer, const uint8_t* rawData, int rawCount) {
        if (rawCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (rawCount <= 7) {
            vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData, rawCount);
        } else {
            while (rawCount >= 8) {
                int count = std::min(rawCount, 255);
                vsize += ScoaCmd::CopyThenRawLong(buffer, 0, rawData, count);
                rawData += count;
                rawCount -= count;
            }
            if (rawCount != 0) {
                vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData, rawCount);
            }
        }
        return vsize;
    }

    std::size_t cmd_RepeatThenRaw(std::vector<uint8_t>& buffer, int repeatCount, uint8_t repeatByte, const uint8_t* rawData, int rawCount) {
        if (repeatCount == 0 && rawCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (repeatCount <= 7) {
            if (rawCount <= 7) {
                if (rawCount == 0) {
                    vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                } else if (repeatCount == 0) {
                    vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData, rawCount);
                } else {
                    if (repeatCount >= 2) {
                        vsize += ScoaCmd::RepeatThenRaw(buffer, repeatCount, repeatByte, rawData, rawCount);
                    } else {
                        vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                        vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData, rawCount);
                    }
                }
            } else {
                if (repeatCount >= 2) {
                    vsize += ScoaCmd::RepeatThenRaw(buffer, repeatCount, repeatByte, rawData, 7);
                    rawData += 7;
                    rawCount -= 7;
                } else if (repeatCount != 0) {
                    vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                }
                while (rawCount >= 8) {
                    int count = std::min(rawCount, 255);
                    vsize += ScoaCmd::CopyThenRawLong(buffer, 0, rawData, count);
                    rawData += count;
                    rawCount -= count;
                }
                if (rawCount != 0) {
                    vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData, rawCount);
                }
            }
        } else {
            while (repeatCount >= 8) {
                int count = std::min(repeatCount, 255);
                vsize += ScoaCmd::CopyThenRepeatLong(buffer, 0, count, repeatByte);
                repeatCount -= count;
            }
            return vsize + cmd_RepeatThenRaw(buffer, repeatCount, repeatByte, rawData, rawCount);
        }
        return vsize;
    }

    std::size_t cmd_CopyThenRepeat(std::vector<uint8_t>& buffer, int copyCount, int repeatCount, uint8_t repeatByte) {
        if (repeatCount == 0 && copyCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (copyCount <= 7) {
            while (repeatCount >= 8) {
                int rep = std::min(repeatCount, 255);
                vsize += ScoaCmd::CopyThenRepeatLong(buffer, copyCount, rep, repeatByte);
                copyCount = 0;
                repeatCount -= rep;
            }
            if (copyCount != 0 || repeatCount != 0) {
                vsize += ScoaCmd::CopyThenRepeat(buffer, copyCount, repeatCount, repeatByte);
            }
        } else {
            while (copyCount >= 8) {
                int copy = std::min(copyCount, 248);
                copy = copy - (copy % 8);
                vsize += ScoaCmd::CopyLong(buffer, copy);
                copyCount -= copy;
            }
            return vsize + cmd_CopyThenRepeat(buffer, copyCount, repeatCount, repeatByte);
        }
        return vsize;
    }

    std::size_t ScoaStreambuf::encodeLine(const std::vector<uint8_t>& line) {
        std::size_t encodedSize = 0;
        assert(line.size() == state.LineSize);
        for (unsigned i = 0; i < state.LineSize; i++) {
            if (state.Copy[i] == state.LineSize - i) {
                encodedSize += ScoaCmd::EOL(this->buffer);
                break;
            }
            if (state.Raw[i] != 0) {
                unsigned nextPos = i + state.Raw[i];
                std::vector<uint8_t> rawData(line.data() + i, line.data() + i + state.Raw[i]);
                assert(rawData.size() == state.Raw[i]);
                encodedSize += cmd_WriteRaw(this->buffer, rawData.data(), rawData.size());
                i = nextPos - 1;
                continue;
            } else if (state.Copy[i] != 0) {
                unsigned nextPos = i + state.Copy[i];
                assert(nextPos < state.LineSize);
                if (state.Raw[nextPos] != 0) {
                    std::vector<uint8_t> rawData(line.data() + nextPos, line.data() + nextPos + state.Raw[nextPos]);
                    assert(rawData.size() == state.Raw[nextPos]);
                    encodedSize += cmd_CopyThenRaw(this->buffer, state.Copy[i], rawData.data(), rawData.size());
                    nextPos += state.Raw[nextPos];
                } else if (state.Repeat[nextPos] != 1) {
                    encodedSize += cmd_CopyThenRepeat(this->buffer, state.Copy[i], state.Repeat[nextPos], line[nextPos]);
                    nextPos += state.Repeat[nextPos];
                } else {
                    encodedSize += cmd_Copy(this->buffer, state.Copy[i]);
                }
                i = nextPos - 1;
                continue;
            } else if (state.Repeat[i] != 1) {
                unsigned nextPos = i + state.Repeat[i];
                if (nextPos != state.LineSize && state.Raw[nextPos] != 0) {
                    std::vector<uint8_t> rawData(line.data() + nextPos, line.data() + nextPos + state.Raw[nextPos]);
                    assert(rawData.size() == state.Raw[nextPos]);
                    encodedSize += cmd_RepeatThenRaw(this->buffer, state.Repeat[i], line[i], rawData.data(), rawData.size());
                    nextPos += state.Raw[nextPos];
                } else {
                    encodedSize += cmd_RepeatThenRaw(this->buffer, state.Repeat[i], line[i], nullptr, 0);
                }
                i = nextPos - 1;
                continue;
            }
            assert(false);
        }
        return encodedSize;
    }

    int_type ScoaStreambuf::underflow() {
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }
        if (this->linesRemain == 0) {
            return traits_type::eof();
        }

        std::vector<uint8_t> lineBuffer(this->state.LineSize);
        if (this->rasterStream->sgetn(reinterpret_cast<char*>(lineBuffer.data()), lineBuffer.size()) == 0) {
            return traits_type::eof();
        }

        this->buffer.clear();
        this->state.ProcessLine(lineBuffer);
        this->videoSize += this->encodeLine(lineBuffer);

        this->linesRemain--;
        if (this->linesRemain == 0) {
            if ((this->videoSize + 1) % 2 != 0) {
                ScoaCmd::NOP(this->buffer);
            }
            ScoaCmd::EOP(this->buffer);
        }

        char_type* start = reinterpret_cast<char_type*>(this->buffer.data());
        char_type* end = start + this->buffer.size();
        this->setg(start, start, end);
        return traits_type::to_int_type(*this->gptr());
    }
}
