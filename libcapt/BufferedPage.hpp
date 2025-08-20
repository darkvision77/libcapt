#ifndef _LIBCAPT_BUFFERED_PAGE_HPP_
#define _LIBCAPT_BUFFERED_PAGE_HPP_

#include "Protocol/PageParams.hpp"
#include <streambuf>
#include <vector>

namespace Capt {
    class BufferedPage : public std::streambuf {
    private:
        std::vector<char_type> buffer;
        std::streambuf* videoStream = nullptr;
        std::size_t blockSize = 4096;

        int_type underflow() override;
    public:
        unsigned PageNumber = 0;
        Protocol::PageParams Params;

        BufferedPage() = default;

        explicit BufferedPage(unsigned page, const Protocol::PageParams& params, std::streambuf* stream, std::size_t blockSize = 4096) noexcept;

        BufferedPage(const BufferedPage& other) = delete;
        BufferedPage(BufferedPage&& other) noexcept;
        void operator=(BufferedPage&& other) noexcept;

        void ResetPos() noexcept;
    };
}

#endif
