/**
 * @brief Log file.
 *
 * This file is part of HostLogger project.
 *
 * Copyright (c) 2020 YADRO
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

#include <zlib.h>

#include <ctime>
#include <string>

/** @class LogFile
 *  @brief Log file writer.
 */
class LogFile
{
  public:
    /** @brief Constructor - open new file for writing logs.
     *
     *  @param[in] fileName - path to the file
     *
     *  @throw ZlibException in case of errors
     */
    LogFile(const char* fileName);

    ~LogFile();

    LogFile(const LogFile&) = delete;
    LogFile& operator=(const LogFile&) = delete;

    /** @brief Close file.
     *
     *  @throw ZlibException in case of errors
     */
    void close();

    /** @brief Write log message to file.
     *
     *  @param[in] timeStamp - time stamp of the log message
     *  @param[in] message - log message text
     *
     *  @throw ZlibException in case of errors
     */
    void write(time_t timeStamp, const std::string& message) const;

  private:
    /** @brief File name. */
    std::string fileName_;
    /** @brief zLib file descriptor. */
    gzFile fd_;
};
