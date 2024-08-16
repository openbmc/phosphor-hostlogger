// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "buffer_service.hpp"

#include <phosphor-logging/log.hpp>

#include <vector>

using namespace phosphor::logging;

// clang-format off
/** @brief Host state monitor properties.
 *  Used for automatic flushing the log buffer to the persistent file.
 *  Contains a list of properties and a set of their values that trigger the
 *  flush operation.
 *  For example, the current log buffer will be saved to a file when the
 *  "OperatingSystemState" property obtains one of the
 *  listed values ("xyz.openbmc_project...BootComplete", "Inactive", etc).
 */
static const DbusLoop::WatchProperties watchProperties{
  {"xyz.openbmc_project.State.Host", {{
    "RequestedHostTransition", {
      "xyz.openbmc_project.State.Host.Transition.On"}}}},
  {"xyz.openbmc_project.State.OperatingSystem.Status", {{
    "OperatingSystemState", {
      "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.BootComplete",
      "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Inactive",
      "Inactive",
      "Standby"}}}}
};
// clang-format on

BufferService::BufferService(const Config& config, DbusLoop& dbusLoop,
                             HostConsole& hostConsole, LogBuffer& logBuffer,
                             FileStorage& fileStorage) :
    config(config), dbusLoop(&dbusLoop), hostConsole(&hostConsole),
    logBuffer(&logBuffer), fileStorage(&fileStorage)
{}

void BufferService::run()
{
    if (config.bufFlushFull)
    {
        logBuffer->setFullHandler([this]() { this->flush(); });
    }

    hostConsole->connect();

    // Add SIGUSR1 signal handler for manual flushing
    dbusLoop->addSignalHandler(SIGUSR1, [this]() { this->flush(); });
    // Add SIGTERM signal handler for service shutdown
    dbusLoop->addSignalHandler(SIGTERM, [this]() { this->dbusLoop->stop(0); });

    // Register callback for socket IO
    dbusLoop->addIoHandler(*hostConsole, [this]() { this->readConsole(); });

    // Register host state watcher
    if (*config.hostState)
    {
        dbusLoop->addPropertyHandler(config.hostState, watchProperties,
                                     [this]() { this->flush(); });
    }

    if (!*config.hostState && !config.bufFlushFull)
    {
        log<level::WARNING>("Automatic flush disabled");
    }

    log<level::DEBUG>(
        "Initialization complete", entry("SocketId=%s", config.socketId),
        entry("BufMaxSize=%lu", config.bufMaxSize),
        entry("BufMaxTime=%lu", config.bufMaxTime),
        entry("BufFlushFull=%s", config.bufFlushFull ? "y" : "n"),
        entry("HostState=%s", config.hostState),
        entry("OutDir=%s", config.outDir),
        entry("MaxFiles=%lu", config.maxFiles));

    // Run D-Bus event loop
    const int rc = dbusLoop->run();
    if (!logBuffer->empty())
    {
        flush();
    }
    if (rc < 0)
    {
        std::error_code ec(-rc, std::generic_category());
        throw std::system_error(ec, "Error in event loop");
    }
}

void BufferService::flush()
{
    if (logBuffer->empty())
    {
        log<level::INFO>("Ignore flush: buffer is empty");
        return;
    }
    try
    {
        const std::string fileName = fileStorage->save(*logBuffer);
        logBuffer->clear();

        std::string msg = "Host logs flushed to ";
        msg += fileName;
        log<level::INFO>(msg.c_str());
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(ex.what());
    }
}

void BufferService::readConsole()
{
    constexpr size_t bufSize = 128; // enough for most line-oriented output
    std::vector<char> bufData(bufSize);
    char* buf = bufData.data();

    try
    {
        while (const size_t rsz = hostConsole->read(buf, bufSize))
        {
            logBuffer->append(buf, rsz);
        }
    }
    catch (const std::system_error& ex)
    {
        log<level::ERR>(ex.what());
    }
}
