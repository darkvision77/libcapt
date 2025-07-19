if get_option('examples').disabled()
    subdir_done()
endif

executable(
    'printpbm',
    sources: files('printpbm.cpp'),
    dependencies: [libcapt_dep, atomic_dep],
    install: false,
)
