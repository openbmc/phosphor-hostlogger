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

#include "config.hpp"
#include "dbus_watch.hpp"
#include <set>
#include <string>
#include <chrono>

// D-Bus path to the host state object
#define DBUS_HOST_OBJECT_PATH "/xyz/openbmc_project/state/host0"

// Macro to normalize SDBus status code:
// positive code is not an error in the systemd dbus implementation.
#define DBUS_RC_TO_ERR(c) (c = (c <= 0 ? -c : 0))


DbusWatcher::DbusWatcher(LogManager& logManager, sdbusplus::bus::bus& bus)
: logManager_(logManager),
  bus_(bus)
{
}


int DbusWatcher::initialize()
{
    int rc;

    // Add IO callback for host's log stream socket
    rc = sd_event_add_io(bus_.get_event(), NULL,
                         logManager_.getHostLogFd(),
                         EPOLLIN, &DbusWatcher::ioCallback, this);
    if (DBUS_RC_TO_ERR(rc)) {
        fprintf(stderr, "Unable to add IO handler: %i %s\n",
                        rc, strerror(rc));
        return rc;
    }

    // Add flush handler
    if (loggerConfig.flushPeriod == 0)
        registerEventHandler();
    else
        rc = registerTimerHandler();

    return rc;
}


void DbusWatcher::registerEventHandler()
{
    conds_["xyz.openbmc_project.State.Host"] = {
        .property = "RequestedHostTransition",
        .values = {
            "xyz.openbmc_project.State.Host.Transition.On"
        }
    };
    conds_["xyz.openbmc_project.State.OperatingSystem.Status"] = {
        .property = "OperatingSystemState",
        .values = {
            "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.BootComplete",
            "xyz.openbmc_project.State.OperatingSystem.Status.OSStatus.Inactive"
        }
    };
    for (auto& cond: conds_) {
        cond.second.match = std::make_unique<sdbusplus::bus::match_t>(bus_,
                            sdbusplus::bus::match::rules::propertiesChanged(
                            DBUS_HOST_OBJECT_PATH, cond.first),
                            [this](auto& msg){ this->hostStateHandler(msg); });
    }
}


int DbusWatcher::registerTimerHandler()
{
    int rc;
    sd_event_source* ev = NULL;

    rc = sd_event_add_time(bus_.get_event(), &ev, CLOCK_MONOTONIC,
                           UINT64_MAX, 0, &DbusWatcher::timerCallback, this);
    if (DBUS_RC_TO_ERR(rc)) {
        fprintf(stderr, "Unable to add timer handler: %i %s\n", rc, strerror(rc));
        return rc;
    }

    rc = sd_event_source_set_enabled(ev, SD_EVENT_ON);
    if (DBUS_RC_TO_ERR(rc)) {
        fprintf(stderr, "Unable to enable timer handler: %i %s\n", rc, strerror(rc));
        return rc;
    }

    return setupTimer(ev);
}


int DbusWatcher::setupTimer(sd_event_source* event)
{
    // Get the current time and add the delta (flush period)
    using namespace std::chrono;
    auto now = steady_clock::now().time_since_epoch();
    hours timeOut(loggerConfig.flushPeriod);
    auto expireTime = duration_cast<microseconds>(now) +
                      duration_cast<microseconds>(timeOut);

    //Set the time
    int rc = sd_event_source_set_time(event, expireTime.count());
    if (DBUS_RC_TO_ERR(rc))
        fprintf(stderr, "Unable to set timer handler: %i %s\n", rc, strerror(rc));

    return rc;
}


void DbusWatcher::hostStateHandler(sdbusplus::message::message& msg)
{
    std::map<std::string, sdbusplus::message::variant<std::string>> properties;
    std::string interface;

    msg.read(interface, properties);

    bool needFlush = false;
    const auto itc = conds_.find(interface);
    if (itc != conds_.end()) {
        const auto itp = properties.find(itc->second.property);
        if (itp != properties.end()) {
            const auto& propVal = itp->second.get<std::string>();
            needFlush = itc->second.values.find(propVal) != itc->second.values.end();
        }
    }

    if (needFlush)
        logManager_.flush();
}


int DbusWatcher::ioCallback(sd_event_source* /*event*/, int /*fd*/, uint32_t /*revents*/, void* data)
{
    DbusWatcher* instance = static_cast<DbusWatcher*>(data);
    instance->logManager_.handleHostLog();
    return 0;
}


int DbusWatcher::timerCallback(sd_event_source* event, uint64_t /*usec*/, void* data)
{
    DbusWatcher* instance = static_cast<DbusWatcher*>(data);
    instance->logManager_.flush();
    return instance->setupTimer(event);
}
