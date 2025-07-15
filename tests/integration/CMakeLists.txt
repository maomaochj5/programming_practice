# 集成测试CMakeLists.txt

function(add_integration_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    
    target_link_libraries(${test_name}
        SmartPOSCore
        TestCommon
        Qt6::Core
        Qt6::Test
        Qt6::Sql
        Qt6::Widgets
    )
    
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# SmartPOS工作流程集成测试
add_integration_test(TestSmartPOSWorkflow)

# GUI自动化测试
add_integration_test(TestGUIAutomation)

# 基础GUI测试（更稳定）
add_integration_test(TestGUIBasic)

# 创建运行所有集成测试的目标
add_custom_target(all_integration_tests
    DEPENDS 
        TestSmartPOSWorkflow
        TestGUIAutomation
        TestGUIBasic
)
