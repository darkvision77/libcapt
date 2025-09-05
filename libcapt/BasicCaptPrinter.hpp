#ifndef _LIBCAPT_BASIC_CAPT_PRINTER_HPP_
#define _LIBCAPT_BASIC_CAPT_PRINTER_HPP_

#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include "Protocol/PrinterInfo.hpp"
#include <istream>
#include <chrono>
#include <optional>
#include <stop_token>
#include <thread>

namespace Capt {
    class BasicCaptPrinter {
    protected:
        std::iostream& stream;
        std::optional<Protocol::PrinterInfo> cachedInfo;
    public:
        explicit BasicCaptPrinter(std::iostream& stream) noexcept;
        virtual ~BasicCaptPrinter() noexcept = default;

        virtual Protocol::ExtendedStatus GetStatus();
        virtual Protocol::PrinterInfo GetPrinterInfo();

        virtual void ReserveUnit();
        virtual void ClearError(const Protocol::ExtendedStatus* status = nullptr);
        virtual bool GoOnline(unsigned page);
        virtual bool Cleaning();

        // If blockSize is zero, it will be taken from PrinterInfo
        virtual bool WriteVideoData(std::stop_token stopToken, const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize = 0);
        inline virtual bool WriteVideoData(const Protocol::PageParams& params, std::streambuf& videoStream, std::size_t blockSize = 0) {
            return this->WriteVideoData(std::stop_token{}, params, videoStream, blockSize);
        }

        virtual void GoOffline();
        virtual void ReleaseUnit();

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
        inline virtual std::optional<Protocol::ExtendedStatus> WaitPrintEnd(std::stop_token stopToken) {
            return this->WaitStatus(stopToken, [](Protocol::ExtendedStatus ex) {
                return !ex.IsPrinting();
            }, std::chrono::seconds(1));
        }

        inline virtual Protocol::ExtendedStatus WaitPrintEnd() {
            return this->WaitPrintEnd(std::stop_token{}).value();
        }
    };
}

#endif
