// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#include "config.hpp"
#include "dbus_loop.hpp"
#include "host_console.hpp"
#include "stream_service.hpp"

#include <phosphor-logging/log.hpp>

#include <cstdlib>

/** @brief Application entry point. */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    using host_logger::stream::Service;
    using phosphor::logging::entry;
    using phosphor::logging::level;
    using phosphor::logging::log;

    try
    {
        Config config;
        auto dbusLoop = std::make_unique<DbusLoop>();
        auto hostConsole = std::make_unique<HostConsole>(config.socketId);
        Service service(config.streamDestination, std::move(dbusLoop),
                        std::move(hostConsole));
        log<level::DEBUG>("Initialization complete",
                          entry("SocketId=%s", config.socketId),
                          entry("StreamDst=%s", config.streamDestination));
        service.run();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "%s\n", ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
