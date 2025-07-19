#ifndef _LIBCAPT_PROTOCOL_PROTOCOL_HPP_
#define _LIBCAPT_PROTOCOL_PROTOCOL_HPP_

#include "Enums.hpp"
#include "ExtendedStatus.hpp"
#include "PageParams.hpp"
#include <iostream>
#include <cstdint>

namespace Capt::Protocol {
    void BeginPage(std::ostream& stream, const PageParams& params);
    void BeginData(std::ostream& stream);
    void EndPage(std::ostream& stream);
    void VideoData(std::ostream& stream, uint8_t* data, std::size_t count);
    ExtendedStatus GetExtendedStatus(std::iostream& stream);
    BasicStatus GetBasicStatus(std::iostream& stream, uint8_t* changed = nullptr);
    uint8_t GoOnline(std::iostream& stream, uint16_t pageNumber);
    uint8_t ReserveUnit(std::iostream& stream);
    uint8_t DiscardData(std::iostream& stream);
    uint8_t ClearError(std::iostream& stream);
    uint8_t GoOffline(std::iostream& stream);
    uint8_t ReleaseUnit(std::iostream& stream);
    uint8_t ClearMisprint(std::iostream& stream);
    uint8_t ResetEngine(std::iostream& stream);
}

#endif
