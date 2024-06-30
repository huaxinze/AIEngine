cmake_minimum_required(VERSION 3.14)

# 设置策略
if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)
project(ai_engine)
set(CMAKE_CXX_STANDARD 11)

# INCLUDE
include_directories(
    ${PROJECT_SOURCE_DIR}
)

# CORE_SRC
aux_source_directory(
    ${PROJECT_SOURCE_DIR}/core/
    CORE_SRC
)

# COMMON_SRC
aux_source_directory(
    ${PROJECT_SOURCE_DIR}/common/
    COMMON_SRC
)

# TEST_SRC
aux_source_directory(
    ${PROJECT_SOURCE_DIR}/test/
    TEST_SRC
)

# THIRD PARTY
## cpp=ipc
add_subdirectory(
    ${PROJECT_SOURCE_DIR}/thirdparty/cpp-ipc
)
include_directories(
    ${PROJECT_SOURCE_DIR}/thirdparty/cpp-ipc/include/
)
## rapidjson
include_directories(
    ${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include/
)
## protobuf
include(FetchContent)
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        v3.21.12
    SOURCE_SUBDIR  cmake
    GIT_PROGRESS   TRUE
    FIND_PACKAGE_ARGS NAMES protobuf
)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_MSVC_STATIC_RUNTIME OFF)
FetchContent_MakeAvailable(protobuf)
## googletest
include(FetchContent)
FetchContent_Declare(
    GTest
    GIT_PROGRESS TRUE
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG 5ec7f0c4a113e2f18ac2c6cc7df51ad6afc24081 # v1.11.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF)
FetchContent_MakeAvailable(GTest)

# 单元测试
enable_testing()
add_executable(
    ${PROJECT_NAME}_test
    ${CORE_SRC}
    ${COMMON_SRC}
    ${TEST_SRC}
)

target_link_libraries(
    ${PROJECT_NAME}_test
    ipc
    gtest_main
    libprotobuf
    libprotobuf-lite
)
