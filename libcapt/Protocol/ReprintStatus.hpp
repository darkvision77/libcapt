#ifndef _LIBCAPT_PROTOCOL_REPRINT_STATUS_HPP_
#define _LIBCAPT_PROTOCOL_REPRINT_STATUS_HPP_

namespace Capt::Protocol {
    enum class ReprintStatus {
        None = 0,
        Prev,
        Current,
    };
}

#endif
