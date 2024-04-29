// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#include "stream_service.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <phosphor-logging/log.hpp>

#include <vector>

using namespace phosphor::logging;

StreamService::StreamService(const char* streamDestination, DbusLoop& dbusLoop,
                             HostConsole& hostConsole) :
    destinationPath(streamDestination), dbusLoop(&dbusLoop),
    hostConsole(&hostConsole), outputSocketFd(-1), destination()
{}

StreamService::~StreamService()
{
    if (outputSocketFd != -1)
    {
        close(outputSocketFd);
    }
}

void StreamService::run()
{
    setStreamSocket();
    hostConsole->connect();
    // Add SIGTERM signal handler for service shutdown
    dbusLoop->addSignalHandler(SIGTERM, [this]() { this->dbusLoop->stop(0); });
    // Register callback for socket IO
    dbusLoop->addIoHandler(*hostConsole, [this]() { this->readConsole(); });

    // Run D-Bus event loop
    const int rc = dbusLoop->run();
    if (rc < 0)
    {
        std::error_code ec(-rc, std::generic_category());
        throw std::system_error(ec, "Error in event loop");
    }
}

void StreamService::readConsole()
{
    constexpr size_t bufSize = 128; // enough for most line-oriented output
    std::vector<char> bufData(bufSize);
    char* buf = bufData.data();

    try
    {
        while (const size_t rsz = hostConsole->read(buf, bufSize))
        {
            streamConsole(buf, rsz);
        }
    }
    catch (const std::system_error& ex)
    {
        log<level::ERR>(ex.what());
    }
}

void StreamService::streamConsole(const char* data, size_t len)
{
    // Send all received characters in a blocking manner.
    size_t sent = 0;
    while (sent < len)
    {
        // Datagram sockets preserve message boundaries. Furthermore,
        // In most implementation, UNIX domain datagram sockets are
        // always reliable and don't reorder datagrams.
        ssize_t curr_sent =
            sendto(outputSocketFd, data + sent, len - sent, 0,
                   reinterpret_cast<const sockaddr*>(&destination),
                   sizeof(destination) - sizeof(destination.sun_path) +
                       strlen(destinationPath + 1) + 1);
        if (curr_sent == -1)
        {
            std::string error = "Unable to send to the destination ";
            error += destinationPath;
            std::error_code ec(errno ? errno : EIO, std::generic_category());
            throw std::system_error(ec, error);
        }
        sent += curr_sent;
    }
}

void StreamService::setStreamSocket()
{
    destination.sun_family = AF_UNIX;
    // To deal with abstract namespace unix socket.
    size_t len = strlen(destinationPath + 1) + 1;
    memcpy(destination.sun_path, destinationPath, len);
    destination.sun_path[len] = '\0';
    outputSocketFd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (outputSocketFd == -1)
    {
        std::error_code ec(errno ? errno : EIO, std::generic_category());
        throw std::system_error(ec, "Unable to create output socket.");
    }
}
