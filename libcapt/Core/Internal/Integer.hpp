#ifndef _LIBCAPT_CORE_INTERNAL_INTEGER_HPP_
#define _LIBCAPT_CORE_INTERNAL_INTEGER_HPP_

#include "Config.hpp"
#include <bit>
#include <concepts>
#include <istream>

#if !HAVE_BYTESWAP
#include <cstdint>
#endif

namespace Capt::Internal {
    namespace impl {
        #if !HAVE_BYTESWAP
        constexpr uint16_t bswap(uint16_t value) noexcept { return __builtin_bswap16(value); }
        constexpr uint32_t bswap(uint32_t value) noexcept { return __builtin_bswap32(value); }
        constexpr uint64_t bswap(uint64_t value) noexcept { return __builtin_bswap64(value); }
        #else
        template<std::integral T>
        constexpr T bswap(T value) noexcept {
            return std::byteswap(value);
        }
        #endif
    }

    template<std::integral T>
    [[nodiscard]] constexpr T LittleToCpu(T value) noexcept {
        static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little);
        if constexpr (std::endian::native == std::endian::big) {
            return impl::bswap(value);
        } else {
            return value;
        }
    }

    template<std::integral T>
    inline T ReadLittle(std::istream& stream) {
        if (!stream.good()) {
            return 0;
        }
        T value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return LittleToCpu(value);
    }

    template<std::integral T>
    inline std::ostream& WriteLittle(std::ostream& stream, T value) {
        if (stream.good()) {
            value = LittleToCpu(value);
            stream.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
        return stream;
    }
}

#endif
