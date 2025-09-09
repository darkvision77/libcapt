sources = files(
    'Compression/Decoder/DecoderStreambuf.cpp',
    'Compression/ScoaCmd.cpp',
    'Compression/ScoaState.cpp',
    'Compression/ScoaStreambuf.cpp',

    'Core/CaptPacket.cpp',
    'Core/PacketReader.cpp',

    'Utility/BufferedPage.cpp',
    'Utility/CropStreambuf.cpp',

    'Protocol/Protocol.cpp',

    'BasicCaptPrinter.cpp',
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

    'Utility/BufferedPage.hpp',
    'Utility/Crop.hpp',
    'Utility/CropStreambuf.hpp',

    'Protocol/Enums.hpp',
    'Protocol/ExtendedStatus.hpp',
    'Protocol/PageParams.hpp',
    'Protocol/PrinterInfo.hpp',
    'Protocol/Protocol.hpp',
    'Protocol/ProtocolError.hpp',
    'Protocol/ReprintStatus.hpp',

    'BasicCaptPrinter.hpp',
    'UnexpectedBehaviourError.hpp',
)

static_subproject = get_option('default_library') == 'static' and meson.is_subproject()

include = include_directories('.')
if not static_subproject
    install_headers(headers, preserve_path: true, subdir: 'libcapt')
endif

libcapt = library(
    'libcapt',
    sources,
    include_directories: include,
    name_prefix: '',
    install: not static_subproject,
)

libcapt_dep = declare_dependency(
    include_directories: include_directories('..'),
    link_with: libcapt,
)
