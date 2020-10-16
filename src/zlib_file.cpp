// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "zlib_file.hpp"

#include "zlib_exception.hpp"

ZlibFile::ZlibFile(const std::string& fileName)
{
    fd = gzopen(fileName.c_str(), "w");
    if (fd == Z_NULL)
    {
        throw ZlibException(ZlibException::create, Z_ERRNO, fd, fileName);
    }
    this->fileName = fileName;
}

ZlibFile::~ZlibFile()
{
    if (fd != Z_NULL)
    {
        gzclose_w(fd);
    }
}

void ZlibFile::close()
{
    if (fd != Z_NULL)
    {
        const int rc = gzclose_w(fd);
        if (rc != Z_OK)
        {
            throw ZlibException(ZlibException::close, rc, fd, fileName);
        }
        fd = Z_NULL;
        fileName.clear();
    }
}

void ZlibFile::write(const tm& timeStamp, const std::string& message) const
{
    int rc;

    // Write time stamp.
    // "tm_gmtoff" is the number of seconds east of UTC, so we need to calculate
    // timezone offset. For example, for U.S. Eastern Standard Time, the value
    // is -18000 = -5*60*60."

    rc = gzprintf(fd, "[ %i-%02i-%02iT%02i:%02i:%02i%+03d:%02d ]",
                  timeStamp.tm_year + 1900, timeStamp.tm_mon + 1,
                  timeStamp.tm_mday, timeStamp.tm_hour, timeStamp.tm_min,
                  timeStamp.tm_sec, timeStamp.tm_gmtoff / (60 * 60),
                  abs(timeStamp.tm_gmtoff % (60 * 60)) / 60);

    if (rc <= 0)
    {
        throw ZlibException(ZlibException::write, rc, fd, fileName);
    }

    // Write message
    const size_t len = message.length();
    if (len)
    {
        rc = gzwrite(fd, message.data(), static_cast<unsigned int>(len));
        if (rc <= 0)
        {
            throw ZlibException(ZlibException::write, rc, fd, fileName);
        }
    }

    // Write EOL
    rc = gzputc(fd, '\n');
    if (rc <= 0)
    {
        throw ZlibException(ZlibException::write, rc, fd, fileName);
    }
}
