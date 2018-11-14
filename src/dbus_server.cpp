/**
 * @brief Server side implementation of the D-Bus interface
 *
 * This file is part of HostLogger project.
 *
 * Copyright (c) 2018 YADRO
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

#include "dbus_server.hpp"

#include <xyz/openbmc_project/Common/File/error.hpp>

DbusServer::DbusServer(LogManager& logManager, sdbusplus::bus::bus& bus,
                       const char* path /*= HOSTLOGGER_DBUS_PATH*/) :
    server_inherit(bus, path),
    logManager_(logManager)
{
}

void DbusServer::flush()
{
    const int rc = logManager_.flush();
    if (rc != 0)
        throw sdbusplus::xyz::openbmc_project::Common::File::Error::Write();
}
