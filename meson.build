project(
    'libcapt',
    'cpp',
    version: '0.1.1',
    license: 'BSD-2-Clause',
    license_files: ['LICENSE'],
    default_options: [
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
        'warning_level=3',
        'cpp_std=c++23',
        'b_lundef=false',
    ],
    meson_version: '>=1.4.0',
)

subdir('libcapt')
subdir('tests')
subdir('examples')
