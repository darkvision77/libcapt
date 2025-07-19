sources = files(
    'Compression/Decoder/DecoderStreambuf.cpp',
    'Compression/ScoaCmd.cpp',
    'Compression/ScoaState.cpp',
    'Compression/ScoaStreambuf.cpp',

    'Core/CaptPacket.cpp',
    'Core/PacketBuilder.cpp',
    'Core/PacketReader.cpp',

    'Protocol/Protocol.cpp',

    'CaptPrinter.cpp',
)

headers = files(
    'Compression/Decoder/DecoderError.hpp',
    'Compression/Decoder/DecoderStreambuf.hpp',
    'Compression/ScoaCmd.hpp',
    'Compression/ScoaState.hpp',
    'Compression/ScoaStreambuf.hpp',

    'Core/CaptPacket.hpp',
    'Core/PacketBuilder.hpp',
    'Core/PacketReader.hpp',

    'Protocol/Enums.hpp',
    'Protocol/ExtendedStatus.hpp',
    'Protocol/PageParams.hpp',
    'Protocol/Protocol.hpp',

    'CaptPrinter.hpp',
    'CaptPrinterError.hpp',
)

include = include_directories('.')
install_headers(headers, preserve_path: true, subdir: 'libcapt')

atomic_dep = dependency('atomic')

libcapt = both_libraries(
    'libcapt',
    sources,
    dependencies: [atomic_dep],
    include_directories: include,
    name_prefix: '',
    install: true,
)

libcapt_dep = declare_dependency(
    include_directories: include_directories('..'),
    link_with: libcapt,
)
