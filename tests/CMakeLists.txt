cmake_minimum_required(VERSION 3.16)

# 启用测试
enable_testing()

# 查找Qt6测试模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# 设置测试可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# 包含源代码目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 添加公共测试库
add_library(TestCommon STATIC
    TestCommon.cpp
    TestCommon.h
)

target_link_libraries(TestCommon
    Qt6::Core
    Qt6::Test
    Qt6::Sql
)

# 单元测试
add_subdirectory(unit)

# 集成测试
add_subdirectory(integration)

# 添加自定义目标来运行所有测试
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS all_unit_tests all_integration_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
