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

namespace host_logger::stream
{

namespace
{

constexpr char socketPath[] = "\0rsyslog";
constexpr char firstDatagram[] = "Hello world";
constexpr char secondDatagram[] = "World hello again";

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Le;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::Test;
using ::testing::Throw;

class ServiceTest : public Test
{
  public:
    ServiceTest() :
        dbusLoopMock(std::make_unique<DbusLoopMock>()),
        hostConsoleMock(std::make_unique<HostConsoleMock>()), serverSocket(-1)
    {}
    ~ServiceTest()
    {
        // Stop server
        if (serverSocket != -1)
        {
            close(serverSocket);
        }
    }

  protected:
    void startServer()
    {
        // Start server
        serverSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        ASSERT_NE(serverSocket, -1);
        sockaddr_un sa{};
        sa.sun_family = AF_UNIX;
        memcpy(sa.sun_path, socketPath, sizeof(socketPath) - 1);
        sa.sun_path[sizeof(socketPath) - 1] = '\0';
        const socklen_t len =
            sizeof(sa) - sizeof(sa.sun_path) + sizeof(socketPath) - 1;
        ASSERT_NE(
            bind(serverSocket, reinterpret_cast<const sockaddr*>(&sa), len),
            -1);
    }

    std::unique_ptr<DbusLoopMock> dbusLoopMock;
    std::unique_ptr<HostConsoleMock> hostConsoleMock;
    int serverSocket;
};

TEST_F(ServiceTest, HostConsoleIsNull)
{
    Service service("whatever", std::move(dbusLoopMock), nullptr);
    EXPECT_THROW(service.run(), std::invalid_argument);
}

TEST_F(ServiceTest, DbusLoopIsNull)
{
    Service service("whatever", nullptr, std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::invalid_argument);
}

TEST_F(ServiceTest, DestinationTooLong)
{
    std::string tooLong(1024, 'a');
    Service service(tooLong, std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::invalid_argument);
}

TEST_F(ServiceTest, HostConsolConnectionError)
{
    EXPECT_CALL(*hostConsoleMock, connect())
        .WillOnce(Throw(std::runtime_error("Mock error")));
    Service service("whatever", std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::runtime_error);
}

TEST_F(ServiceTest, IoRegisterError)
{
    InSequence sequence;
    EXPECT_CALL(*hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, addIoHandler(Eq(int(*hostConsoleMock)), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    Service service("whatever", std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::runtime_error);
}

TEST_F(ServiceTest, SignalRegisterError)
{
    InSequence sequence;
    EXPECT_CALL(*hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, addIoHandler(Eq(int(*hostConsoleMock)), _))
        .WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Throw(std::runtime_error("Mock error")));
    Service service("whatever", std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::runtime_error);
}

TEST_F(ServiceTest, DbusEventLoopError)
{
    InSequence sequence;
    EXPECT_CALL(*hostConsoleMock, connect()).WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, addIoHandler(Eq(int(*hostConsoleMock)), _))
        .WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, addSignalHandler(Eq(SIGTERM), _))
        .WillOnce(Return());
    EXPECT_CALL(*dbusLoopMock, run()).WillOnce(Return(-1));
    Service service("whatever", std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    EXPECT_THROW(service.run(), std::system_error);
}

TEST_F(ServiceTest, ConsoleReadTwice)
{
    startServer();

    char buffer[1024];

    EXPECT_CALL(*hostConsoleMock, read(_, Le(1024)))
        .WillOnce(
            DoAll(SetArrayArgument<0>(firstDatagram,
                                      firstDatagram + strlen(firstDatagram)),
                  Return(strlen(firstDatagram))))
        .WillOnce(Return(0))
        .WillOnce(
            DoAll(SetArrayArgument<0>(secondDatagram,
                                      secondDatagram + strlen(secondDatagram)),
                  Return(strlen(secondDatagram))))
        .WillOnce(Return(0));
    EXPECT_CALL(*hostConsoleMock, connect()).WillOnce(Return());

    std::string destination(socketPath, socketPath + sizeof(socketPath) - 1);

    Service service(destination, std::move(dbusLoopMock),
                    std::move(hostConsoleMock));
    service.connect();

    service.readConsole();
    EXPECT_EQ(read(serverSocket, buffer, 1024), strlen(firstDatagram));
    buffer[strlen(firstDatagram)] = '\0';
    EXPECT_STREQ(buffer, firstDatagram);

    service.readConsole();
    EXPECT_EQ(read(serverSocket, buffer, 1024), strlen(secondDatagram));
    buffer[strlen(secondDatagram)] = '\0';
    EXPECT_STREQ(buffer, secondDatagram);
}

} // namespace

} // namespace host_logger::stream
