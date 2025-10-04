target_sources(
    libcapt
    PRIVATE
    ScoaCmd.cpp
    ScoaStreambuf.cpp

    PUBLIC
    ScoaCmd.hpp
    ScoaFunc.hpp
    ScoaState.hpp
    ScoaStreambuf.hpp
)

add_subdirectory(Decoder)
