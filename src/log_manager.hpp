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

#pragma once

#include "log_storage.hpp"


/** @class LogManager
 *  @brief Log manager.
 *         All functions within this class are not thread-safe.
 */
class LogManager
{
public:
    /** @brief Constructor. */
    LogManager();

    /** @brief Destructor. */
    ~LogManager();

    /** @brief Open the host's log stream.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int openHostLog();

    /** @brief Close the host's log stream.
    */
    void closeHostLog();

    /** @brief Get file descriptor by host's log stream.
     *         Descriptor can be used to register it in an external polling manager.
     *
     *  @return file descriptor (actually it is an opened socket)
     */
    int getHostLogFd() const;

    /** @brief Handle incoming data from host's log stream.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int handleHostLog();

    /** @brief Flush log storage: save currently collected messages to a file,
     *         reset the storage and rotate log files.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int flush();

private:
    /** @brief Read incoming data from host's log stream.
     *
     *  @param[out] buffer - buffer to write incoming data
     *  @param[in] bufferLen - maximum size of the buffer in bytes
     *  @param[out] readLen - on output contain number of bytes read from stream
     *
     *  @return error code, 0 if operation completed successfully
     */
    int readHostLog(char* buffer, size_t bufferLen, size_t& readLen) const;

    /** @brief Prepare the path to save the log.
     *         Warning: the name is used in function rotateLogFiles(),
     *                  make sure you don't brake sorting rules.
     *
     * @return path to new log file including its name
     */
    std::string prepareLogPath() const;

    /** @brief Create path for log files.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int createLogPath() const;

    /** @brief Rotate log files in the directory.
     *         Function remove oldest files and keep up to maxFiles_ log files.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int rotateLogFiles() const;

private:
    /** @brief Log storage. */
    LogStorage storage_;
    /** @brief File descriptor of the input log stream. */
    int fd_;
};
