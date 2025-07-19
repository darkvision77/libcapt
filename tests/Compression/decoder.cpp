#include "libcapt/Core/CaptPacket.hpp"
#include "libcapt/Compression/Decoder/DecoderStreambuf.hpp"
#include <fstream>
#include <iostream>
#include <istream>
#include <print>
#include <stdexcept>
#include <streambuf>

using namespace Capt;

class VideoDataStreambuf : public std::streambuf {
private:
    std::vector<uint8_t> buffer;
    std::istream& reader;
public:
    unsigned LineSize = 0;
    unsigned LinesCount = 0;

protected:
    int_type underflow() override {
        if (this->reader.eof() || this->reader.fail()) {
            return traits_type::eof();
        }
        if (this->gptr() < this->egptr()) {
            return traits_type::to_int_type(*this->gptr());
        }

        while (true) {
            CaptPacket packet = CaptPacket::ReadFrom(this->reader);
            if (packet.Opcode == 0xD0A0) {
                this->LineSize = (static_cast<unsigned>(packet.Payload[31-4]) << 8) | static_cast<unsigned>(packet.Payload[30-4]);
                this->LinesCount = (static_cast<unsigned>(packet.Payload[33-4]) << 8) | static_cast<unsigned>(packet.Payload[32-4]);
            } else if (packet.Opcode == 3 || packet.Opcode == 0xD0A2) {
                return traits_type::eof();
            } else if (packet.Opcode == 0xC0A0) {
                this->buffer = packet.Payload;
                break;
            }
        }

        char_type* start = reinterpret_cast<char_type*>(this->buffer.data());
        this->setg(start, start, start + this->buffer.size());
        return traits_type::to_int_type(*this->gptr());
    }

public:
    VideoDataStreambuf(std::istream& reader) : reader(reader) {
        this->underflow();
        if (this->LineSize == 0 || this->LinesCount == 0) {
            throw std::runtime_error("failed to parse PBM file");
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::println(stderr, "Usage: {} filterout outfile cmdfile", argv[0]);
        return 1;
    }
    std::ifstream filterInput(argv[1], std::ios_base::in | std::ios_base::binary);
    if (!filterInput.is_open()) {
        std::println(stderr, "Failed to open filterout file");
        return 1;
    }

    std::ofstream cmdout(argv[3], std::ios_base::out);
    if (!cmdout.is_open()) {
        std::println(stderr, "Failed to open cmdfile");
        return 1;
    }

    VideoDataStreambuf videoStreambuf(filterInput);
    Capt::Compression::DecoderStreambuf dec(videoStreambuf, videoStreambuf.LineSize, &cmdout);

    std::ofstream outfile(argv[2], std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (!outfile.is_open()) {
        std::println(stderr, "Failed to open outfile");
        return 1;
    }
    outfile << "P4\n" << (videoStreambuf.LineSize * 8) << " " << videoStreambuf.LinesCount << "\n";

    std::size_t decodedSize = 0;
    while (true) {
        std::vector<char> buffer(videoStreambuf.LineSize);
        std::streamsize read = dec.sgetn(buffer.data(), buffer.size());
        if (read == 0) {
            break;
        }
        outfile.write(buffer.data(), read);
        decodedSize += read;
    }

    std::println(stderr, "Decoded size  = {}", decodedSize);
    std::println(stderr, "Expected size = {}", videoStreambuf.LineSize * videoStreambuf.LinesCount);
    return 0;
}
