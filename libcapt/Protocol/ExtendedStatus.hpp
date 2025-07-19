#ifndef _LIBCAPT_PROTOCOL_EXTENDED_STATUS_HPP_
#define _LIBCAPT_PROTOCOL_EXTENDED_STATUS_HPP_

#include "Enums.hpp"
#include <cstdint>

namespace Capt::Protocol {
    struct ExtendedStatus {
        BasicStatus Basic;
        AuxStatus Aux;
        ControllerStatus Controller;
        uint8_t PaperAvailableBits;
        EngineReadyStatus Engine;

        uint16_t Start;
        uint16_t Printing;
        uint16_t Shipped;
        uint16_t Printed;

        inline constexpr bool CheckSlotPaper(int slot) const {
            return (this->PaperAvailableBits & (0x80 >> (slot & 0x1f))) != 0;
        }

        inline constexpr bool IsPrinting() const {
            return (this->Aux & AuxStatus::PAPER_DELIVERY) != 0 || (this->Aux & AuxStatus::SAFE_TIMER) != 0;
        }

        inline constexpr bool ReadyToPrint() const {
            return (this->Basic & BasicStatus::NOT_READY) == 0;
        }

        inline constexpr bool UnitReserved() const {
            return (this->Basic & BasicStatus::UNIT_FREE) == 0;
        }

        inline constexpr bool IsOnline() const {
            return (this->Basic & BasicStatus::OFFLINE) == 0;
        }

        inline constexpr bool Misprint() const {
            return (this->Engine & Protocol::EngineReadyStatus::MIS_PRINT) != 0
                || (this->Engine & Protocol::EngineReadyStatus::MIS_PRINT_2) != 0;
        }

        inline constexpr bool Rejected() const {
            return (this->Controller & ControllerStatus::PRINT_REJECTED) != 0;
        }

        inline constexpr bool VideoDataError() const {
            return (this->Controller & ControllerStatus::OVERRUN) != 0
                || (this->Controller & ControllerStatus::UNDERRUN) != 0
                || (this->Controller & ControllerStatus::INVALID_DATA) != 0
                || (this->Controller & ControllerStatus::MISSING_EOP) != 0;
        }

        inline constexpr bool WaitRequired() const {
            return (this->Controller & ControllerStatus::ENGINE_RESET_IN_PROGRESS) != 0
                || (this->Engine & EngineReadyStatus::DOOR_OPEN) != 0
                || (this->Engine & EngineReadyStatus::NO_CARTRIDGE) != 0
                || (this->Engine & EngineReadyStatus::WAITING) != 0
                || (this->Engine & EngineReadyStatus::TEST_PRINTING) != 0
                || (this->Engine & EngineReadyStatus::NO_PRINT_PAPER) != 0
                || (this->Engine & EngineReadyStatus::JAM) != 0
                || (this->Engine & EngineReadyStatus::CLEANING) != 0
                || (this->Engine & EngineReadyStatus::SERVICE_CALL) != 0;
        }
    };
}

#endif
