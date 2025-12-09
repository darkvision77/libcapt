#pragma once
#include <stdexcept>

namespace Capt::Protocol {
    class ProtocolError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}
