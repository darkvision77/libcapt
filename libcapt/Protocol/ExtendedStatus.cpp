#include "ExtendedStatus.hpp"
#include <iomanip>

namespace Capt {
    std::ostream& operator<<(std::ostream& os, const ExtendedStatus& status) {
        auto fill = os.fill();
        auto flags = os.flags();
        os << std::showbase << std::hex << std::setfill('0') << std::internal;
        os << "ExtendedStatus[Basic=" << std::setw(4) << static_cast<int>(status.Basic)
            << " Aux=" << std::setw(4) << static_cast<int>(status.Aux)
            << " Controller=" << std::setw(4) << static_cast<int>(status.Controller)
            << " Engine=" << std::setw(4) << status.Engine
            << std::noshowbase << std::dec
            << " Start=" << status.Start
            << " Printing=" << status.Printing
            << " Shipped=" << status.Shipped
            << " Printed=" << status.Printed
            << ']';
        os.flags(flags);
        os.fill(fill);
        return os;
    }
}
