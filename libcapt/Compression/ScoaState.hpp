#ifndef _LIBCAPT_COMPRESSION_SCOA_STATE_HPP_
#define _LIBCAPT_COMPRESSION_SCOA_STATE_HPP_

#include <cstdint>
#include <vector>

namespace Capt::Compression {
    class ScoaState {
    public:
        unsigned LineSize;
        std::vector<uint8_t> PrevLine;

        std::vector<unsigned> Copy;
        std::vector<unsigned> Repeat;
        std::vector<unsigned> Raw;

        explicit ScoaState(unsigned lineSize);

        void ProcessLine(const std::vector<uint8_t>& line);
    };
}

#endif
