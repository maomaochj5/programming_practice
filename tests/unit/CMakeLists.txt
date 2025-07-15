# 单元测试CMakeLists.txt

# 创建一个函数来简化测试的创建
function(add_unit_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    
    target_link_libraries(${test_name}
        SmartPOSCore
        TestCommon
        Qt6::Core
        Qt6::Test
        Qt6::Sql
        Qt6::Widgets
    )
    
    # 将测试添加到CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # 设置测试属性
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# Product模型测试
add_unit_test(TestProduct)

# ProductManager控制器测试
add_unit_test(TestProductManager)

# CheckoutController控制器测试
add_unit_test(TestCheckoutController)

# DatabaseManager测试
add_unit_test(TestDatabaseManager)

# AIRecommender测试
add_unit_test(TestAIRecommender)

# 创建运行所有单元测试的目标
add_custom_target(all_unit_tests
    DEPENDS 
        TestProduct 
        TestProductManager
        TestCheckoutController
        TestDatabaseManager
        TestAIRecommender
)
