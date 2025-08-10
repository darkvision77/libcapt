#include "CaptPrinter.hpp"
#include "UnexpectedBehaviourError.hpp"
#include "Protocol/Enums.hpp"
#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include "Protocol/Protocol.hpp"
#include <format>
#include <mutex>
#include <cassert>

namespace Capt {
    #define CHECK_RETCODE(EXP) checkRetcode(EXP, #EXP)
    static inline void checkRetcode(uint8_t cmdResult, std::string_view paramName) {
        if (cmdResult != 0) {
            throw UnexpectedBehaviourError(std::format("{} returned non-successfull code (0x{:02X})", paramName, cmdResult));
        }
    }

    CaptPrinter::CaptPrinter(std::iostream& stream) : stream(stream), status(Protocol::ExtendedStatus()) {}

    Protocol::ExtendedStatus CaptPrinter::updateStatus() {
        auto ex = Protocol::PC_GET_EXTENDED_STATUS(this->stream);
        if (!ex) {
            throw UnexpectedBehaviourError(std::format("PC_GET_EXTENDED_STATUS error (0x{:02X})", static_cast<uint8_t>(ex.error())));
        }
        this->status.store(*ex);
        return *ex;
    }

    Protocol::ExtendedStatus CaptPrinter::GetStatus() {
        std::unique_lock lock(this->streamlock, std::defer_lock);
        if (lock.try_lock()) {
            return this->updateStatus();
        }
        return this->status.load();
    }

    void CaptPrinter::ReserveUnit() {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::PC_RESERVE_UNIT(this->stream));
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (!ex.UnitReserved()) {
            throw UnexpectedBehaviourError("failed to reserve unit");
        }
    }

    void CaptPrinter::ClearError() {
        std::unique_lock lock(this->streamlock);
        Protocol::ExtendedStatus ex = this->updateStatus();
        assert(ex.UnitReserved());
        CHECK_RETCODE(Protocol::PCR_CLEAR_ERROR(this->stream));
        if (ex.Misprint()) {
            CHECK_RETCODE(Protocol::PCR_CLEAR_MISPRINT(this->stream));
        }
        if ((ex.Controller & Protocol::ControllerStatus::ENGINE_COMM_ERROR) != 0) {
            CHECK_RETCODE(Protocol::PCR_RESET_ENGINE(this->stream));
        }
        if (ex.Rejected() || ex.VideoDataError()) {
            CHECK_RETCODE(Protocol::PCR_DISCARD_DATA(this->stream));
        }
    }

    bool CaptPrinter::GoOnline(unsigned page) {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::PCR_GO_ONLINE(this->stream, page));
        Protocol::ExtendedStatus ex = this->updateStatus();
        return ex.IsOnline();
    }

    void CaptPrinter::Cleaning() {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::PCR_CLEANING(this->stream));
    }

    bool CaptPrinter::WritePage(const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize) {
        std::unique_lock lock(this->streamlock);
        Protocol::IC_BEGIN_PAGE(this->stream, params);
        Protocol::IC_BEGIN_DATA(this->stream);
        while (true) {
            std::vector<uint8_t> buffer(blockSize);
            std::streamsize read = videoStream.sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if (read <= 0) {
                break;
            }
            while (true) {
                Protocol::BasicStatus bs = Protocol::PCR_GET_BASIC_STATUS(this->stream);
                if ((bs & Protocol::BasicStatus::NOT_READY) != 0) {
                    return false;
                }
                if ((bs & Protocol::BasicStatus::IM_DATA_BUSY) == 0) {
                    break;
                }
            }
            Protocol::IC_VIDEO_DATA(this->stream, {buffer.data(), static_cast<std::size_t>(read)});
        }
        Protocol::IC_END_PAGE(this->stream);
        return true;
    }

    void CaptPrinter::GoOffline() {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::PCR_GO_OFFLINE(this->stream));
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (ex.IsOnline()) {
            throw UnexpectedBehaviourError("failed to offline");
        }
    }

    void CaptPrinter::ReleaseUnit() {
        std::unique_lock lock(this->streamlock);
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (!ex.UnitReserved()) {
            return;
        }
        CHECK_RETCODE(Protocol::PCR_RELEASE_UNIT(this->stream));
        ex = this->updateStatus();
        if (ex.UnitReserved()) {
            throw UnexpectedBehaviourError("failed to reserve unit");
        }
    }

    void CaptPrinter::WaitPrintEnd() {
        std::unique_lock lock(this->streamlock);
        this->waitStatus([](Protocol::ExtendedStatus ex) {
            return !ex.IsPrinting();
        }, 1000);
    }
}
