#include "BufferedPage.hpp"

namespace Capt {
    BufferedPage::int_type BufferedPage::underflow() {
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }
        if (this->videoStream == nullptr) {
            return traits_type::eof();
        }

        std::size_t oldSize = this->buffer.size();
        this->buffer.resize(oldSize + blockSize);

        std::streamsize read = this->videoStream->sgetn(this->buffer.data() + oldSize, blockSize);
        this->buffer.resize(oldSize + read);
        if (read == 0) {
            return traits_type::eof();
        }

        char_type* start = this->buffer.data();
        char_type* end = start + this->buffer.size();
        this->setg(start, start + oldSize, end);
        return traits_type::to_int_type(*this->gptr());
    }

    BufferedPage::BufferedPage(unsigned page, const Protocol::PageParams& params, std::streambuf* stream, std::size_t blockSize) noexcept
        : videoStream(stream), blockSize(blockSize), PageNumber(page), Params(params) {}

    BufferedPage::BufferedPage(BufferedPage&& other) noexcept : buffer(std::move(other.buffer)), videoStream(nullptr), PageNumber(other.PageNumber), Params(std::move(other.Params)) {}

    void BufferedPage::operator=(BufferedPage&& other) noexcept {
        this->buffer = std::move(other.buffer);
        this->PageNumber = other.PageNumber;
        this->Params = std::move(other.Params);
        this->ResetPos();
    }

    void BufferedPage::ResetPos() noexcept {
        char_type* start = this->buffer.data();
        char_type* end = start + this->buffer.size();
        this->setg(start, start, end);
    }
}
