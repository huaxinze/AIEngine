#include "model_config_test.h"

#include <iostream>

using namespace core;

namespace test {

TEST_F(ModelConfigTest, ValidateModelInput) {
  Status status = ValidateModelConfig(config, 0.0);
  EXPECT_EQ(status.IsOk(), true);
}

}