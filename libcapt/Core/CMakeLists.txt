target_sources(
    libcapt
    PRIVATE
    PacketHeader.cpp
    PacketReader.cpp
    StreamPacket.cpp

    PUBLIC
    PacketBuilder.hpp
    PacketHeader.hpp
    PacketReader.hpp
    StreamPacket.hpp
)

add_subdirectory(Internal)
