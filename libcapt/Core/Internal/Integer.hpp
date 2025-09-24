#ifndef _LIBCAPT_CORE_INTERNAL_INTEGER_HPP_
#define _LIBCAPT_CORE_INTERNAL_INTEGER_HPP_

#include <bit>
#include <concepts>
#include <istream>

namespace Capt::Internal {
    template<std::integral T>
    [[nodiscard]] constexpr T LittleToCpu(T value) noexcept {
        static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little);
        if constexpr (std::endian::native == std::endian::big) {
            return std::byteswap(value);
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
