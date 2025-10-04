cmake_minimum_required(VERSION 3.20)
message(STATUS "Using CMake v${CMAKE_VERSION}")

set(LIBCAPT_VERSION 0.12.1)
project(
    libcapt
    VERSION ${LIBCAPT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(COMPRESSION_TESTS "Build compressor and decoder tests" OFF)
option(COVERAGE "Enable code coverage" OFF)
option(SANITIZE "Enable address and undefined sanitizers" OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(COVERAGE)
    add_compile_options(--coverage -g)
    add_link_options(--coverage)
endif()

if(SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

add_subdirectory(libcapt)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
