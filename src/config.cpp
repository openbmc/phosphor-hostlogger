// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "config.hpp"

#include <sys/un.h>

#include <algorithm>
#include <climits>
#include <cstring>
#include <stdexcept>
#include <string>

namespace
{
constexpr char bufferModeStr[] = "buffer";
constexpr char streamModeStr[] = "stream";
} // namespace

/**
 * @brief Set boolean value from environment variable.
 *
 * @param[in] name name of environment variable
 * @param[out] value value to set
 *
 * @throw std::invalid_argument in case of errors
 */
static void safeSet(const char* name, bool& value)
{
    const char* envVal = std::getenv(name);
    if (envVal)
    {
        if (strcmp(envVal, "true") == 0)
        {
            value = true;
        }
        else if (strcmp(envVal, "false") == 0)
        {
            value = false;
        }
        else
        {
            std::string err = "Invalid value of environment variable ";
            err += name;
            err += ": '";
            err += envVal;
            err += "', expected 'true' or 'false'";
            throw std::invalid_argument(err);
        }
    }
}

/**
 * @brief Set unsigned numeric value from environment variable.
 *
 * @param[in] name name of environment variable
 * @param[out] value value to set
 *
 * @throw std::invalid_argument in case of errors
 */
static void safeSet(const char* name, size_t& value)
{
    const char* envVal = std::getenv(name);
    if (envVal)
    {
        const size_t num = strtoul(envVal, nullptr, 0);
        if (std::all_of(envVal, envVal + strlen(envVal), isdigit) &&
            num != ULONG_MAX)
        {
            value = num;
        }
        else
        {
            std::string err = "Invalid argument: ";
            err += envVal;
            err += ", expected unsigned numeric value";
            throw std::invalid_argument(err);
        }
    }
}

/**
 * @brief Set string value from environment variable.
 *
 * @param[in] name name of environment variable
 * @param[out] value value to set
 */
static void safeSet(const char* name, const char*& value)
{
    const char* envVal = std::getenv(name);
    if (envVal)
    {
        value = envVal;
    }
}

Config::Config()
{
    safeSet("SOCKET_ID", socketId);
    const char* mode_str = bufferModeStr;
    safeSet("MODE", mode_str);
    if (strcmp(mode_str, bufferModeStr) == 0)
    {
        mode = Mode::bufferMode;
    }
    else if (strcmp(mode_str, streamModeStr) == 0)
    {
        mode = Mode::streamMode;
    }
    else
    {
        throw std::invalid_argument(
            "Invalid value for mode; expect either 'stream' or 'buffer'");
    }

    if (mode == Mode::bufferMode)
    {
        safeSet("BUF_MAXSIZE", bufMaxSize);
        safeSet("BUF_MAXTIME", bufMaxTime);
        safeSet("FLUSH_FULL", bufFlushFull);
        safeSet("HOST_STATE", hostState);
        safeSet("OUT_DIR", outDir);
        safeSet("MAX_FILES", maxFiles);
        // Validate parameters
        if (bufFlushFull && !bufMaxSize && !bufMaxTime)
        {
            throw std::invalid_argument("Flush policy is set to save the "
                                        "buffer as it fills, but buffer's "
                                        "limits are not defined");
        }
    }
    else
    {
        // mode == Mode::streamMode
        safeSet("STREAM_DST", streamDestination);
        // We need an extra +1 for null terminator.
        if (strlen(streamDestination) + 1 > sizeof(sockaddr_un::sun_path))
        {
            throw std::invalid_argument("Invalid STREAM_DST: too long");
        }
    }
}
