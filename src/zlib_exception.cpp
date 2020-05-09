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

#include "zlib_exception.hpp"

#include <cstring>

ZlibException::ZlibException(Operation op, int code, gzFile fd,
                             const std::string& fileName)
{
    std::string details;
    if (code == Z_ERRNO)
    {
        // System error
        const int errCode = errno ? errno : EIO;
        details = strerror(errCode);
    }
    else if (fd != Z_NULL)
    {
        // Try to get description from zLib
        int lastErrCode = 0;
        const char* lastErrDesc = gzerror(fd, &lastErrCode);
        if (lastErrCode)
        {
            details = '[';
            details += std::to_string(lastErrCode);
            details += "] ";
            details += lastErrDesc;
        }
    }
    if (details.empty())
    {
        details = "Internal zlib error (code ";
        details += std::to_string(code);
        details += ')';
    }

    what_ = "Unable to ";
    switch (op)
    {
        case Create:
            what_ += "create";
            break;
        case Close:
            what_ += "close";
            break;
        case Write:
            what_ += "write";
            break;
    }
    what_ += " file ";
    what_ += fileName;
    what_ += ": ";
    what_ += details;
}

const char* ZlibException::what() const noexcept
{
    return what_.c_str();
}
