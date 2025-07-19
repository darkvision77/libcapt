#include "libcapt/Compression/ScoaStreambuf.hpp"
#include "libcapt/Protocol/Protocol.hpp"
#include <cctype>
#include <fstream>
#include <istream>
#include <print>
#include <stdexcept>
#include <vector>

using namespace Capt;

static bool readPbmHeader(std::istream& stream, unsigned& width, unsigned& height) {
    char buffer[3];
    stream.read(buffer, sizeof(buffer));
    if (stream.eof()) {
        return false;
    }
    if (buffer[0] != 'P' || buffer[1] != '4' || !std::isspace(buffer[2])) {
        throw std::runtime_error("PBM: invalid magic");
    }
    while (stream.peek() == '#') {
        while (stream.get() != '\n') {
            if (stream.eof()) {
                throw std::runtime_error("PBM: unexpected EOF");
            }
        }
    }
    if (!(stream >> width)) {
        throw std::runtime_error("PBM: failed to read width");
    }
    if (!std::isspace(stream.get())) {
        throw std::runtime_error("PBM: unexpected char");
    }
    if (!(stream >> height)) {
        throw std::runtime_error("PBM: failed to read height");
    }
    while (!std::isspace(stream.get()) && stream.good());
    if (stream.eof()) {
        throw std::runtime_error("PBM: unexpected EOF");
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::println(stderr, "Usage {} pbmfile outfile", argv[0]);
        return 1;
    }

    std::ifstream pbmStream(argv[1], std::ios_base::in | std::ios_base::binary);
    if (!pbmStream.is_open()) {
        std::println(stderr, "Failed to open pbmfile");
        return 1;
    }

    std::ofstream outStream(argv[2], std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);
    if (!outStream.is_open()) {
        std::println(stderr, "Failed to open outfile");
        return 1;
    }

    unsigned lineSize;
    unsigned lines;
    if (!readPbmHeader(pbmStream, lineSize, lines)) {
        std::println(stderr, "PBM read failed");
        return 1;
    }
    if (lineSize % 8 != 0) {
        throw std::runtime_error("PBM width must be a multiple of 8");
    }
    lineSize /= 8;

    Compression::ScoaStreambuf scoaStreambuf(*pbmStream.rdbuf(), lineSize, lines);

    Protocol::PageParams pp {
        .ImageLineSize = static_cast<uint16_t>(lineSize),
        .ImageLines = static_cast<uint16_t>(lines),
    };
    std::println(stderr, "LineSize = {}", pp.ImageLineSize);
    std::println(stderr, "Lines = {}", pp.ImageLines);

    Protocol::BeginPage(outStream, pp);
    Protocol::BeginData(outStream);
    const std::size_t maxsize = 4096;
    while (true) {
        std::vector<uint8_t> buffer(maxsize);
        std::streamsize read = scoaStreambuf.sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (read == 0) {
            break;
        }
        Protocol::VideoData(outStream, buffer.data(), read);
    }
    Protocol::EndPage(outStream);
    return 0;
}
