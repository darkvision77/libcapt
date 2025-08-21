#ifndef _LIBCAPT_UTILITY_CROP_HPP_
#define _LIBCAPT_UTILITY_CROP_HPP_

#include <algorithm>
#include <cstddef>

namespace Capt::Utility {
    constexpr std::size_t CropLineSize(std::size_t lineSize) noexcept {
        return lineSize - (lineSize % 4);
    }

    constexpr std::size_t CropLineSize(std::size_t lineSize, std::size_t paperWidth) noexcept {
        return CropLineSize(std::min(lineSize, paperWidth / 8));
    }

    constexpr std::size_t CropLinesCount(std::size_t count) noexcept {
        return count - (count % 32);
    }

    constexpr std::size_t CropLinesCount(std::size_t count, std::size_t paperHeight) noexcept {
        return CropLinesCount(std::min(count, paperHeight));
    }
}

#endif
