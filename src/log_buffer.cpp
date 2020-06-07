// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "log_buffer.hpp"

LogBuffer::LogBuffer(size_t maxSize, size_t maxTime) :
    lastComplete(true), sizeLimit(maxSize), timeLimit(maxTime)
{}

void LogBuffer::append(const char* data, size_t sz)
{
    // Split raw data into separate messages by EOL symbol (\n).
    // Stream may not be ended with EOL, so we handle this situation by
    // lastComplete flag.
    size_t pos = 0;
    while (pos < sz)
    {
        // Search for EOL ('\n')
        size_t eol = pos;
        while (eol < sz && data[eol] != '\n')
        {
            ++eol;
        }
        const bool eolFound = eol < sz;
        const char* msgText = data + pos;
        size_t msgLen = (eolFound ? eol : sz) - pos;

        // Remove '\r' from the end of message
        while (msgLen && msgText[msgLen - 1] == '\r')
        {
            --msgLen;
        }

        // Append message to the container
        if (msgLen)
        {
            if (!lastComplete && !messages.empty())
            {
                // The last message is incomplete, add data as part of it
                messages.back().text.append(msgText, msgLen);
            }
            else
            {
                Message msg;
                time(&msg.timeStamp);
                msg.text.assign(msgText, msgLen);
                messages.push_back(msg);
            }
        }

        pos = eol + 1; // Skip '\n'
        lastComplete = eolFound;
    }

    shrink();
}

void LogBuffer::setFullHandler(std::function<void()> cb)
{
    fullHandler = cb;
}

void LogBuffer::clear()
{
    messages.clear();
    lastComplete = true;
}

bool LogBuffer::empty() const
{
    return messages.empty();
}

LogBuffer::container_t::const_iterator LogBuffer::begin() const
{
    return messages.begin();
}

LogBuffer::container_t::const_iterator LogBuffer::end() const
{
    return messages.end();
}

void LogBuffer::shrink()
{
    if (sizeLimit && messages.size() > sizeLimit)
    {
        if (fullHandler)
        {
            fullHandler();
        }
        while (messages.size() > sizeLimit)
        {
            messages.pop_front();
        }
    }
    if (timeLimit)
    {
        time_t oldest;
        time(&oldest);
        oldest -= timeLimit * 60 /* sec */;
        if (!messages.empty() && messages.begin()->timeStamp < oldest)
        {
            if (fullHandler)
            {
                fullHandler();
            }
            while (!messages.empty() && messages.begin()->timeStamp < oldest)
            {
                messages.pop_front();
            }
        }
    }
}
