#include "ScoaStreambuf.hpp"
#include "ScoaState.hpp"
#include "ScoaCmd.hpp"
#include <cassert>
#include <ios>
#include <iostream>

namespace Capt::Compression {
    using int_type = ScoaStreambuf::int_type;

    ScoaStreambuf::ScoaStreambuf(std::streambuf& rasterStream, unsigned lineSize, unsigned lines)
        : rasterStream(&rasterStream), state(lineSize), lineBuffer(lineSize), linesRemain(lines), videoSize(0) {}

    std::size_t cmd_Copy(std::vector<uint8_t>& buffer, unsigned copyCount) {
        if (copyCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        while (copyCount >= 8) {
            unsigned copy = std::min(copyCount, 255u);
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

    std::size_t cmd_CopyThenRaw(std::vector<uint8_t>& buffer, unsigned copyCount, std::span<const uint8_t> rawData) {
        if (copyCount == 0 && rawData.size() == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (copyCount <= 7) {
            while (rawData.size() >= 8) {
                std::size_t count = std::min(rawData.size(), 255uz);
                vsize += ScoaCmd::CopyThenRawLong(buffer, copyCount, rawData.subspan(0, count));
                copyCount = 0;
                rawData = rawData.subspan(count);
            }
            if (copyCount != 0 || rawData.size() != 0) {
                vsize += ScoaCmd::CopyThenRaw(buffer, copyCount, rawData);
            }
        } else {
            while (copyCount >= 8) {
                unsigned copy = std::min(copyCount, 248u);
                copy = copy - (copy % 8);
                vsize += ScoaCmd::CopyLong(buffer, copy);
                copyCount -= copy;
            }
            return vsize + cmd_CopyThenRaw(buffer, copyCount, rawData);
        }
        return vsize;
    }

    std::size_t cmd_WriteRaw(std::vector<uint8_t>& buffer, std::span<const uint8_t> rawData) {
        if (rawData.size() == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (rawData.size() <= 7) {
            vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData);
        } else {
            while (rawData.size() >= 8) {
                std::size_t count = std::min(rawData.size(), 255uz);
                vsize += ScoaCmd::CopyThenRawLong(buffer, 0, rawData.subspan(0, count));
                rawData = rawData.subspan(count);
            }
            if (rawData.size() != 0) {
                vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData);
            }
        }
        return vsize;
    }

    std::size_t cmd_RepeatThenRaw(std::vector<uint8_t>& buffer, unsigned repeatCount, uint8_t repeatByte, std::span<const uint8_t> rawData) {
        if (repeatCount == 0 && rawData.size() == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (repeatCount <= 7) {
            if (rawData.size() <= 7) {
                if (rawData.size() == 0) {
                    vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                } else if (repeatCount == 0) {
                    vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData);
                } else {
                    if (repeatCount >= 2) {
                        vsize += ScoaCmd::RepeatThenRaw(buffer, repeatCount, repeatByte, rawData);
                    } else {
                        vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                        vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData);
                    }
                }
            } else {
                if (repeatCount >= 2) {
                    vsize += ScoaCmd::RepeatThenRaw(buffer, repeatCount, repeatByte, rawData.subspan(0, 7));
                    rawData = rawData.subspan(7);
                } else if (repeatCount != 0) {
                    vsize += ScoaCmd::CopyThenRepeat(buffer, 0, repeatCount, repeatByte);
                }
                while (rawData.size() >= 8) {
                    std::size_t count = std::min(rawData.size(), 255uz);
                    vsize += ScoaCmd::CopyThenRawLong(buffer, 0, rawData.subspan(0, count));
                    rawData = rawData.subspan(count);
                }
                if (rawData.size() != 0) {
                    vsize += ScoaCmd::CopyThenRaw(buffer, 0, rawData);
                }
            }
        } else {
            while (repeatCount >= 8) {
                unsigned count = std::min(repeatCount, 255u);
                vsize += ScoaCmd::CopyThenRepeatLong(buffer, 0, count, repeatByte);
                repeatCount -= count;
            }
            return vsize + cmd_RepeatThenRaw(buffer, repeatCount, repeatByte, rawData);
        }
        return vsize;
    }

    std::size_t cmd_CopyThenRepeat(std::vector<uint8_t>& buffer, unsigned copyCount, unsigned repeatCount, uint8_t repeatByte) {
        if (repeatCount == 0 && copyCount == 0) {
            return 0;
        }
        std::size_t vsize = 0;
        if (copyCount <= 7) {
            while (repeatCount >= 8) {
                unsigned rep = std::min(repeatCount, 255u);
                vsize += ScoaCmd::CopyThenRepeatLong(buffer, copyCount, rep, repeatByte);
                copyCount = 0;
                repeatCount -= rep;
            }
            if (copyCount != 0 || repeatCount != 0) {
                vsize += ScoaCmd::CopyThenRepeat(buffer, copyCount, repeatCount, repeatByte);
            }
        } else {
            while (copyCount >= 8) {
                unsigned copy = std::min(copyCount, 248u);
                copy = copy - (copy % 8);
                vsize += ScoaCmd::CopyLong(buffer, copy);
                copyCount -= copy;
            }
            return vsize + cmd_CopyThenRepeat(buffer, copyCount, repeatCount, repeatByte);
        }
        return vsize;
    }

    std::size_t ScoaStreambuf::encodeLine(std::span<const uint8_t> line) {
        std::size_t encodedSize = 0;
        assert(line.size() == state.LineSize);
        for (unsigned i = 0; i < state.LineSize;) {
            if (state.Copy[i] == state.LineSize - i) {
                encodedSize += ScoaCmd::EOL(this->buffer);
                break;
            }
            unsigned nextPos = i;
            if (state.Raw[i] != 0) {
                encodedSize += cmd_WriteRaw(this->buffer, line.subspan(i, state.Raw[i]));
                nextPos += state.Raw[i];
            } else if (state.Copy[i] != 0) {
                nextPos += state.Copy[i];
                assert(nextPos < state.LineSize);
                if (state.Raw[nextPos] != 0) {
                    encodedSize += cmd_CopyThenRaw(this->buffer, state.Copy[i], line.subspan(nextPos, state.Raw[nextPos]));
                    nextPos += state.Raw[nextPos];
                } else if (state.Repeat[nextPos] != 1) {
                    encodedSize += cmd_CopyThenRepeat(this->buffer, state.Copy[i], state.Repeat[nextPos], line[nextPos]);
                    nextPos += state.Repeat[nextPos];
                } else {
                    encodedSize += cmd_Copy(this->buffer, state.Copy[i]);
                }
            } else if (state.Repeat[i] != 1) {
                nextPos += state.Repeat[i];
                if (nextPos != state.LineSize && state.Raw[nextPos] != 0) {
                    encodedSize += cmd_RepeatThenRaw(this->buffer, state.Repeat[i], line[i], line.subspan(nextPos, state.Raw[nextPos]));
                    nextPos += state.Raw[nextPos];
                } else {
                    encodedSize += cmd_RepeatThenRaw(this->buffer, state.Repeat[i], line[i], {});
                }
            }
            assert(i != nextPos);
            i = nextPos;
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

        std::streamsize read = this->rasterStream->sgetn(reinterpret_cast<char*>(this->lineBuffer.data()), this->lineBuffer.size());
        if (read < static_cast<std::streamsize>(this->lineBuffer.size())) {
            return traits_type::eof();
        }

        this->buffer.clear();
        this->state.ProcessLine(this->lineBuffer);
        this->videoSize += this->encodeLine(this->lineBuffer);
        this->state.PrevLine.resize(this->lineBuffer.size());
        this->state.PrevLine.swap(this->lineBuffer);

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
