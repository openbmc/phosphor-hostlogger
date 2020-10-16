// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "zlib_exception.hpp"
#include "zlib_file.hpp"

#include <gtest/gtest.h>

TEST(ZlibFileTest, Exception)
{
    ASSERT_THROW(ZlibFile("invalid/path"), ZlibException);
}

TEST(ZlibFileTest, Write)
{
    const std::string msg = "Test message";
    time_t currTime;
    time(&currTime);
    tm localTime;
    localtime_r(&currTime, &localTime);

    const std::string path = "/tmp/zlib_file_test.out";
    ZlibFile file(path);
    file.write(localTime, msg);
    file.close();

    char expect[64];
    const int len =
        snprintf(expect, sizeof(expect),
                 "[ %i-%02i-%02iT%02i:%02i:%02i%+03d:%02d ] %s\n",
                 timeStamp.tm_year + 1900, timeStamp.tm_mon + 1,
                 timeStamp.tm_mday, timeStamp.tm_hour, timeStamp.tm_min,
                 timeStamp.tm_sec, timeStamp.tm_gmtoff / (60 * 60),
                 abs(timeStamp.tm_gmtoff % (60 * 60)) / 60, msg.c_str());

    gzFile fd = gzopen(path.c_str(), "r");
    ASSERT_TRUE(fd);
    char buf[64];
    memset(buf, 0, sizeof(buf));
    EXPECT_EQ(gzread(fd, buf, sizeof(buf)), len);
    EXPECT_STREQ(buf, expect);
    EXPECT_EQ(gzclose(fd), 0);

    unlink(path.c_str());
}
