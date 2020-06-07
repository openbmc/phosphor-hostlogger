// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#include "log_buffer.hpp"

#include <gtest/gtest.h>

TEST(LogBufferTest, AppendSimple)
{
    const std::string msg = "Test message";

    LogBuffer buf(0, 0);
    buf.append((msg + "\n").data(), msg.length() + 1);
    ASSERT_EQ(std::distance(buf.begin(), buf.end()), 1);
    EXPECT_EQ(buf.begin()->text, msg);
    EXPECT_NE(buf.begin()->timeStamp, 0);
}

TEST(LogBufferTest, AppendPartial)
{
    const std::string msg[] = {"Begin", "End"};

    LogBuffer buf(0, 0);
    buf.append(msg[0].data(), msg[0].length());
    ASSERT_EQ(std::distance(buf.begin(), buf.end()), 1);
    buf.append(msg[1].data(), msg[1].length());
    ASSERT_EQ(std::distance(buf.begin(), buf.end()), 1);
    buf.append("\n", 1);
    ASSERT_EQ(std::distance(buf.begin(), buf.end()), 1);
    EXPECT_EQ(buf.begin()->text, msg[0] + msg[1]);
    EXPECT_NE(buf.begin()->timeStamp, 0);
    buf.append("x\n", 2);
    EXPECT_EQ(std::distance(buf.begin(), buf.end()), 2);
}

TEST(LogBufferTest, Clear)
{
    const std::string msg = "Test message\n";

    LogBuffer buf(0, 0);
    buf.append(msg.data(), msg.length());
    EXPECT_FALSE(buf.empty());
    buf.clear();
    EXPECT_TRUE(buf.empty());
}

TEST(LogBufferTest, SizeLimit)
{
    const size_t limit = 5;
    const std::string msg = "Test message\n";

    LogBuffer buf(limit, 0);
    for (size_t i = 0; i < limit + 3; ++i)
    {
        buf.append(msg.data(), msg.length());
    }
    EXPECT_EQ(std::distance(buf.begin(), buf.end()), limit);
}

TEST(LogBufferTest, FullHandler)
{
    const size_t limit = 5;
    const std::string msg = "Test message\n";

    size_t count = 0;

    LogBuffer buf(limit, 0);
    buf.setFullHandler([&count, &buf]() {
        ++count;
        buf.clear();
    });
    for (size_t i = 0; i < limit + 3; ++i)
    {
        buf.append(msg.data(), msg.length());
    }
    EXPECT_EQ(count, 1);
    EXPECT_EQ(std::distance(buf.begin(), buf.end()), 2);
}
