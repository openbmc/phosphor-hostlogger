// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 GOOGLE

#pragma once

#include "file_storage.hpp"

#include <gmock/gmock.h>

class FileStorageMock : public FileStorage
{
  public:
    FileStorageMock() : FileStorage("/tmp", "fake", -1) {}
    MOCK_METHOD(std::string, save, (const LogBuffer& buf), (const override));
};
