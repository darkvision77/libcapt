#ifndef _LIBCAPT_COMPRESSION_SCOA_STREAMBUF_HPP_
#define _LIBCAPT_COMPRESSION_SCOA_STREAMBUF_HPP_

#include "ScoaState.hpp"
#include <streambuf>
#include <vector>

namespace Capt::Compression {
    class ScoaStreambuf : public std::streambuf {
    private:
        std::streambuf* rasterStream;
        ScoaState state;
        std::vector<uint8_t> buffer;
        unsigned linesRemain;
        std::size_t videoSize;

        std::size_t encodeLine(const std::vector<uint8_t>& line);

    protected:
        int_type underflow() override;

    public:
        explicit ScoaStreambuf(std::streambuf& rasterStream, unsigned lineSize, unsigned lines);
    };
}

#endif
