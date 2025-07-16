cmake_minimum_required(VERSION 3.16)

project(SmartPOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Concurrent Network PrintSupport Multimedia)

# Add ZXing-C++ for real barcode recognition
include(FetchContent)
FetchContent_Declare(
    ZXing
    GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
    GIT_TAG        v2.2.1  # 使用稳定版本
)
set(BUILD_READERS_ONLY ON)
set(BUILD_EXAMPLES OFF)
set(BUILD_BLACKBOX_TESTS OFF)
FetchContent_MakeAvailable(ZXing)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(src)

# Source files (excluding main.cpp for library)
set(CORE_SOURCES
    src/models/Product.cpp
    src/models/Customer.cpp
    src/models/Sale.cpp
    src/models/SaleItem.cpp
    src/controllers/ProductManager.cpp
    src/controllers/CheckoutController.cpp
    src/ui/MainWindow.cpp
    src/ui/ProductDialog.cpp
    src/ui/PaymentDialog.cpp
    src/ui/CartDelegate.cpp
    src/ui/RecommendationItemWidget.cpp
    src/ui/ProductManagementDialog.cpp
    src/ui/SalesReportDialog.cpp
    src/database/DatabaseManager.cpp
    src/barcode/BarcodeScanner.cpp
    src/ai/AIRecommender.cpp
    src/utils/ReceiptPrinter.cpp
)

# All sources including main.cpp
set(SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    src/ui/MainWindow.ui
)

# Header files
set(HEADERS
    src/models/Product.h
    src/models/Customer.h
    src/models/Sale.h
    src/models/SaleItem.h
    src/controllers/ProductManager.h
    src/controllers/CheckoutController.h
    src/ui/MainWindow.h
    src/ui/PaymentDialog.h
    src/ui/ProductDialog.h
    src/ui/ProductManagementDialog.h
    src/ui/RecommendationItemWidget.h
    src/ui/SalesReportDialog.h
    src/database/DatabaseManager.h
    src/barcode/BarcodeScanner.h
    src/ai/AIRecommender.h
    src/utils/ReceiptPrinter.h
)

# UI files
set(FORMS
    src/ui/MainWindow.ui
    src/ui/PaymentDialog.ui
    src/ui/ProductDialog.ui
    src/ui/ProductManagementDialog.ui
    src/ui/SalesReportDialog.ui
)

# Resource files
set(RESOURCES
    resources/resources.qrc
)

# Create a library for the core functionality (for testing)
add_library(SmartPOSCore STATIC ${CORE_SOURCES} ${HEADERS} ${RESOURCES})
target_link_libraries(SmartPOSCore PRIVATE
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

# Add include directories for auto-generated UI headers
target_include_directories(SmartPOSCore PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/SmartPOSCore_autogen/include
    ${CMAKE_BINARY_DIR}/_deps/zxing-src/core/src/zxing
)

# Create executable
add_executable(SmartPOS ${SOURCES})
target_link_libraries(SmartPOS PRIVATE
    SmartPOSCore
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

# Add include directories for executable
target_include_directories(SmartPOS PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/SmartPOS_autogen/include
)

# Compiler-specific options
if(MSVC)
    target_compile_options(SmartPOS PRIVATE /W4)
else()
    target_compile_options(SmartPOS PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
install(TARGETS SmartPOS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Copy Qt runtime dependencies (Windows)
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET SmartPOS POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:SmartPOS>
            COMMENT "Deploying Qt libraries")
    endif()
endif()

# Testing
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
