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
#include <stop_token>
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
        bool WriteVideoData(std::stop_token stopToken, const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize = 0);
        inline bool WriteVideoData(const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize = 0) {
            return this->WriteVideoData(std::stop_token{}, params, videoStream, blockSize);
        }

        void GoOffline();
        void ReleaseUnit();

        // nullopt if stop requested
        template<typename TFunc, typename Rep, typename Period>
        std::optional<Protocol::ExtendedStatus> WaitStatus(std::stop_token stopToken, TFunc func, const std::chrono::duration<Rep, Period>& delay) {
            while (!stopToken.stop_requested()) {
                Protocol::ExtendedStatus ex = this->GetStatus();
                if (func(ex)) {
                    return ex;
                }
                std::this_thread::sleep_for(delay);
            }
            return std::nullopt;
        }

        template<typename TFunc, typename Rep, typename Period>
        Protocol::ExtendedStatus WaitStatus(TFunc func, const std::chrono::duration<Rep, Period>& delay) {
            return this->WaitStatus(std::stop_token{}, func, delay).value();
        }

        // nullopt if stop requested
        inline std::optional<Protocol::ExtendedStatus> WaitPrintEnd(std::stop_token stopToken) {
            return this->WaitStatus(stopToken, [](Protocol::ExtendedStatus ex) {
                return !ex.IsPrinting();
            }, std::chrono::seconds(1));
        }

        inline Protocol::ExtendedStatus WaitPrintEnd() {
            return this->WaitPrintEnd(std::stop_token{}).value();
        }
    };
}

#endif
