#ifndef _LIBCAPT_CAPT_PRINTER_HPP_
#define _LIBCAPT_CAPT_PRINTER_HPP_

#include "Protocol/ExtendedStatus.hpp"
#include "Protocol/PageParams.hpp"
#include <atomic>
#include <istream>
#include <mutex>
#include <chrono>
#include <thread>

namespace Capt {
    class CaptPrinter {
    private:
        std::iostream& stream;
        std::mutex streamlock;
        std::atomic<Protocol::ExtendedStatus> status;

        template<typename TFunc>
        Protocol::ExtendedStatus waitStatus(TFunc func, unsigned delayMs) {
            while (true) {
                Protocol::ExtendedStatus ex = this->updateStatus();
                if (func(ex)) {
                    return ex;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }
        }

        Protocol::ExtendedStatus updateStatus();

    public:
        explicit CaptPrinter(std::iostream& stream);

        Protocol::ExtendedStatus GetStatus();
        void ReserveUnit();
        void ClearError();
        bool GoOnline(unsigned page);
        bool WritePage(Protocol::PageParams params, std::streambuf& videoStream, std::size_t blockSize = 4096);
        void GoOffline();
        void ReleaseUnit();
        void WaitPrintEnd();
    };
}

#endif
