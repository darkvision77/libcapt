#ifndef _LIBCAPT_REPRINT_STATUS_HPP_
#define _LIBCAPT_REPRINT_STATUS_HPP_

namespace Capt::Protocol {
    enum class ReprintStatus {
        None = 0,
        Prev,
        Current,
    };
}

#endif
