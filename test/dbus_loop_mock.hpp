// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 GOOGLE

#pragma once

#include "dbus_loop.hpp"

#include <gmock/gmock.h>

class DbusLoopMock : public DbusLoop
{
  public:
    MOCK_METHOD(int, run, (), (const, override));
    MOCK_METHOD(void, addIoHandler, (int fd, std::function<void()> callback),
                (override));
    MOCK_METHOD(void, addSignalHandler,
                (int signal, std::function<void()> callback), (override));
    MOCK_METHOD(void, addPropertyHandler,
                (const std::string& objPath, const WatchProperties& props,
                 std::function<void()> callback),
                (override));
};
