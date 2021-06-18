// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 YADRO

#pragma once

/**
 * @class Service
 * @brief The log service interface
 */
class Service
{
  public:
    virtual ~Service() = default;

    /**
     * @brief Run the service.
     */
    virtual void run() = 0;
};
