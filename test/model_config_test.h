#pragma once

#include <string>
#include <gtest/gtest.h>
#include "core/model_config.h"
#include "core/model_config_utils.h"

namespace test {

class ModelConfigTest : public testing::Test {
 protected:
  void SetUp() override {
    core::LoadModelConfigFormTextProto(path, &config);
  }
  void TearDown() override {
  }

  inference::ModelConfig config;
  std::string path = "D:/work/Git/AIEngine/test/data/config.pbtxt";
};

}