#ifndef _LIBCAPT_COMPRESSION_DECODER_DECODER_STREAMBUF_HPP_
#define _LIBCAPT_COMPRESSION_DECODER_DECODER_STREAMBUF_HPP_

#include <ostream>
#include <streambuf>
#include <vector>
#include <cstdint>

namespace Capt::Compression {
    class DecoderStreambuf : public std::streambuf {
    private:
        std::streambuf& reader;
        std::ostream* commandLog;

        std::vector<uint8_t> buffer;
        unsigned lineSize;
        std::size_t videoSize;

        void repeat(unsigned count, uint8_t repeatByte);
        void repeatX(unsigned count);
        void copy(unsigned count);
        void raw(const std::vector<uint8_t>& data);
        void endLine();

        int decodeNext();
        bool decodeLine();

        int_type underflow() override;
    public:
        explicit DecoderStreambuf(std::streambuf& reader, unsigned lineSize, std::ostream* commandLog = nullptr) noexcept;
    };
}

#endif
