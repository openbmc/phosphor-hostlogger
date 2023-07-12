# Rules for building with Meson

project(
  'hostlogger',
  'cpp',
  default_options: [
   'warning_level=3',
   'werror=true',
   'cpp_std=c++23',
  ],
  license: 'Apache-2.0',
  meson_version: '>=1.1.1',
)

# version info from git
version = vcs_tag(command: [ 'git', 'describe', '--always', '--dirty', '--long' ],
                  input: 'src/version.hpp.in',
                  output: 'version.hpp')

# unit tests
build_tests = get_option('tests')
subdir('test')

# install systemd unit template file
systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_variable(
  'systemdsystemunitdir',
  pkgconfig_define: ['prefix', get_option('prefix')],
)
configure_file(
  copy: true,
  input: 'hostlogger@.service',
  install: true,
  install_dir: systemd_system_unit_dir,
  output: 'hostlogger@.service'
)

executable(
  'hostlogger',
  [
    version,
    'src/config.cpp',
    'src/dbus_loop.cpp',
    'src/file_storage.cpp',
    'src/host_console.cpp',
    'src/log_buffer.cpp',
    'src/main.cpp',
    'src/buffer_service.cpp',
    'src/stream_service.cpp',
    'src/zlib_exception.cpp',
    'src/zlib_file.cpp',
  ],
  dependencies: [
    systemd,
    dependency('phosphor-logging'),
    dependency('zlib'),
  ],
  install: true
)
