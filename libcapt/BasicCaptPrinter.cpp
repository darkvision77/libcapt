#include "BasicCaptPrinter.hpp"
#include "UnexpectedBehaviourError.hpp"
#include "Protocol/Enums.hpp"
#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include "Protocol/Protocol.hpp"
#include <iomanip>
#include <cassert>
#include <sstream>
#include <vector>

namespace Capt {
    #define CHECK_RETCODE(EXP) checkRetcode(EXP, #EXP)
    inline static void checkRetcode(uint8_t cmdResult, std::string_view paramName) {
        if (cmdResult != 0) {
            std::ostringstream ss;
            ss << paramName << " returned non-successfull code (0x" << std::hex << std::setfill('0') << std::setw(2) << cmdResult << ')';
            throw UnexpectedBehaviourError(ss.str());
        }
    }

    BasicCaptPrinter::BasicCaptPrinter(std::iostream& stream) noexcept : stream(stream) {}

    Protocol::ExtendedStatus BasicCaptPrinter::GetStatus() {
        return Protocol::PC_GET_EXTENDED_STATUS(this->stream);
    }

    Protocol::PrinterInfo BasicCaptPrinter::GetPrinterInfo() {
        if (!this->cachedInfo) {
            this->cachedInfo = Protocol::PC_GET_PRINTER_INFO(this->stream);
        }
        return *this->cachedInfo;
    }

    void BasicCaptPrinter::ReserveUnit() {
        CHECK_RETCODE(Protocol::PC_RESERVE_UNIT(this->stream));
        Protocol::ExtendedStatus ex = this->GetStatus();
        if (!ex.UnitReserved()) {
            throw UnexpectedBehaviourError("failed to reserve unit");
        }
    }

    void BasicCaptPrinter::ClearError(const Protocol::ExtendedStatus* status) {
        Protocol::ExtendedStatus ex = status == nullptr ? this->GetStatus() : *status;
        assert(ex.UnitReserved());
        CHECK_RETCODE(Protocol::PCR_CLEAR_ERROR(this->stream));
        CHECK_RETCODE(Protocol::PCR_DISCARD_DATA(this->stream));
        if (ex.Misprint()) {
            CHECK_RETCODE(Protocol::PCR_CLEAR_MISPRINT(this->stream));
        }
        if ((ex.Controller & Protocol::ControllerStatus::ENGINE_COMM_ERROR) != 0) {
            CHECK_RETCODE(Protocol::PCR_RESET_ENGINE(this->stream));
        }
    }

    bool BasicCaptPrinter::GoOnline(unsigned page) {
        CHECK_RETCODE(Protocol::PCR_GO_ONLINE(this->stream, page));
        Protocol::ExtendedStatus ex = this->GetStatus();
        return ex.Online();
    }

    void BasicCaptPrinter::Cleaning() {
        CHECK_RETCODE(Protocol::PCR_CLEANING(this->stream));
    }

    bool BasicCaptPrinter::WriteVideoData(std::stop_token stopToken, const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize) {
        if (blockSize == 0) {
            blockSize = this->GetPrinterInfo().BlockSize;
        }
        assert((blockSize + 4) <= UINT16_MAX);

        Protocol::IC_BEGIN_PAGE(this->stream, params);
        Protocol::IC_BEGIN_DATA(this->stream);
        std::vector<uint8_t> buffer(blockSize);
        while (true) {
            std::streamsize read = videoStream.sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if (read <= 0) {
                break;
            }
            while (!stopToken.stop_requested()) {
                Protocol::BasicStatus bs = Protocol::PCR_GET_BASIC_STATUS(this->stream);
                if ((bs & Protocol::BasicStatus::NOT_READY) != 0) {
                    return false;
                }
                if ((bs & Protocol::BasicStatus::IM_DATA_BUSY) == 0) {
                    break;
                }
            }
            if (stopToken.stop_requested()) {
                return false;
            }
            Protocol::IC_VIDEO_DATA(this->stream, {buffer.data(), static_cast<std::size_t>(read)});
        }
        Protocol::IC_END_PAGE(this->stream);
        return true;
    }

    void BasicCaptPrinter::GoOffline() {
        CHECK_RETCODE(Protocol::PCR_GO_OFFLINE(this->stream));
        Protocol::ExtendedStatus ex = this->GetStatus();
        if (ex.Online()) {
            throw UnexpectedBehaviourError("failed to offline");
        }
    }

    void BasicCaptPrinter::ReleaseUnit() {
        Protocol::ExtendedStatus ex = this->GetStatus();
        if (!ex.UnitReserved()) {
            return;
        }
        CHECK_RETCODE(Protocol::PCR_RELEASE_UNIT(this->stream));
        ex = this->GetStatus();
        if (ex.UnitReserved()) {
            throw UnexpectedBehaviourError("failed to reserve unit");
        }
    }
}
