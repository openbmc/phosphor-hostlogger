/**
 * @brief Log storage.
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

#include <zlib.h>

#include <ctime>
#include <list>
#include <string>

/** @class LogStorage
 *  @brief Log storage implementation.
 *         All functions within this class are not thread-safe.
 */
class LogStorage
{
  public:
    /** @brief Constructor. */
    LogStorage();

    /** @brief Parse input log stream and append messages to the storage.
     *
     *  @param[in] data - pointer to the message buffer
     *  @param[in] len - length of the buffer in bytes
     */
    void parse(const char* data, size_t len);

    /** @brief Clear (reset) storage. */
    void clear();

    /** @brief Check storage for empty.
     *
     *  @return true if storage is empty
     */
    bool empty() const;

    /** @brief Save messages from storage to the specified file.
     *
     *  @param[in] fileName - path to the file
     *
     *  @return error code, 0 if operation completed successfully
     */
    int save(const char* fileName) const;

  private:
    /** @struct Message
     *  @brief Represent log message (single line from host log).
     */
    struct Message
    {
        /** @brief Timestamp (message creation time). */
        time_t timeStamp;
        /** @brief Text of the message. */
        std::string text;
    };

    /** @brief Append new message to the storage.
     *
     *  @param[in] msg - pointer to the message buffer
     *  @param[in] len - length of the buffer in bytes
     */
    void append(const char* msg, size_t len);

    /** @brief Write message to the file.
     *
     *  @param[in] fd - descriptor of the file to write
     *  @param[in] msg - message to write
     *
     *  @return error code, 0 if operation completed successfully
     */
    int msgwrite(gzFile fd, const Message& msg) const;

    /** @brief Shrink storage by removing oldest messages. */
    void shrink();

  private:
    /** @brief List of messages. */
    std::list<Message> messages_;
    /** @brief Flag to indicate that the last message is incomplete. */
    bool last_complete_;
};
