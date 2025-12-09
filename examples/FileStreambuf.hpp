#pragma once
#include <streambuf>
#include <cassert>
#include <cstdio>
#include <vector>

class FileStreambuf : public std::streambuf {
private:
    std::FILE* file = nullptr;
    std::vector<char_type> wbuff;
    char_type curr = 0;

    int_type overflow(int_type c = traits_type::eof()) override {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *this->pptr() = traits_type::to_char_type(c);
            this->pbump(1);
        }
        return this->sync() == 0 ? traits_type::not_eof(c) : traits_type::eof();
    }

    int_type underflow() override {
        assert(this->file != nullptr);
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }
        std::size_t read = std::fread(&this->curr, sizeof(char_type), 1, this->file);
        if (read != 1) {
            return traits_type::eof();
        }
        this->setg(&this->curr, &this->curr, &this->curr + 1);
        return traits_type::to_int_type(*this->gptr());
    }

    std::streamsize xsgetn(char_type* s, std::streamsize n) override {
        assert(this->file != nullptr);
        if (n <= 0) {
            return n;
        }
        return std::fread(s, sizeof(char_type), n, this->file);
    }

    int sync() override {
        assert(this->file != nullptr);
        std::ptrdiff_t count = this->pptr() - this->pbase();
        if (count != 0) {
            std::ptrdiff_t w = std::fwrite(this->pbase(), sizeof(char_type), count, this->file);
            if (w != count) {
                return -1;
            }
            this->pbump(-w);
            assert(this->pptr() == this->pbase());
        }
        return std::fflush(this->file) == 0 ? 0 : -1;
    }
public:
    explicit FileStreambuf(std::size_t wbuffsize = 4096) : wbuff(wbuffsize) {
        char_type* start = this->wbuff.data();
        char_type* end = start + this->wbuff.size() - 1;
        this->setp(start, end);
    }

    ~FileStreambuf() {
        this->Close();
    }

    bool Open(const char* filename, const char* mode) {
        this->file = std::fopen(filename, mode);
        return this->file != nullptr;
    }

    void Close() {
        if (this->file != nullptr) {
            std::fclose(this->file);
            this->file = nullptr;
        }
    }
};
