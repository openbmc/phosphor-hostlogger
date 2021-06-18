#include "stream_service.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <phosphor-logging/log.hpp>

#include <cstring>
#include <system_error>

namespace host_logger::stream
{

using phosphor::logging::entry;
using phosphor::logging::level;
using phosphor::logging::log;

// Enough for most line-oriented output.
// Note that the syslog has a 1024 bytes message size limit, which means this
// constant shall not be larger than 1024.
constexpr int readBufferSize = 128;

Service::Service(const std::string& streamDestination,
                 std::unique_ptr<DbusLoop> dbusLoop,
                 std::unique_ptr<HostConsole> hostConsole) :
    dbusLoop(std::move(dbusLoop)),
    hostConsole(std::move(hostConsole)), outputSocketFd(-1),
    destinationStr(streamDestination)
{}

Service::~Service()
{
    if (outputSocketFd != -1)
    {
        close(outputSocketFd);
    }
}

void Service::connect()
{
    if (hostConsole == nullptr)
    {
        throw std::invalid_argument("Uninitialized hostConsole");
    }
    if (dbusLoop == nullptr)
    {
        throw std::invalid_argument("Uninitialized dbusLoop");
    }
    if (destinationStr.size() + 1 > sizeof(destination.sun_path))
    {
        throw std::invalid_argument(
            "Uninitialized streamDestination: too long");
    }
    destination.sun_family = AF_UNIX;
    memcpy(destination.sun_path, destinationStr.c_str(), destinationStr.size());
    destination.sun_path[destinationStr.size()] = '\0';

    hostConsole->connect();

    if (outputSocketFd != -1)
    {
        throw std::runtime_error("Output socket already opened");
    }

    // Create socket
    outputSocketFd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (outputSocketFd == -1)
    {
        std::error_code ec(errno ? errno : EIO, std::generic_category());
        throw std::system_error(ec, "Unable to create output socket");
    }
}

void Service::readConsole()
{
    char buffer[readBufferSize];
    try
    {
        while (const size_t received =
                   hostConsole->read(buffer, readBufferSize))
        {
            // Send all received characters in a blocking manner.
            size_t sent = 0;
            while (sent < received)
            {
                // Datagram sockets preserve message boundaries. Furthermore,
                // In most implementation, UNIX domain datagram sockets are
                // always reliable and don't reorder datagrams.
                ssize_t curr_sent =
                    sendto(outputSocketFd, buffer + sent, received - sent, 0,
                           reinterpret_cast<const sockaddr*>(&destination),
                           sizeof(destination) - sizeof(destination.sun_path) +
                               destinationStr.size());
                if (curr_sent == -1)
                {
                    std::error_code ec(errno ? errno : EIO,
                                       std::generic_category());
                    throw std::system_error(
                        ec, "Unable to send to the destination");
                }
                sent += curr_sent;
            }
        }
    }
    catch (const std::system_error& ex)
    {
        log<level::ERR>(ex.what());
    }
}

void Service::run()
{
    connect();

    // Register callback for socket IO
    dbusLoop->addIoHandler(*hostConsole, [this]() { this->readConsole(); });
    // Add SIGTERM signal handler for service shutdown
    dbusLoop->addSignalHandler(SIGTERM, [this]() { this->dbusLoop->stop(0); });

    // Run D-Bus event loop
    const int rc = dbusLoop->run();
    if (rc < 0)
    {
        std::error_code ec(-rc, std::generic_category());
        throw std::system_error(ec, "Error in event loop");
    }
}

} // namespace host_logger::stream
