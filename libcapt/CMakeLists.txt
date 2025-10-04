add_library(libcapt)
set_target_properties(libcapt PROPERTIES PREFIX "")
target_include_directories(libcapt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libcapt PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

target_sources(
    libcapt
    PRIVATE
    BasicCaptPrinter.cpp

    PUBLIC
    BasicCaptPrinter.hpp
    UnexpectedBehaviourError.hpp
)

add_subdirectory(Compression)
add_subdirectory(Core)
add_subdirectory(Protocol)
add_subdirectory(Utility)

install(TARGETS libcapt)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.hpp"
)
