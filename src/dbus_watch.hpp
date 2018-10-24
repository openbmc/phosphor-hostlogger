/**
 * @brief D-Bus signal watcher.
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

#include <set>
#include <map>
#include <string>

#include <sdbusplus/bus/match.hpp>



/** @class DbusServer
 *  @brief D-Bus service by host logger.
 */
class DbusWatcher
{
public:
    /** @brief Constructor.
     *
     *  @param[in] logManager - log manager
     *  @param[in] bus - bus to attach to
     */
    DbusWatcher(LogManager& logManager, sdbusplus::bus::bus& bus);

    /** @brief Initialize watcher.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int initialize();

private:
    /** @brief Register D-Bus event handler. */
    void registerEventHandler();

    /** @brief Register D-Bus timer handler.
     *
     *  @return error code, 0 if operation completed successfully
     */
    int registerTimerHandler();

    /** @brief Setup D-Bus timer.
     *
     *  @param[in] event - event source to setup
     *
     *  @return error code, 0 if operation completed successfully
     */
    int setupTimer(sd_event_source* event);

    /** @brief Callback function for host state change.
     *
     *  @param[in] msg - data associated with subscribed signal
     */
    void hostStateHandler(sdbusplus::message::message& msg);

    /** @brief D-Bus IO callback used to handle incoming data on the opened file.
     *         See sd_event_io_handler_t for details.
     */
    static int ioCallback(sd_event_source* event, int fd, uint32_t revents, void* data);

    /** @brief D-Bus timer callback used to flush log store.
     *         See sd_event_add_time for details.
     */
    static int timerCallback(sd_event_source* event, uint64_t usec, void* data);

private:
    /** @struct FlushCondition
     *  @brief Describes flush conditions for log manager based on host state event.
     */
    struct FlushCondition
    {
        /** @brief D-Bus property name to watch. */
        std::string property;
        /** @brief Set of possible values for property watched. */
        std::set<std::string> values;
        /** @brief Match object to create D-Bus handler. */
        std::unique_ptr<sdbusplus::bus::match_t> match;
    };

private:
    /** @brief Log manager instance. */
    LogManager& logManager_;

    /** @brief D-Bus bus. */
    sdbusplus::bus::bus& bus_;

    /** @brief Log storage flush conditions.
     *         Map D-Bus interface name to condition description.
     */
    std::map<std::string, FlushCondition> conds_;
};
