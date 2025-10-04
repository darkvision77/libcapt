target_sources(
    libcapt
    PRIVATE
    ExtendedStatus.cpp
    Protocol.cpp

    PUBLIC
    Enums.hpp
    ExtendedStatus.hpp
    PageParams.hpp
    PrinterInfo.hpp
    Protocol.hpp
    ProtocolError.hpp
    ReprintStatus.hpp
)
