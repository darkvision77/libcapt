#ifndef _LIBCAPT_UNEXPECTED_BEHAVIOUR_ERROR_HPP_
#define _LIBCAPT_UNEXPECTED_BEHAVIOUR_ERROR_HPP_

#include <stdexcept>

namespace Capt {
    class UnexpectedBehaviourError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

#endif
