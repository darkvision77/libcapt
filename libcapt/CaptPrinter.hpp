#ifndef _LIBCAPT_CAPT_PRINTER_HPP_
#define _LIBCAPT_CAPT_PRINTER_HPP_

#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include "Protocol/PrinterInfo.hpp"
#include <atomic>
#include <istream>
#include <mutex>
#include <chrono>
#include <optional>
#include <thread>

namespace Capt {
    class CaptPrinter {
    private:
        std::iostream& stream;
        std::mutex streamlock;
        std::atomic<Protocol::ExtendedStatus> status;
        std::optional<Protocol::PrinterInfo> cachedInfo;

        Protocol::ExtendedStatus updateStatus();
    public:
        explicit CaptPrinter(std::iostream& stream) noexcept;

        Protocol::ExtendedStatus GetStatus();
        Protocol::PrinterInfo GetPrinterInfo();

        void ReserveUnit();
        void ClearError(const Protocol::ExtendedStatus* status = nullptr);
        bool GoOnline(unsigned page);
        void Cleaning();

        // If blockSize is zero, it will be taken from PrinterInfo
        bool WriteVideoData(const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize = 0);

        void GoOffline();
        void ReleaseUnit();

        template<typename TFunc, typename Rep, typename Period>
        Protocol::ExtendedStatus WaitStatus(TFunc func, const std::chrono::duration<Rep, Period>& delay) {
            while (true) {
                Protocol::ExtendedStatus ex = this->updateStatus();
                if (func(ex)) {
                    return ex;
                }
                std::this_thread::sleep_for(delay);
            }
        }

        void WaitPrintEnd();
    };
}

#endif
