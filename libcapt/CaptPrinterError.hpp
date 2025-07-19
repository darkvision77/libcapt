#ifndef _LIBCAPT_CAPT_PRINTER_ERROR_HPP_
#define _LIBCAPT_CAPT_PRINTER_ERROR_HPP_

#include <stdexcept>

namespace Capt {
    class CaptPrinterError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

#endif
