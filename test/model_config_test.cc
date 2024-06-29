#include <iostream>
#include <fstream>
#include <set>
#include <gtest/gtest.h>
#include <google/protobuf/text_format.h>
#include "../core/model_config.pb.h"

class TestDemo : public testing::Test {
  protected:
    int* myObject;

    void SetUp() override {
        myObject = new int(42); // 示例初始化
    }

    void TearDown() override {
        delete myObject;
    }
};

// 使用 TEST_F() 进行测试
TEST_F(TestDemo, Test1) {
    // 可以在这里访问 myObject
    EXPECT_EQ(*myObject, 42);
}

TEST_F(TestDemo, Test2) {
    // 也可以在这里访问 myObject
    EXPECT_NE(*myObject, 0);
}