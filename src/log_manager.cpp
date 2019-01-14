/**
 * @brief Log manager.
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

#include "log_manager.hpp"

#include "config.hpp"

#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <set>
#include <vector>

// Path to the Unix Domain socket file used to read host's logs
#define HOSTLOG_SOCKET_PATH "\0obmc-console"
// Number of connection attempts
#define HOSTLOG_SOCKET_ATTEMPTS 60
// Pause between connection attempts in seconds
#define HOSTLOG_SOCKET_PAUSE 1
// Max buffer size to read from socket
#define MAX_SOCKET_BUFFER_SIZE 512

LogManager::LogManager() : fd_(-1)
{
}

LogManager::~LogManager()
{
    closeHostLog();
}

int LogManager::openHostLog()
{
    int rc;

    do
    {
        // Create socket
        fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd_ == -1)
        {
            rc = errno;
            fprintf(stderr, "Unable to create socket: error [%i] %s\n", rc,
                    strerror(rc));
            break;
        }

        // Set non-blocking mode for socket
        int opt = 1;
        rc = ioctl(fd_, FIONBIO, &opt);
        if (rc != 0)
        {
            rc = errno;
            fprintf(stderr,
                    "Unable to set non-blocking mode for log socket: error "
                    "[%i] %s\n",
                    rc, strerror(rc));
            break;
        }

        sockaddr_un sa = {0};
        sa.sun_family = AF_UNIX;
        constexpr int min_path =
            sizeof(HOSTLOG_SOCKET_PATH) < sizeof(sa.sun_path)
                ? sizeof(HOSTLOG_SOCKET_PATH)
                : sizeof(sa.sun_path);
        memcpy(&sa.sun_path, HOSTLOG_SOCKET_PATH, min_path);

        // Connect to host's log stream via socket.
        // The owner of the socket (server) is obmc-console service and
        // we have a dependency on it written in the systemd unit file, but
        // we can't guarantee that the socket is initialized at the moment.
        rc = -1;
        for (int attempt = 0; rc != 0 && attempt < HOSTLOG_SOCKET_ATTEMPTS;
             ++attempt)
        {
            rc = connect(fd_, reinterpret_cast<const sockaddr*>(&sa),
                         sizeof(sa) - sizeof(sa.sun_path) + min_path - 1);
            sleep(HOSTLOG_SOCKET_PAUSE);
        }
        if (rc < 0)
        {
            rc = errno;
            fprintf(stderr,
                    "Unable to connect to host log socket: error [%i] %s\n", rc,
                    strerror(rc));
        }
    } while (false);

    if (rc != 0)
        closeHostLog();

    return rc;
}

void LogManager::closeHostLog()
{
    if (fd_ != -1)
    {
        ::close(fd_);
        fd_ = -1;
    }
}

int LogManager::getHostLogFd() const
{
    return fd_;
}

int LogManager::handleHostLog()
{
    int rc = 0;
    std::vector<char> buff(MAX_SOCKET_BUFFER_SIZE);
    size_t readLen = MAX_SOCKET_BUFFER_SIZE;

    // Read all existing data from log stream
    while (rc == 0 && readLen != 0)
    {
        rc = readHostLog(&buff[0], buff.size(), readLen);
        if (rc == 0 && readLen != 0)
            storage_.parse(&buff[0], readLen);
    }

    return rc;
}

int LogManager::flush()
{
    int rc;

    if (storage_.empty())
        return 0; // Nothing to save

    const std::string logFile = prepareLogPath();
    if (logFile.empty())
        return EIO;

    rc = storage_.write(logFile.c_str());
    if (rc != 0)
        return rc;

    storage_.clear();

    // Non critical tasks, don't check returned status
    rotateLogFiles();

    return 0;
}

int LogManager::readHostLog(char* buffer, size_t bufferLen,
                            size_t& readLen) const
{
    int rc = 0;

    const ssize_t rsz = ::read(fd_, buffer, bufferLen);
    if (rsz >= 0)
        readLen = static_cast<size_t>(rsz);
    else
    {
        readLen = 0;
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            rc = errno;
            fprintf(stderr, "Unable to read host log: error [%i] %s\n", rc,
                    strerror(rc));
        }
    }

    return rc;
}

std::string LogManager::prepareLogPath() const
{
    // Create path for logs
    if (::access(loggerConfig.path, F_OK) != 0)
    {
        const std::string logPath(loggerConfig.path);
        const size_t len = logPath.length();
        size_t pos = 0;
        while (pos < len - 1)
        {
            pos = logPath.find('/', pos + 1);
            const std::string createPath = logPath.substr(0, pos);
            if (::mkdir(createPath.c_str(),
                        S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0 &&
                errno != EEXIST)
            {
                const int rc = errno;
                fprintf(stderr, "Unable to create dir %s: error [%i] %s\n",
                        createPath.c_str(), rc, strerror(rc));
                return std::string();
            }
        }
    }

    // Construct log file name
    time_t ts;
    time(&ts);
    tm lt = {0};
    localtime_r(&ts, &lt);
    char fileName[64];
    snprintf(fileName, sizeof(fileName), "/host_%i%02i%02i_%02i%02i%02i.log.gz",
             lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour,
             lt.tm_min, lt.tm_sec);

    return std::string(loggerConfig.path) + fileName;
}

int LogManager::rotateLogFiles() const
{
    if (loggerConfig.rotationLimit == 0)
        return 0; // Not applicable

    int rc = 0;

    // Get file list to std::set
    std::set<std::string> logFiles;
    DIR* dh = opendir(loggerConfig.path);
    if (!dh)
    {
        rc = errno;
        fprintf(stderr, "Unable to open directory %s: error [%i] %s\n",
                loggerConfig.path, rc, strerror(rc));
        return rc;
    }
    dirent* dir;
    while ((dir = readdir(dh)))
    {
        if (dir->d_type != DT_DIR)
            logFiles.insert(dir->d_name);
    }
    closedir(dh);

    // Log file has a name with a timestamp generated by prepareLogPath().
    // The sorted array of names (std::set) will contain the oldest file on the
    // top.
    // Remove oldest files.
    int filesToRemove =
        static_cast<int>(logFiles.size()) - loggerConfig.rotationLimit;
    while (rc == 0 && --filesToRemove >= 0)
    {
        std::string fileToRemove = loggerConfig.path;
        fileToRemove += '/';
        fileToRemove += *logFiles.begin();
        if (::unlink(fileToRemove.c_str()) == -1)
        {
            rc = errno;
            fprintf(stderr, "Unable to delete file %s: error [%i] %s\n",
                    fileToRemove.c_str(), rc, strerror(rc));
        }
        logFiles.erase(fileToRemove);
    }

    return rc;
}
