# Rules for building tests

test(
    'hostlogger',
    executable(
        'hostlogger_test',
        [
            'config_test.cpp',
            'file_storage_test.cpp',
            'host_console_test.cpp',
            'log_buffer_test.cpp',
            'buffer_service_test.cpp',
            'stream_service_test.cpp',
            'zlib_file_test.cpp',
            '../src/buffer_service.cpp',
            '../src/config.cpp',
            '../src/dbus_loop.cpp',
            '../src/file_storage.cpp',
            '../src/host_console.cpp',
            '../src/log_buffer.cpp',
            '../src/stream_service.cpp',
            '../src/zlib_exception.cpp',
            '../src/zlib_file.cpp',
        ],
        dependencies: [
            dependency(
                'gtest',
                main: true,
                disabler: true,
                required: build_tests,
            ),
            dependency('gmock', disabler: true, required: build_tests),
            dependency('zlib'),
            dependency('phosphor-logging'),
        ],
        cpp_args: ['-DSTREAM_SERVICE', '-DBUFFER_SERVICE'],
        include_directories: '../src',
    ),
)
