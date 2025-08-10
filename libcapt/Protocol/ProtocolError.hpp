#ifndef _LIBCAPT_PROTOCOL_PROTOCOL_ERROR_HPP_
#define _LIBCAPT_PROTOCOL_PROTOCOL_ERROR_HPP_

#include <stdexcept>

namespace Capt::Protocol {
    class ProtocolError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

#endif
