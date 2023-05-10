// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 GOOGLE

#pragma once

#include "host_console.hpp"

#include <gmock/gmock.h>

class HostConsoleMock : public HostConsole
{
  public:
    HostConsoleMock() : HostConsole("") {}
    MOCK_METHOD(void, connect, (), (override));
    MOCK_METHOD(size_t, read, (char* buf, size_t sz), (const, override));
    // Returns a fixed integer for testing.
    virtual operator int() const override
    {
        return 1;
    };
};
