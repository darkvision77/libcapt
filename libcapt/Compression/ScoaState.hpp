#ifndef _LIBCAPT_COMPRESSION_SCOA_STATE_HPP_
#define _LIBCAPT_COMPRESSION_SCOA_STATE_HPP_

#include <cstdint>
#include <span>
#include <vector>

namespace Capt::Compression {
    struct ScoaState {
        unsigned LineSize;
        std::vector<uint8_t> PrevLine;

        std::vector<unsigned> Copy;
        std::vector<unsigned> Repeat;
        std::vector<unsigned> Raw;

        explicit ScoaState(unsigned lineSize);

        void ProcessLine(std::span<const uint8_t> line);
    };
}

#endif
