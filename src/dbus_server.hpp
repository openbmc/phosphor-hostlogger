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

#pragma once

#include "log_manager.hpp"
#include <xyz/openbmc_project/HostLogger/server.hpp>

/** @brief D-Bus interface name. */
#define HOSTLOGGER_DBUS_IFACE "xyz.openbmc_project.HostLogger"
/** @brief D-Bus path. */
#define HOSTLOGGER_DBUS_PATH  "/xyz/openbmc_project/HostLogger"

/** @brief unique_ptr functor to release an event reference. */
struct EventDeleter
{
    void operator()(sd_event* event) const
    {
        sd_event_unref(event);
    }
};

/* @brief Alias 'event' to a unique_ptr type for auto-release. */
using EventPtr = std::unique_ptr<sd_event, EventDeleter>;

// Typedef for super class
using server_inherit = sdbusplus::server::object_t<sdbusplus::xyz::openbmc_project::server::HostLogger>;


/** @class DbusServer
 *  @brief D-Bus service by host logger.
 */
class DbusServer: public server_inherit
{
public:
    /** @brief Constructor.
     *
     *  @param[in] logManager - log manager
     *  @param[in] bus - bus to attach to
     *  @param[in] path - bath to attach at, optional, default is HOSTLOGGER_DBUS_PATH
     */
    DbusServer(LogManager& logManager, sdbusplus::bus::bus& bus, const char* path = HOSTLOGGER_DBUS_PATH);

    // From server_inherit
    void flush();

private:
    /** @brief Log manager instance. */
    LogManager& logManager_;
};
