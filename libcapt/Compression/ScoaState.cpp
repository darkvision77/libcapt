#include "ScoaState.hpp"
#include <stdexcept>

namespace Capt::Compression {
    ScoaState::ScoaState(unsigned lineSize)
        : LineSize(lineSize), Copy(lineSize), Repeat(lineSize), Raw(lineSize) {}

    void ScoaState::ProcessLine(const std::vector<uint8_t>& line) {
        if (line.size() != this->LineSize) {
            throw std::invalid_argument("line size mismatch");
        }

        unsigned copyCount = 0;
        unsigned repCount = 1;
        unsigned rawCount = 0;

        for (int i = line.size()-1; i >= 0; i--) {
            this->Repeat[i] = repCount;
            if (i >= 1 && line[i] == line[i-1]) {
                repCount++;
            } else {
                repCount = 1;
            }

            if (this->PrevLine.size() != 0) {
                if (line[i] == this->PrevLine[i]) {
                    copyCount++;
                } else {
                    copyCount = 0;
                }
            }
            this->Copy[i] = copyCount;

            if (this->Copy[i] == 0 && this->Repeat[i] == 1) {
                rawCount++;
            } else {
                rawCount = 0;
            }
            this->Raw[i] = rawCount;
        }
        this->PrevLine = line;
    }
}
