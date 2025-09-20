sources = files(
    'BasicCaptPrinter.cpp',
)

headers = files(
    'BasicCaptPrinter.hpp',
    'UnexpectedBehaviourError.hpp',
)

subdir('Compression')
subdir('Core')
subdir('Protocol')
subdir('Utility')

static_subproject = get_option('default_library') == 'static' and meson.is_subproject()

include = include_directories('.')
if not static_subproject
    fs = import('fs')
    headers_rel = []
    foreach file : headers
        headers_rel += fs.relative_to(file, '')
    endforeach
    install_headers(headers_rel, preserve_path: true, subdir: 'libcapt')
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
