# CMake generated Testfile for 
# Source directory: /Users/maomao/Project/normal_1/tests/integration
# Build directory: /Users/maomao/Project/normal_1/build/tests/integration
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestSmartPOSWorkflow "/Users/maomao/Project/normal_1/build/tests/TestSmartPOSWorkflow")
set_tests_properties(TestSmartPOSWorkflow PROPERTIES  TIMEOUT "60" WORKING_DIRECTORY "/Users/maomao/Project/normal_1/build" _BACKTRACE_TRIPLES "/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;15;add_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;24;add_integration_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;0;")
add_test(TestGUIAutomation "/Users/maomao/Project/normal_1/build/tests/TestGUIAutomation")
set_tests_properties(TestGUIAutomation PROPERTIES  TIMEOUT "60" WORKING_DIRECTORY "/Users/maomao/Project/normal_1/build" _BACKTRACE_TRIPLES "/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;15;add_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;27;add_integration_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;0;")
add_test(TestGUIBasic "/Users/maomao/Project/normal_1/build/tests/TestGUIBasic")
set_tests_properties(TestGUIBasic PROPERTIES  TIMEOUT "60" WORKING_DIRECTORY "/Users/maomao/Project/normal_1/build" _BACKTRACE_TRIPLES "/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;15;add_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;30;add_integration_test;/Users/maomao/Project/normal_1/tests/integration/CMakeLists.txt;0;")
