pdffiles = files(
    'stm32g030f6.pdf',
)

pbmfiles = files(
    'randA.pbm',
    'randB.pbm',
    'randC.pbm',
    'randD.pbm',
    'randE.pbm',
    'randF.pbm',
    'empty.pbm',
    'black-1.pbm',
    'repx-1.pbm',
)

pdfseparate = find_program('pdfseparate')
pdfinfo = find_program('pdfinfo')
pdf2pbm = find_program(meson.project_source_root() / 'scripts/pdf2pbm.sh')

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
        pbmfiles += pagepbm_tgt
    endforeach
endforeach
