cmake_minimum_required(VERSION 3.16)
project(SmartPOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Concurrent Network PrintSupport Multimedia Test)

include(FetchContent)
FetchContent_Declare(
    ZXing
    GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
    GIT_TAG        v2.2.1
)
set(BUILD_READERS_ONLY ON)
set(BUILD_EXAMPLES OFF)
set(BUILD_BLACKBOX_TESTS OFF)
FetchContent_MakeAvailable(ZXing)

include_directories(src)

set(APP_SOURCES
    src/main.cpp
    src/database/DatabaseManager.cpp
    src/models/Product.cpp
    src/models/Sale.cpp
    src/models/SaleItem.cpp
    src/models/Customer.cpp
    src/controllers/CheckoutController.cpp
    src/controllers/ProductManager.cpp
    src/ai/AIClient.cpp
    src/ai/AIRecommender.cpp
    src/barcode/BarcodeScanner.cpp
    src/utils/ReceiptPrinter.cpp
    src/ui/MainWindow.cpp
    src/ui/ProductDialog.cpp
    src/ui/PaymentDialog.cpp
    src/ui/SalesReportDialog.cpp
    src/ui/ProductManagementDialog.cpp
    src/ui/RecommendationDialog.cpp
    src/ui/RecommendationItemWidget.cpp
    src/ui/CartDelegate.cpp
    ${PROJECT_SOURCE_DIR}/resources/resources.qrc
)

add_executable(SmartPOS ${APP_SOURCES})

target_link_libraries(SmartPOS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Concurrent
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Multimedia
    ZXing::ZXing
)

target_include_directories(SmartPOS PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT MSVC)
    target_compile_options(SmartPOS PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS SmartPOS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET SmartPOS POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:SmartPOS>
            COMMENT "Deploying Qt libraries")
    endif()
endif()

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(debug_gui_test tests/debug_gui_test.cpp)
    target_link_libraries(debug_gui_test PRIVATE Qt6::Widgets SmartPOS)

    add_executable(test_gui_interface tests/integration/TestGUIInterface.cpp)
    target_link_libraries(test_gui_interface PRIVATE Qt6::Test Qt6::Widgets SmartPOS)
endif()

add_executable(import_products import_products.cpp)
target_link_libraries(import_products PRIVATE Qt6::Core Qt6::Sql)