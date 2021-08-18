# cmake 最低版本号要求
cmake_minimum_required(VERSION 2.8)

# 项目信息
project("WebServer")

set(CMAKE_BUILD_TYPE "Debug")

# 编译参数
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -lpthread -O0 -Wall -g -ggdb -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -lpthread -O3 -Wall -std=c++17")

# 设置可执行文件生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置头文件路径列表
set(INC_LIST
    ${CMAKE_SOURCE_DIR}/inc
)

# 包含头文件
include_directories(${INC_LIST})

# 添加cmake子目录
add_subdirectory(src)

file(GLOB TEST_SOURCES tests/*.cpp)
foreach(test_source_file ${TEST_SOURCES})
    string(REGEX MATCH "tests/.*" relative_path ${test_source_file})
    string(REPLACE "tests/" "" target_name ${relative_path})
    string(REPLACE ".cpp" "" target_name ${target_name})
    add_executable(${target_name} ${test_source_file})
    target_link_libraries(${target_name} source-objects)
endforeach(test_source_file ${TEST_SOURCES})