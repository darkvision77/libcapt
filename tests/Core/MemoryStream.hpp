#ifndef _LIBCAPT_TESTS_CORE_MEMORY_STREAM_HPP_
#define _LIBCAPT_TESTS_CORE_MEMORY_STREAM_HPP_

#include <streambuf>
#include <iostream>
#include <vector>

class MemoryStreambuf : public std::streambuf {
protected:
	int_type overflow(int_type c) override {
        if (c == traits_type::eof()){
            return traits_type::eof();
        }
        this->Buffer.push_back(traits_type::to_char_type(c));
        return c;
    }

	int_type underflow() override {
        if (this->Buffer.empty()) {
            return traits_type::eof();
        }
        return traits_type::to_int_type(Buffer[0]);
    }

    int_type uflow() override {
        int_type val = this->underflow();
        if (val != traits_type::eof()) {
            this->Buffer.erase(this->Buffer.begin());
        }
        return val;
    }

public:
	std::vector<uint8_t> Buffer;

	MemoryStreambuf() {}
};

class MemoryStream : public std::iostream {
private:
    MemoryStreambuf buf;
public:
    MemoryStream() : std::iostream(&buf) {}
    MemoryStream(const std::vector<uint8_t>& buffer) : std::iostream(&buf) {
        this->buf.Buffer = buffer;
    }

    std::vector<uint8_t>& Buffer() noexcept {
        return this->buf.Buffer;
    }
};

#endif
