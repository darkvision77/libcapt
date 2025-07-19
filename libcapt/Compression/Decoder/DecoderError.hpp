#ifndef _LIBCAPT_COMPRESSION_DECODER_DECODER_ERROR_HPP_
#define _LIBCAPT_COMPRESSION_DECODER_DECODER_ERROR_HPP_

#include <stdexcept>

namespace Capt::Compression {
    class DecoderError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

#endif
