cmake_minimum_required(VERSION 3.16)

# 启用测试
enable_testing()

# 查找Qt6测试模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# 设置测试可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# 包含源代码目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 只保留GUI调试测试
add_executable(DebugGUITest
    debug_gui_test.cpp
)

target_link_libraries(DebugGUITest
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::Sql
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Multimedia
    SmartPOSCore
)

# Add include directories for auto-generated UI headers
target_include_directories(DebugGUITest PRIVATE
    ${CMAKE_BINARY_DIR}/SmartPOSCore_autogen/include
)

# 添加GUI调试测试到CTest
add_test(NAME DebugGUITest COMMAND DebugGUITest)
