cmake_minimum_required(VERSION 3.14)

# 设置策略
if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

project(demo)

set(CMAKE_CXX_STANDARD 11)

# ipc_consumer
add_executable(
    ipc_consumer
    ${PROJECT_SOURCE_DIR}/ipc_consumer.cc
)

target_link_libraries(
    ipc_consumer
)

# ipc_producer
add_executable(
    ipc_producer
    ${PROJECT_SOURCE_DIR}/ipc_producer.cc
)

target_link_libraries(
    ipc_producer
)


