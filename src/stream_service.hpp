// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#pragma once

#include "config.hpp"
#include "dbus_loop.hpp"
#include "file_storage.hpp"
#include "host_console.hpp"
#include "log_buffer.hpp"
#include "service.hpp"

#include <sys/un.h>

/**
 * @class Service
 * @brief Log service: watches for events and handles them.
 */
class StreamService : public Service
{
  public:
    /**
     * @brief Constructor for stream-only mode. All arguments should outlive
     * this class.
     *
     * @param streamDestination the destination socket to stream logs.
     * @param dbusLoop the DbusLoop instance.
     * @param hostConsole the HostConsole instance.
     */
    StreamService(const char* streamDestination, DbusLoop& dbusLoop,
                  HostConsole& hostConsole);

    /**
     * @brief Destructor; close the file descriptor.
     */
    ~StreamService() override;

    /**
     * @brief Run the service.
     *
     * @throw std::exception in case of errors
     */
    void run() override;

  protected:
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
    /** @brief Path to the destination (the rsyslog unix socket) */
    const char* destinationPath;
    /** @brief D-Bus event loop. */
    DbusLoop* dbusLoop;
    /** @brief Host console connection. */
    HostConsole* hostConsole;
    /** @brief File descriptor of the ouput socket */
    int outputSocketFd;
    /** @brief Address of the destination (the rsyslog unix socket) */
    sockaddr_un destination;
};
