project(
    'Math',
    'cpp',
    version: '0.1.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
    ],
    meson_version: '>=0.63.0',
)

# Header-only library: include dir + dependency object
math_inc = include_directories('include')

math_dep = declare_dependency(include_directories: math_inc)
meson.override_dependency('Math', math_dep)
install_subdir('include/', install_dir: get_option('includedir'), strip_directory: true)

pkg = import('pkgconfig')
pkg.generate(
    name: 'Math',
    filebase: 'Math',
    description: 'Header-only math library',
    version: meson.project_version()
)

math_tests = executable(
    'MathTests',
    'tests/test.cpp',
    dependencies: math_dep,
    override_options: ['cpp_std=c++20'],
)

test('MathTests', math_tests)