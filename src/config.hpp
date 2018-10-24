/**
 * @brief Logger configuration.
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

/** @struct Config
 *  @brief Represent the configuration of the logger.
 */
struct Config
{
    /** @brief Path to write log files. */
    const char* path;
    /** @brief Storage limit (message count). */
    int storageSizeLimit;
    /** @brief Storage limit (max time). */
    int storageTimeLimit;
    /** @brief Flush policy: save logs every flushPeriod hours. */
    int flushPeriod;
    /** @brief Rotation limit (max files to store). */
    int rotationLimit;
};

/** @brief Global logger configuration instance. */
extern Config loggerConfig;
