// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "buffer_service.hpp"
#include "config.hpp"
#include "service.hpp"
#include "stream_service.hpp"
#include "version.hpp"

#include <getopt.h>

#include <phosphor-logging/log.hpp>

/** @brief Print version info. */
static void printVersion()
{
    puts("Host logger service rev." HOSTLOGGER_VERSION ".");
}

/**
 * @brief Print help usage info.
 *
 * @param[in] app application's file name
 */
static void printHelp(const char* app)
{
    printVersion();
    puts("Copyright (c) 2020 YADRO.");
    printf("Usage: %s [OPTION...]\n", app);
    puts("  -v, --version  Print version and exit");
    puts("  -h, --help     Print this help and exit");
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    // clang-format off
    const struct option longOpts[] = {
        { "version", no_argument, nullptr, 'v' },
        { "help",    no_argument, nullptr, 'h' },
        { nullptr,   0,           nullptr,  0  }
    };
    // clang-format on
    const char* shortOpts = "vh";
    opterr = 0; // prevent native error messages
    int val;
    while ((val = getopt_long(argc, argv, shortOpts, longOpts, nullptr)) != -1)
    {
        switch (val)
        {
            case 'v':
                printVersion();
                return EXIT_SUCCESS;
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Invalid argument: %s\n", argv[optind - 1]);
                return EXIT_FAILURE;
        }
    }
    if (optind < argc)
    {
        fprintf(stderr, "Unexpected argument: %s\n", argv[optind - 1]);
        return EXIT_FAILURE;
    }

    try
    {
        Config config;
        DbusLoop dbus_loop;
        HostConsole host_console(config.socketId);
        using phosphor::logging::level;
        using phosphor::logging::log;
        if (config.mode == Mode::streamMode)
        {
            log<level::INFO>("HostLogger is in stream mode.");
            StreamService service(config.streamDestination, dbus_loop,
                                  host_console);
            service.run();
        }
        else
        {
            log<level::INFO>("HostLogger is in buffer mode.");
            LogBuffer logBuffer(config.bufMaxSize, config.bufMaxTime);
            FileStorage fileStorage(config.outDir, config.socketId,
                                    config.maxFiles);
            BufferService service(config, dbus_loop, host_console, logBuffer,
                                  fileStorage);
            service.run();
        }
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "%s\n", ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
