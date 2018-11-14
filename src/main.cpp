/**
 * @brief Host logger service entry point.
 *
 * This file is part of HostLogger project.
 *
 * Copyright (c) 2018 YADRO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.hpp"
#include "dbus_server.hpp"
#include "dbus_watch.hpp"
#include "log_manager.hpp"

#include <getopt.h>

#include <climits>
#include <cstdio>
#include <cstdlib>

// Global logger configuration instance
Config loggerConfig = {.path = LOG_OUTPUT_PATH,
                       .storageSizeLimit = LOG_STORAGE_SIZE_LIMIT,
                       .storageTimeLimit = LOG_STORAGE_TIME_LIMIT,
                       .flushPeriod = LOG_FLUSH_PERIOD,
                       .rotationLimit = LOG_ROTATION_LIMIT};

/** @brief Print title with version info. */
static void printTitle()
{
    printf("Host logger service " PACKAGE_VERSION ".\n");
}

/** @brief Print help usage info.
 *
 *  @param[in] app - application's file name
 */
static void printHelp(const char* app)
{
    printTitle();
    printf("Copyright (c) 2018 YADRO.\n");
    printf("Usage: %s [options]\n", app);
    printf(
        "Options (defaults are specified in brackets):\n"
        "  -p, --path=PATH   Path used to store logs [%s]\n"
        "Intermediate storage buffer capacity setup:\n"
        "  -s, --szlimit=N   Store up to N last messages [%i], 0=unlimited\n"
        "  -t, --tmlimit=N   Store messages for last N hours [%i], "
        "0=unlimited\n"
        "Flush storage buffer policy:\n"
        "  -f, --flush=N     Flush logs every N hours [%i]\n"
        "                    If this option is set to 0 flush will be called "
        "at\n"
        "                    every host state change event from D-Bus.\n"
        "Log files rotation policy:\n"
        "  -r, --rotate=N    Store up to N files in the log directory [%i],\n"
        "                    0=unlimited\n"
        "Common options:\n"
        "  -v, --version     Print version and exit\n"
        "  -h, --help        Print this help and exit\n",
        loggerConfig.path, loggerConfig.storageSizeLimit,
        loggerConfig.storageTimeLimit, loggerConfig.flushPeriod,
        loggerConfig.rotationLimit);
}

/** @brief Get numeric positive value from string argument.
 *
 *  @param[in] param - parameter name
 *  @param[in] arg - parameter text value
 *
 *  @return positive numeric value from string argument or -1 on errors
 */
static int getNumericArg(const char* param, const char* arg)
{
    char* ep = nullptr;
    const unsigned long val = strtoul(arg, &ep, 0);
    if (val > INT_MAX || !ep || ep == arg || *ep != 0)
    {
        fprintf(stderr, "Invalid %s param: %s, expected 0<=N<=%i\n", param, arg,
                INT_MAX);
        return -1;
    }
    return static_cast<int>(val);
}

/** @brief Application entry point. */
int main(int argc, char* argv[])
{
    int opt_val;
    // clang-format off
    const struct option opts[] = {
        { "path",    required_argument, 0, 'p' },
        { "szlimit", required_argument, 0, 's' },
        { "tmlimit", required_argument, 0, 't' },
        { "flush",   required_argument, 0, 'f' },
        { "rotate",  required_argument, 0, 'r' },
        { "version", no_argument,       0, 'v' },
        { "help",    no_argument,       0, 'h' },
        { 0,         0,                 0,  0  }
    };
    // clang-format on

    opterr = 0;
    while ((opt_val = getopt_long(argc, argv, "p:s:t:f:r:vh", opts, NULL)) !=
           -1)
    {
        switch (opt_val)
        {
            case 'p':
                loggerConfig.path = optarg;
                if (*loggerConfig.path != '/')
                {
                    fprintf(stderr,
                            "Invalid directory: %s, expected absolute path\n",
                            loggerConfig.path);
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                loggerConfig.storageSizeLimit =
                    getNumericArg(opts[optind - 1].name, optarg);
                if (loggerConfig.storageSizeLimit < 0)
                    return EXIT_FAILURE;
                break;
            case 't':
                loggerConfig.storageTimeLimit =
                    getNumericArg(opts[optind - 1].name, optarg);
                if (loggerConfig.storageTimeLimit < 0)
                    return EXIT_FAILURE;
                break;
            case 'f':
                loggerConfig.flushPeriod =
                    getNumericArg(opts[optind - 1].name, optarg);
                if (loggerConfig.flushPeriod < 0)
                    return EXIT_FAILURE;
                break;
            case 'r':
                loggerConfig.rotationLimit =
                    getNumericArg(opts[optind - 1].name, optarg);
                if (loggerConfig.rotationLimit < 0)
                    return EXIT_FAILURE;
                break;
            case 'v':
                printTitle();
                return EXIT_SUCCESS;
            case 'h':
                printHelp(argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Invalid option: %s\n", argv[optind - 1]);
                return EXIT_FAILURE;
        }
    }

    int rc;

    // Initialize log manager
    LogManager logManager;
    rc = logManager.openHostLog();
    if (rc != 0)
        return rc;

    // Initialize D-Bus server
    sdbusplus::bus::bus bus = sdbusplus::bus::new_default();
    sd_event* event = nullptr;
    rc = sd_event_default(&event);
    if (rc < 0)
    {
        fprintf(stderr, "Error occurred during the sd_event_default: %i\n", rc);
        return EXIT_FAILURE;
    }
    EventPtr eventPtr(event);
    bus.attach_event(eventPtr.get(), SD_EVENT_PRIORITY_NORMAL);

    DbusServer dbusMgr(logManager, bus);
    bus.request_name(HOSTLOGGER_DBUS_IFACE);

    // Initialize D-Bus watcher
    DbusWatcher dbusWatch(logManager, bus);
    rc = dbusWatch.initialize();
    if (rc < 0)
        return EXIT_FAILURE;

    // D-Bus event processing
    rc = sd_event_loop(eventPtr.get());
    if (rc != 0)
        fprintf(stderr, "Error occurred during the sd_event_loop: %i\n", rc);

    return rc ? rc : -1; // Allways retrun an error code
}
