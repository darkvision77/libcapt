tests = [
    'CaptPacketTest',
    'PacketBuilderTest',
    'PacketReaderTest',
]

gtest_dep = dependency('gtest', main: true)
gmock_dep = dependency('gmock')

foreach t : tests
    exe = executable(
        t,
        '@0@.cpp'.format(t),
        dependencies: [libcapt_dep, gtest_dep, gmock_dep],
        install: false,
    )
    test(t, exe, protocol: 'gtest', suite: ['general', 'core'])
endforeach
