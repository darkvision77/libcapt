#ifndef _LIBCAPT_COMPRESSION_SCOA_CMD_HPP_
#define _LIBCAPT_COMPRESSION_SCOA_CMD_HPP_

#include <cstdint>
#include <vector>

namespace Capt::Compression::ScoaCmd {
    std::size_t CopyThenRepeat(std::vector<uint8_t>& buffer, unsigned copyCount, unsigned repeatCount, uint8_t repeatByte);
    std::size_t CopyThenRaw(std::vector<uint8_t>& buffer, unsigned copyCount, const uint8_t* rawData, unsigned rawCount);
    std::size_t Extend(std::vector<uint8_t>& buffer, unsigned count);
    std::size_t RepeatXLong(std::vector<uint8_t>& buffer, unsigned repeatCount);
    std::size_t CopyThenRepeatLong(std::vector<uint8_t>& buffer, unsigned copyCount, unsigned repeatCount, uint8_t repeatByte);
    std::size_t RepeatThenRawLong(std::vector<uint8_t>& buffer, unsigned repeatCount, uint8_t repeatByte, const uint8_t* rawData, unsigned rawCount);
    std::size_t CopyThenRawLong(std::vector<uint8_t>& buffer, unsigned copyCount, const uint8_t* rawData, unsigned rawCount);
    std::size_t CopyLong(std::vector<uint8_t>& buffer, unsigned copyCount);
    std::size_t EOP(std::vector<uint8_t>& buffer);
    std::size_t RepeatX(std::vector<uint8_t>& buffer, unsigned repeatCount);
    std::size_t CopyShort(std::vector<uint8_t>& buffer, unsigned copyCount);
    std::size_t EOL(std::vector<uint8_t>& buffer);
    std::size_t NOP(std::vector<uint8_t>& buffer);
    std::size_t RepeatThenRaw(std::vector<uint8_t>& buffer, unsigned repeatCount, uint8_t repeatByte, const uint8_t* rawData, unsigned rawCount);
}

#endif
