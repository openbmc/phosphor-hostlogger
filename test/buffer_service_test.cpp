// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#include "buffer_service.hpp"
#include "config.hpp"
#include "dbus_loop_mock.hpp"
#include "file_storage_mock.hpp"
#include "host_console_mock.hpp"
#include "log_buffer_mock.hpp"

#include <memory>
#include <string>
#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

constexpr char firstDatagram[] = "Hello world";
// Shouldn't read more than maximum size of a datagram.
constexpr int consoleReadMaxSize = 1024;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Le;
using ::testing::Ref;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::StrEq;
using ::testing::Test;
using ::testing::Throw;

// A helper class that owns config.
struct ConfigInTest
{
    Config config;
    ConfigInTest() : config()
    {}
};

class BufferServiceTest : public Test, public ConfigInTest, public BufferService
{
  public:
    // ConfigInTest::config is initialized before BufferService.
    BufferServiceTest() :
        BufferService(ConfigInTest::config, dbusLoopMock, hostConsoleMock,
                      logBufferMock, fileStorageMock)
    {}

    MOCK_METHOD(void, flush, (), (override));
    MOCK_METHOD(void, readConsole, (), (override));

  protected:
    // Set hostConsole firstly read specified data and then read nothing.
    void setHostConsoleOnce(char const* data, size_t len)
    {
        EXPECT_CALL(hostConsoleMock, read(_, Le(consoleReadMaxSize)))
            .WillOnce(DoAll(SetArrayArgument<0>(data, data + len), Return(len)))
            .WillOnce(Return(0));
    }

    DbusLoopMock dbusLoopMock;
    HostConsoleMock hostConsoleMock;
    LogBufferMock logBufferMock;
    FileStorageMock fileStorageMock;
};

TEST_F(BufferServiceTest, WrongService)
{
    ConfigInTest::config.mode = Mode::streamMode;
    EXPECT_THROW(BufferService(ConfigInTest::config, dbusLoopMock,
                               hostConsoleMock, logBufferMock, fileStorageMock),
                 std::invalid_argument);
}

TEST_F(BufferServiceTest, FlushEmptyBuffer)
{
    EXPECT_CALL(logBufferMock, empty()).WillOnce(Return(true));
    EXPECT_NO_THROW(BufferService::flush());
}

TEST_F(BufferServiceTest, FlushExceptionCaught)
{
    InSequence sequence;
    EXPECT_CALL(logBufferMock, empty()).WillOnce(Return(false));
    EXPECT_CALL(fileStorageMock, save(Ref(logBufferMock)))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    EXPECT_NO_THROW(BufferService::flush());
}

TEST_F(BufferServiceTest, FlushOk)
{
    InSequence sequence;
    EXPECT_CALL(logBufferMock, empty()).WillOnce(Return(false));
    EXPECT_CALL(fileStorageMock, save(Ref(logBufferMock)));
    EXPECT_CALL(logBufferMock, clear());
    EXPECT_NO_THROW(BufferService::flush());
}

TEST_F(BufferServiceTest, ReadConsoleExceptionCaught)
{
    InSequence sequence;
    // Shouldn't read more than maximum size of a datagram.
    EXPECT_CALL(hostConsoleMock, read(_, Le(1024)))
        .WillOnce(Throw(std::system_error(std::error_code(), "Mock error")));
    EXPECT_NO_THROW(BufferService::readConsole());
}

TEST_F(BufferServiceTest, ReadConsoleOk)
{

    setHostConsoleOnce(firstDatagram, strlen(firstDatagram));
    EXPECT_CALL(logBufferMock,
                append(StrEq(firstDatagram), Eq(strlen(firstDatagram))))
        .WillOnce(Return());
    EXPECT_NO_THROW(BufferService::readConsole());
}

TEST_F(BufferServiceTest, RunIoRegisterError)
{
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGUSR1), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addIoHandler(Eq(int(hostConsoleMock)), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    EXPECT_THROW(run(), std::runtime_error);
}

TEST_F(BufferServiceTest, RunSignalRegisterError)
{
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGUSR1), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    EXPECT_THROW(run(), std::runtime_error);
}

TEST_F(BufferServiceTest, RunOk)
{
    ConfigInTest::config.bufFlushFull = true;
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addIoHandler(Eq(int(hostConsoleMock)), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Return());
    EXPECT_CALL(logBufferMock, setFullHandler(_)).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGUSR1), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock,
                addPropertyHandler(StrEq(ConfigInTest::config.hostState), _, _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, run).WillOnce(Return(0));
    EXPECT_CALL(logBufferMock, empty()).WillOnce(Return(false));
    EXPECT_CALL(*this, flush()).WillOnce(Return());
    EXPECT_NO_THROW(run());
}
} // namespace
