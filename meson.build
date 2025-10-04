project(
    'libcapt',
    'cpp',
    version: '0.12.1',
    license: 'BSD-2-Clause',
    license_files: ['LICENSE'],
    default_options: [
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
        'warning_level=3',
        'cpp_std=c++23',
        'b_lundef=false',
    ],
    meson_version: '>=1.5.0',
)

cpp = meson.get_compiler('cpp')

cpp.compiles('''
#include <stop_token>
int main() { std::stop_token token; return 0; }
''', name: 'std::stop_token support', required: true)

subdir('libcapt')
subdir('tests')
subdir('examples')
