/**
 * @brief zLib exception.
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

#include <exception>
#include <string>

/** @class ZlibException
 *  @brief zLib exception.
 */
class ZlibException : public std::exception
{
  public:
    /** @brief File operation types. */
    enum Operation
    {
        Create,
        Write,
        Close
    };

    /** @brief Constructor.
     *
     *  @param[in] op - type of operation
     *  @param[in] code - zLib status code
     *  @param[in] fd - zLib file descriptor
     *  @param[in] fileName - file name
     */
    ZlibException(Operation op, int code, gzFile fd,
                  const std::string& fileName);

    // From std::exception
    const char* what() const noexcept override;

  private:
    /** @brief Error description buffer. */
    std::string what_;
};
