// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#pragma once

#include "config.hpp"
#include "dbus_loop.hpp"
#include "host_console.hpp"

#include <sys/un.h>

#include <memory>
#include <string>

namespace host_logger::stream
{

/**
 * @class Service
 * @brief Stream service: watches for io events and forward logs to the rsyslog
 * unix socket.
 */
class Service
{
  public:
    /**
     * @brief Constructor.
     *
     * @param streamDestination path to the rsyslog unix socket.
     * @param dbusLoop the DbusLoop instance.
     * @param hostConsole the HostConsole instance.
     */
    Service(const std::string& streamDestination,
            std::unique_ptr<DbusLoop> dbusLoop,
            std::unique_ptr<HostConsole> hostConsole);

    /**
     * @brief Destructor. Close the output socket.
     */
    ~Service() = default;

    /**
     * @brief Run the service.
     *
     * @throw std::exception in case of errors
     */
    void run();

    /**
     * @brief Read data from host console and send it to the destination.
     */
    void readConsole();

    /**
     * @brief Connect to the host console and open a socket to stream logs.
     *
     * @throw std::invalid_argument if dbusLoop or hostConsole is nullptr
     * @throw std::system_error in case of errors
     */
    void connect();

  private:
    /** @brief D-Bus event loop. */
    std::unique_ptr<DbusLoop> dbusLoop;
    /** @brief Host console connection. */
    std::unique_ptr<HostConsole> hostConsole;
    /** @brief File descriptor of the ouput socket */
    int outputSocketFd;
    /** @brief Address of the destination (the rsyslog unix socket) */
    sockaddr_un destination;
    std::string destinationStr;
};

} // namespace host_logger::stream
