// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 Google

#include "config.hpp"
#include "dbus_loop_mock.hpp"
#include "host_console_mock.hpp"
#include "stream_service.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

constexpr char socketPath[] = "\0rsyslog";
constexpr char firstDatagram[] = "Hello world";
constexpr char secondDatagram[] = "World hello again";
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

class StreamServiceTest : public Test, public StreamService
{
  public:
    StreamServiceTest() :
        StreamService(socketPath, dbusLoopMock, hostConsoleMock),
        serverSocket(-1)
    {}
    ~StreamServiceTest() override
    {
        // Stop server
        if (serverSocket != -1)
        {
            close(serverSocket);
        }
    }

    MOCK_METHOD(void, readConsole, (), (override));
    MOCK_METHOD(void, streamConsole, (const char* data, size_t len),
                (override));
    MOCK_METHOD(void, setStreamSocket, (), (override));

  protected:
    // Start a server for reading datagrams.
    void startServer()
    {
        serverSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        ASSERT_NE(serverSocket, -1);
        sockaddr_un sa{};
        sa.sun_family = AF_UNIX;
        memcpy(sa.sun_path, socketPath, sizeof(socketPath) - 1);
        sa.sun_path[sizeof(socketPath) - 1] = '\0';
        const socklen_t len =
            sizeof(sa) - sizeof(sa.sun_path) + sizeof(socketPath) - 1;
        ASSERT_NE(bind(serverSocket, reinterpret_cast<const sockaddr*>(&sa),
                       len),
                  -1);
    }

    // Set hostConsole firstly read specified data and then read nothing.
    void setHostConsoleOnce(const char* data, size_t len)
    {
        EXPECT_CALL(hostConsoleMock, read(_, Le(consoleReadMaxSize)))
            .WillOnce(DoAll(SetArrayArgument<0>(data, data + len), Return(len)))
            .WillOnce(Return(0));
    }

    DbusLoopMock dbusLoopMock;
    HostConsoleMock hostConsoleMock;
    int serverSocket;
};

TEST_F(StreamServiceTest, ReadConsoleExceptionCaught)
{
    InSequence sequence;
    // Shouldn't read more than maximum size of a datagram.
    EXPECT_CALL(hostConsoleMock, read(_, Le(1024)))
        .WillOnce(Throw(std::system_error(std::error_code(), "Mock error")));
    EXPECT_NO_THROW(StreamService::readConsole());
}

TEST_F(StreamServiceTest, ReadConsoleOk)
{
    // stream mode
    setHostConsoleOnce(firstDatagram, strlen(firstDatagram));
    EXPECT_CALL(*this,
                streamConsole(StrEq(firstDatagram), Eq(strlen(firstDatagram))))
        .WillOnce(Return());
    EXPECT_NO_THROW(StreamService::readConsole());
}

TEST_F(StreamServiceTest, StreamConsoleOk)
{
    startServer();
    EXPECT_NO_THROW(StreamService::setStreamSocket());
    EXPECT_NO_THROW(
        StreamService::streamConsole(firstDatagram, strlen(firstDatagram)));
    EXPECT_NO_THROW(
        StreamService::streamConsole(secondDatagram, strlen(secondDatagram)));
    char buffer[consoleReadMaxSize];
    EXPECT_EQ(read(serverSocket, buffer, consoleReadMaxSize),
              strlen(firstDatagram));
    buffer[strlen(firstDatagram)] = '\0';
    EXPECT_STREQ(buffer, firstDatagram);
    EXPECT_EQ(read(serverSocket, buffer, consoleReadMaxSize),
              strlen(secondDatagram));
    buffer[strlen(secondDatagram)] = '\0';
    EXPECT_STREQ(buffer, secondDatagram);
}

TEST_F(StreamServiceTest, RunIoRegisterError)
{
    EXPECT_CALL(*this, setStreamSocket()).WillOnce(Return());
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addIoHandler(Eq(int(hostConsoleMock)), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    EXPECT_THROW(run(), std::runtime_error);
}

TEST_F(StreamServiceTest, RunSignalRegisterError)
{
    EXPECT_CALL(*this, setStreamSocket()).WillOnce(Return());
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    EXPECT_THROW(run(), std::runtime_error);
}

TEST_F(StreamServiceTest, RunOk)
{
    EXPECT_CALL(hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addIoHandler(Eq(int(hostConsoleMock)), _))
        .WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Return());
    EXPECT_CALL(*this, setStreamSocket()).WillOnce(Return());
    EXPECT_CALL(dbusLoopMock, run).WillOnce(Return(0));
    EXPECT_NO_THROW(run());
}
} // namespace
