// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include "config.hpp"
#include "dbus_loop.hpp"
#include "file_storage.hpp"
#include "host_console.hpp"
#include "log_buffer.hpp"
#include "service.hpp"

#include <sys/un.h>

/**
 * @class BufferService
 * @brief Buffer based log service: watches for events and handles them.
 */
class BufferService : public Service
{
  public:
    /**
     * @brief Constructor for buffer-only mode or buffer + stream mode.  All
     * arguments should outlive this class.
     *
     * @param config service configuration.
     * @param dbusLoop the DbusLoop instance.
     * @param hostConsole the HostConsole instance.
     * @param logBuffer the logBuffer instance.
     * @param fileStorage the fileStorage instance.
     *
     * @throw std::exception in case of errors
     */
    BufferService(const Config& config, DbusLoop& dbusLoop,
                  HostConsole& hostConsole, LogBuffer& logBuffer,
                  FileStorage& fileStorage);

    ~BufferService() override = default;

    /**
     * @brief Run the service.
     *
     * @throw std::exception in case of errors
     */
    void run() override;

  protected:
    /**
     * @brief Flush log buffer to a file.
     */
    virtual void flush();

    /**
     * @brief Read data from host console and perform actions according to
     * modes.
     */
    virtual void readConsole();

  private:
    /** @brief Service configuration. */
    const Config& config;
    /** @brief D-Bus event loop. */
    DbusLoop* dbusLoop;
    /** @brief Host console connection. */
    HostConsole* hostConsole;
    /** @brief Intermediate storage: container for parsed log messages. */
    LogBuffer* logBuffer;
    /** @brief Persistent storage. */
    FileStorage* fileStorage;
};
