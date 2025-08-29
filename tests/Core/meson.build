tests = [
    'BufferedPageTest',
    'CaptPacketTest',
    'CropStreambufTest',
    'PacketBuilderTest',
    'PacketReaderTest',
    'ProtocolTest',
    'ScoaStreambufTest',
]

gtest_dep = dependency('gtest_main')
gmock_dep = dependency('gmock')

foreach t : tests
    exe = executable(
        t,
        '@0@.cpp'.format(t),
        dependencies: [libcapt_dep, gtest_dep, gmock_dep],
        install: false,
    )
    test(t, exe, protocol: 'gtest', suite: ['core'])
endforeach
