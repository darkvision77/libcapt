#include "CaptPrinter.hpp"
#include "CaptPrinterError.hpp"
#include "Protocol/Enums.hpp"
#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include "Protocol/Protocol.hpp"
#include <format>
#include <mutex>
#include <cassert>

namespace Capt {
    #define CHECK_RETCODE(EXP) checkRetcode(EXP, #EXP)
    static inline void checkRetcode(uint8_t cmdResult, std::string paramName) {
        if (cmdResult != 0) {
            throw CaptPrinterError(std::format("Critical protocol error: {} returned non-successfull code (0x{:02X})", paramName, cmdResult));
        }
    }

    CaptPrinter::CaptPrinter(std::iostream& stream) : stream(stream), status(Protocol::ExtendedStatus()) {}

    Protocol::ExtendedStatus CaptPrinter::updateStatus() {
        Protocol::ExtendedStatus ex = Protocol::GetExtendedStatus(this->stream);
        this->status.store(ex);
        return ex;
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
        CHECK_RETCODE(Protocol::ReserveUnit(this->stream));
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (!ex.UnitReserved()) {
            throw CaptPrinterError("Critical protocol error: unexpected behaviour (failed to reserve unit)");
        }
    }

    void CaptPrinter::ClearError() {
        std::unique_lock lock(this->streamlock);
        Protocol::ExtendedStatus ex = this->updateStatus();
        assert(ex.UnitReserved());
        CHECK_RETCODE(Protocol::ClearError(this->stream));
        if (ex.Misprint()) {
            CHECK_RETCODE(Protocol::ClearMisprint(this->stream));
        }
        if ((ex.Controller & Protocol::ControllerStatus::ENGINE_COMM_ERROR) != 0) {
            CHECK_RETCODE(Protocol::ResetEngine(this->stream));
        }
        if (ex.Rejected() || ex.VideoDataError()) {
            CHECK_RETCODE(Protocol::DiscardData(this->stream));
        }
    }

    bool CaptPrinter::GoOnline(unsigned page) {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::GoOnline(this->stream, page));
        Protocol::ExtendedStatus ex = this->updateStatus();
        return ex.IsOnline();
    }

    bool CaptPrinter::WritePage(Protocol::PageParams params, std::streambuf& videoStream, std::size_t blockSize) {
        std::unique_lock lock(this->streamlock);
        Protocol::BeginPage(this->stream, params);
        Protocol::BeginData(this->stream);
        while (true) {
            std::vector<uint8_t> buffer(blockSize);
            std::streamsize read = videoStream.sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if (read == 0) {
                break;
            }
            while (true) {
                Protocol::BasicStatus bs = Protocol::GetBasicStatus(this->stream);
                if ((bs & Protocol::BasicStatus::NOT_READY) != 0) {
                    return false;
                }
                if ((bs & Protocol::BasicStatus::IM_DATA_BUSY) == 0) {
                    break;
                }
            }
            Protocol::VideoData(this->stream, buffer.data(), read);
        }
        Protocol::EndPage(this->stream);
        return true;
    }

    void CaptPrinter::GoOffline() {
        std::unique_lock lock(this->streamlock);
        CHECK_RETCODE(Protocol::GoOffline(this->stream));
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (ex.IsOnline()) {
            throw CaptPrinterError("Critical protocol error: unexpected behaviour (failed to go offline)");
        }
    }

    void CaptPrinter::ReleaseUnit() {
        std::unique_lock lock(this->streamlock);
        Protocol::ExtendedStatus ex = this->updateStatus();
        if (!ex.UnitReserved()) {
            return;
        }
        CHECK_RETCODE(Protocol::ReleaseUnit(this->stream));
        ex = this->updateStatus();
        if (ex.UnitReserved()) {
            throw CaptPrinterError("Critical protocol error: unexpected behaviour (failed to reserve unit)");
        }
    }

    void CaptPrinter::WaitPrintEnd() {
        std::unique_lock lock(this->streamlock);
        this->waitStatus([](Protocol::ExtendedStatus ex) {
            return !ex.IsPrinting();
        }, 1000);
    }
}
