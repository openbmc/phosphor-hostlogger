// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

#include <cstddef>

enum class Mode
{
    bufferMode,
    streamMode
};

/**
 * @struct Config
 * @brief Configuration of the service, initialized with default values.
 */
struct Config
{
    /**
     * @brief Constructor: load configuration from environment variables.
     *
     * @throw std::invalid_argument invalid format in one of the variables
     */
    Config();

    /** The following configs are for both modes. */
    /** @brief Socket ID used for connection with host console. */
    const char* socketId = "";
    /** @brief The mode the service is in. */
    Mode mode = Mode::bufferMode;

    /** The following configs are for buffer mode. */
    /** @brief Max number of messages stored inside intermediate buffer. */
    size_t bufMaxSize = 3000;
    /** @brief Max age of messages (in minutes) inside intermediate buffer. */
    size_t bufMaxTime = 0;
    /** @brief Flag indicated we need to flush console buffer as it fills. */
    bool bufFlushFull = false;
    /** @brief Path to D-Bus object that provides host's state information,
     * optional */
    const char* hostState = "";
    /** @brief Absolute path to the output directory for log files. */
    const char* outDir = "/var/lib/obmc/hostlogs";
    /** @brief Max number of log files in the output directory. */
    size_t maxFiles = 10;

    /** The following configs are for stream mode. */
    /** @brief Path to the unix socket that receives the log stream. */
    const char* streamDestination = "/run/rsyslog/console_input";
};
