testdecscript = find_program('testdecoder.sh')
testcompscript = find_program('testcompressor.sh')
filter = find_program(meson.project_source_root() / 'scripts/filter/filter.sh')

decoder_exe = executable(
    'decoder',
    'decoder.cpp',
    dependencies: [libcapt_dep],
    install: false,
)
compressor_exe = executable(
    'compressor',
    'compressor.cpp',
    dependencies: [libcapt_dep],
    install: false,
)

subdir('data')

foreach pbm : pbmfiles
    basename = pbm.full_path().split('/')[-1].split('.')[0]
    pagefilt_tgt = custom_target(
        input: pbm,
        output: ['@BASENAME@.log', '@BASENAME@.bin'],
        command: [filter, '@INPUT@', '@OUTPUT@'],
        build_by_default: true,
        install: false,
    )
    test(
        'TestDecoder-'+basename,
        testdecscript,
        args: [decoder_exe, pagefilt_tgt[1], pbm, pagefilt_tgt[0]],
        is_parallel: true,
        depends: [decoder_exe, pagefilt_tgt],
        suite: ['compression', 'decoder'],
        timeout: 0,
    )
    test(
        'TestCompressor-'+basename,
        testcompscript,
        args: [compressor_exe, decoder_exe, pbm],
        is_parallel: true,
        depends: [compressor_exe, decoder_exe],
        suite: ['compression', 'compressor'],
        timeout: 0,
    )
endforeach
