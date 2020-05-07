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

#include "log_storage.hpp"

#include "config.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

LogStorage::LogStorage() : last_complete_(true)
{
}

void LogStorage::parse(const char* data, size_t len)
{
    // Split log stream to separate messages.
    // Stream may not be ended with EOL, so we handle this situation by
    // last_complete_ flag.
    size_t pos = 0;
    while (pos < len)
    {
        // Search for EOL ('\n')
        size_t eol = pos;
        while (eol < len && data[eol] != '\n')
            ++eol;
        const bool eolFound = eol < len;
        const char* msgText = data + pos;
        size_t msgLen = (eolFound ? eol : len) - pos;

        // Remove '\r' from the end of message
        while (msgLen && msgText[msgLen - 1] == '\r')
            --msgLen;

        // Append message to store
        if (msgLen)
            append(msgText, msgLen);

        pos = eol + 1; // Skip '\n'
        last_complete_ = eolFound;
    }
}

void LogStorage::append(const char* msg, size_t len)
{
    if (!last_complete_)
    {
        // The last message is incomplete, add msg as part of it
        if (!messages_.empty())
        {
            Message& last_msg = *messages_.rbegin();
            last_msg.text.append(msg, len);
        }
    }
    else
    {
        Message new_msg;
        time(&new_msg.timeStamp);
        new_msg.text.assign(msg, len);
        messages_.push_back(new_msg);
        shrink();
    }
}

void LogStorage::clear()
{
    messages_.clear();
    last_complete_ = true;
}

bool LogStorage::empty() const
{
    return messages_.empty();
}

int LogStorage::write(const char* fileName) const
{
    int rc = 0;

    if (empty())
    {
        printf("No messages to write\n");
        return 0;
    }

    const gzFile fd = gzopen(fileName, "w");
    if (fd == Z_NULL)
    {
        rc = errno;
        fprintf(stderr, "Unable to open file %s: error [%i] %s\n", fileName, rc,
                strerror(rc));
        return rc;
    }

    // Write full datetime stamp as the first record
    const time_t& logStartTime = messages_.begin()->timeStamp;
    tm localTime = {0};
    localtime_r(&logStartTime, &localTime);
    char msgText[64];
    snprintf(msgText, sizeof(msgText),
             ">>> Log collection started at %02i.%02i.%i %02i:%02i:%02i",
             localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year + 1900,
             localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
    const Message startMsg = {logStartTime, msgText};
    rc |= write(fd, startMsg);

    // Write messages
    for (auto it = messages_.begin(); rc == 0 && it != messages_.end(); ++it)
        rc |= write(fd, *it);

    const int rcClose = gzclose_w(fd);
    if (rcClose != Z_OK)
    {
        fprintf(stderr, "Unable to close file %s: error [%i]\n", fileName,
                rcClose);
        if (rc == 0)
            rc = rcClose;
    }

    return rc;
}

int LogStorage::write(gzFile fd, const Message& msg) const
{
    int rc = 0;

    // Convert timestamp to local time
    tm localTime = {0};
    localtime_r(&msg.timeStamp, &localTime);

    // Write message to the file
    do
    {
        rc = gzprintf(fd, "[ %02i:%02i:%02i ]: ", localTime.tm_hour,
                      localTime.tm_min, localTime.tm_sec);
        if (rc <= 0)
            break;

        const size_t len = msg.text.length();
        if (len)
        {
            rc = gzwrite(fd, msg.text.data(), static_cast<unsigned int>(len));
            if (rc <= 0)
                break;
        }

        rc = gzputc(fd, '\n');
    } while (false);

    if (rc <= 0)
    {
        fprintf(stderr, "Unable to write file: error [%i]\n", -rc);
        const char* errDesc = gzerror(fd, &rc);
        if (errDesc)
            fprintf(stderr, "[%i] %s\n", rc, errDesc);
        return EIO;
    }

    return 0;
}

void LogStorage::shrink()
{
    if (loggerConfig.storageSizeLimit)
    {
        while (messages_.size() >
               static_cast<size_t>(loggerConfig.storageSizeLimit))
            messages_.pop_front();
    }
    if (loggerConfig.storageTimeLimit)
    {
        // Get time for N hours ago
        time_t oldestTimeStamp;
        time(&oldestTimeStamp);
        oldestTimeStamp -= loggerConfig.storageTimeLimit * 60 * 60;
        while (!messages_.empty() &&
               messages_.begin()->timeStamp < oldestTimeStamp)
            messages_.pop_front();
    }
}
