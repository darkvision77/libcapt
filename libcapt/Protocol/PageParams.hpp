#ifndef _LIBCAPT_PROTOCOL_PAGE_PARAMS_HPP_
#define _LIBCAPT_PROTOCOL_PAGE_PARAMS_HPP_

#include "Enums.hpp"
#include <cstdint>

namespace Capt::Protocol {
    struct PageParams {
        uint8_t PaperSize;
        uint8_t TonerDensity;
        uint8_t Mode;
        ResolutionIdx Resolution;
        bool SmoothEnable;
        bool TonerSaving;
        uint16_t MarginLeft;
        uint16_t MarginTop;
        uint16_t ImageLineSize;
        uint16_t ImageLines;
        uint16_t PaperWidth;
        uint16_t PaperHeight;
    };
}

#endif
