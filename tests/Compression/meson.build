pdfseparate = find_program('pdfseparate')
pdfinfo = find_program('pdfinfo')
testdecscript = find_program('testdecoder.sh')
testcompscript = find_program('testcompressor.sh')
pdf2pbm = find_program(meson.project_source_root() / 'scripts/pdf2pbm.sh')
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

pdffiles = files(
    'data/stm32g030f6.pdf',
)

pbmfiles = files(
    'data/randA.pbm',
    'data/randB.pbm',
    'data/randC.pbm',
    'data/randD.pbm',
    'data/randE.pbm',
    'data/randF.pbm',
    'data/empty.pbm',
)

foreach file : pdffiles
    r = run_command(pdfinfo, file, check: true)
    pages = -1
    foreach line : r.stdout().splitlines()
        if line.startswith('Pages:')
            pages = line.split()[1].to_int()
            break
        endif
    endforeach
    if pages == -1
        error('cannot get pages count')
    endif

    outfiles = []
    foreach i : range(1, pages+1)
        outfiles += '@0@-page@1@.pdf'.format(file.full_path().split('/')[-1].split('.')[0], i.to_string(fill: 4))
    endforeach

    pdfsep_tgt = custom_target(
        input: file,
        output: outfiles,
        command: [pdfseparate, '@INPUT@', '@OUTDIR@/@BASENAME@-page%04d.pdf'],
        install: false,
    )
    separatedfiles = pdfsep_tgt.to_list()

    foreach pagefile : separatedfiles
        pagepbm_tgt = custom_target(
            input: pagefile,
            output: '@BASENAME@.pbm',
            command: [pdf2pbm, '@INPUT@', '@OUTPUT@'],
            install: false,
            depends: separatedfiles,
        )
        pbmfiles += pagepbm_tgt[0]
    endforeach
endforeach

foreach pbm : pbmfiles
    basename = pbm.full_path().split('/')[-1].split('.')[0]
    pagefilt_tgt = custom_target(
        input: pbm,
        output: ['@BASENAME@.log', '@BASENAME@.bin'],
        command: [filter, '@INPUT@', '@OUTPUT@'],
        install: false,
    )
    test(
        'TestDecoder-'+basename,
        testdecscript,
        args: [decoder_exe.full_path(), pagefilt_tgt[1], pbm, pagefilt_tgt[0]],
        is_parallel: true,
        depends: [decoder_exe, pagefilt_tgt],
        suite: ['compression', 'decoder'],
    )
    test(
        'TestCompressor-'+basename,
        testcompscript,
        args: [compressor_exe.full_path(), decoder_exe.full_path(), pbm],
        is_parallel: true,
        depends: [compressor_exe, decoder_exe],
        suite: ['compression', 'compressor'],
    )
endforeach
