// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 GOOGLE

#pragma once

#include "log_buffer.hpp"

#include <gmock/gmock.h>

class LogBufferMock : public LogBuffer
{
  public:
    LogBufferMock() : LogBuffer(-1, -1)
    {}
    MOCK_METHOD(void, append, (const char* data, size_t sz), (override));
    MOCK_METHOD(void, setFullHandler, (std::function<void()> cb), (override));
    MOCK_METHOD(bool, empty, (), (const, override));
    MOCK_METHOD(void, clear, (), (override));
};
