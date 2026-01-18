cmake_minimum_required(VERSION 3.21)
message(STATUS "Using CMake v${CMAKE_VERSION}")

project(
    libcapt
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LIBCAPT_BUILD_EXAMPLES "Build examples" OFF)
option(LIBCAPT_BUILD_TESTS "Build tests" OFF)
option(LIBCAPT_COMPRESSION_TESTS "Build compressor and decoder tests" OFF)
option(LIBCAPT_COVERAGE "Enable code coverage" OFF)
option(LIBCAPT_SANITIZE "Enable address and undefined sanitizers" OFF)
option(LIBCAPT_INSTALL "Install targets" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(LIBCAPT_COVERAGE)
    include(Coverage)
endif()

if(LIBCAPT_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(libcapt)

if(LIBCAPT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LIBCAPT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set(LIBCAPT_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/libcapt")

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/libcaptConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libcaptConfig.cmake"
    INSTALL_DESTINATION "${LIBCAPT_CONFIG_PATH}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libcaptConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

if(LIBCAPT_INSTALL)
    install(
        EXPORT libcaptExports
        NAMESPACE libcapt::
        FILE libcaptTargets.cmake
        DESTINATION "${LIBCAPT_CONFIG_PATH}"
    )

    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libcaptConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libcaptConfigVersion.cmake"
        DESTINATION "${LIBCAPT_CONFIG_PATH}"
    )
endif()
