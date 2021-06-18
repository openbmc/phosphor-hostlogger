// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include "config.hpp"
#include "dbus_loop.hpp"
#include "file_storage.hpp"
#include "host_console.hpp"
#include "log_buffer.hpp"

#include <sys/un.h>

/**
 * @class Service
 * @brief Log service: watches for events and handles them.
 */
class Service
{
  public:
    /**
     * @brief Constructor for stream-only mode. All arguments should outlive
     * this class.
     *
     * @param config service configuration.
     * @param dbusLoop the DbusLoop instance.
     * @param hostConsole the HostConsole instance.
     *
     * @throw std::exception in case of errors
     */
    Service(const Config& config, DbusLoop& dbusLoop, HostConsole& hostConsole);

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
    Service(const Config& config, DbusLoop& dbusLoop, HostConsole& hostConsole,
            LogBuffer& logBuffer, FileStorage& fileStorage);

    virtual ~Service() = default;

    /**
     * @brief Run the service.
     *
     * @throw std::exception in case of errors
     */
    void run();

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

    /**
     * @brief Stream console data to a datagram unix socket.
     *
     * @param data the bytes to stream
     * @param len the length of the bytes array
     *
     * @throw std::exception in case of errors
     */
    virtual void streamConsole(const char* data, size_t len);

    /**
     * @brief Set up stream socket
     *
     * @throw std::exception in case of errors
     */
    virtual void setStreamSocket();

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
    /** @brief File descriptor of the ouput socket */
    int outputSocketFd;
    /** @brief Address of the destination (the rsyslog unix socket) */
    sockaddr_un destination;
};
