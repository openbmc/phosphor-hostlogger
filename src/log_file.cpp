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

#include "log_file.hpp"

#include "zlib_exception.hpp"

LogFile::LogFile(const char* fileName)
{
    fd_ = gzopen(fileName, "w");
    if (fd_ == Z_NULL)
        throw ZlibException(ZlibException::Create, Z_ERRNO, fd_, fileName);
    fileName_ = fileName;
}

LogFile::~LogFile()
{
    if (fd_ != Z_NULL)
        gzclose_w(fd_);
}

void LogFile::close()
{
    if (fd_ != Z_NULL)
    {
        const int rc = gzclose_w(fd_);
        if (rc != Z_OK)
            throw ZlibException(ZlibException::Close, rc, fd_, fileName_);
        fd_ = Z_NULL;
        fileName_.clear();
    }
}

void LogFile::write(time_t timeStamp, const std::string& message) const
{
    int rc;

    // Convert time stamp and write it
    tm tmLocal;
    localtime_r(&timeStamp, &tmLocal);
    rc = gzprintf(fd_, "[ %02i:%02i:%02i ]: ", tmLocal.tm_hour, tmLocal.tm_min,
                  tmLocal.tm_sec);
    if (rc <= 0)
        throw ZlibException(ZlibException::Write, rc, fd_, fileName_);

    // Write message
    const size_t len = message.length();
    if (len)
    {
        rc = gzwrite(fd_, message.data(), static_cast<unsigned int>(len));
        if (rc <= 0)
            throw ZlibException(ZlibException::Write, rc, fd_, fileName_);
    }

    // Write EOL
    rc = gzputc(fd_, '\n');
    if (rc <= 0)
        throw ZlibException(ZlibException::Write, rc, fd_, fileName_);
}
