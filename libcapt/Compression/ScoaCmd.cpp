#include "ScoaCmd.hpp"
#include <vector>
#include <cassert>

namespace Capt::Compression::ScoaCmd {
    std::size_t CopyThenRepeat(std::vector<uint8_t>& buffer, unsigned copyCount, unsigned repeatCount, uint8_t repeatByte) {
        assert(copyCount >= 0);
        assert(copyCount <= 7);
        assert(repeatCount >= 0);
        assert(repeatCount <= 7);
        assert(repeatCount != 0 || copyCount >= 3); // Check collision
        buffer.push_back(0x40 | ((repeatCount & 0x07) << 3) | (copyCount & 0x07));
        buffer.push_back(repeatByte);
        return 2;
    }

    std::size_t CopyThenRaw(std::vector<uint8_t>& buffer, unsigned copyCount, std::span<const uint8_t> rawData) {
        assert(copyCount >= 0 && copyCount <= 7);
        assert(rawData.size() >= 1 && rawData.size() <= 7);
        buffer.push_back(((rawData.size() & 0x07) << 3) | (copyCount & 0x07));
        buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        return 1 + rawData.size();
    }

    std::size_t Extend(std::vector<uint8_t>& buffer, unsigned count) {
        buffer.push_back(0xa0 | ((count >> 3) & 0x1f));
        return 1;
    }

    std::size_t RepeatXLong(std::vector<uint8_t>& buffer, unsigned repeatCount) {
        assert(repeatCount >= 8);
        assert(repeatCount <= 255);
        std::size_t vsize = Extend(buffer, repeatCount);
        buffer.push_back((repeatCount & 0x07) << 3);
        return vsize + 1;
    }

    std::size_t CopyThenRepeatLong(std::vector<uint8_t>& buffer, unsigned copyCount, unsigned repeatCount, uint8_t repeatByte) {
        assert(copyCount >= 0);
        assert(copyCount <= 7);
        assert(repeatCount >= 8);
        assert(repeatCount <= 255);
        std::size_t vsize = Extend(buffer, repeatCount);
        buffer.push_back(0x80 | ((repeatCount & 0x07) << 3) | (copyCount & 0x07));
        buffer.push_back(repeatByte);
        return vsize + 2;
    }

    std::size_t RepeatThenRawLong(std::vector<uint8_t>& buffer, unsigned repeatCount, uint8_t repeatByte, std::span<const uint8_t> rawData) {
        assert(rawData.size() >= 8);
        assert(rawData.size() <= 255);
        assert(repeatCount >= 2);
        assert(repeatCount <= 7);
        std::size_t vsize = Extend(buffer, rawData.size());
        buffer.push_back(0x40 | ((repeatCount & 0x07) << 3) | (rawData.size() & 0x07));
        buffer.push_back(repeatByte);
        buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        return vsize + 2 + rawData.size();
    }

    std::size_t CopyThenRawLong(std::vector<uint8_t>& buffer, unsigned copyCount, std::span<const uint8_t> rawData) {
        assert(rawData.size() >= 8);
        assert(rawData.size() <= 255);
        assert(copyCount >= 0);
        assert(copyCount <= 7);
        std::size_t vsize = Extend(buffer, rawData.size());
        buffer.push_back(0xc0 | (rawData.size() & 0x07) << 3 | (copyCount & 0x07));
        buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        return vsize + 1 + rawData.size();
    }

    std::size_t CopyLong(std::vector<uint8_t>& buffer, unsigned copyCount) {
        assert(copyCount >= 8);
        assert(copyCount <= 248);
        assert(copyCount % 8 == 0);
        buffer.push_back(0x80 | ((copyCount >> 3) & 0x1f));
        return 1;
    }

    std::size_t EOP(std::vector<uint8_t>& buffer) {
        buffer.push_back(0x42);
        return 1;
    }

    std::size_t RepeatX(std::vector<uint8_t>& buffer, unsigned repeatCount) {
        assert(repeatCount != 0);
        assert(repeatCount <= 7);
        buffer.push_back(0xc0 | ((repeatCount & 0x07) << 3));
        return 1;
    }

    std::size_t CopyShort(std::vector<uint8_t>& buffer, unsigned copyCount) {
        assert(copyCount != 0);
        assert(copyCount <= 7);
        buffer.push_back(0xc0 | (copyCount & 0x07));
        return 1;
    }

    std::size_t EOL(std::vector<uint8_t>& buffer) {
        buffer.push_back(0x41);
        return 1;
    }

    std::size_t NOP(std::vector<uint8_t>& buffer) {
        buffer.push_back(0x40);
        return 1;
    }

    std::size_t RepeatThenRaw(std::vector<uint8_t>& buffer, unsigned repeatCount, uint8_t repeatByte, std::span<const uint8_t> rawData) {
        assert(rawData.size() != 0 && repeatCount != 0);
        assert(repeatCount >= 2);
        assert(repeatCount <= 7);
        assert(rawData.size() >= 1);
        assert(rawData.size() <= 7);
        buffer.push_back(0xc0 | ((repeatCount & 0x07) << 3) | (rawData.size() & 0x07));
        buffer.push_back(repeatByte);
        buffer.insert(buffer.end(), rawData.begin(), rawData.end());
        return 2 + rawData.size();
    }
}
