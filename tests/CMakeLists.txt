# Test configuration for ESP32 Driver IDE

# Simple test executable that runs basic component tests
add_executable(basic_tests
    basic_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/text_editor.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/syntax_highlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/file_manager/file_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/string_utils.cpp
)

target_include_directories(basic_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add test to CTest
add_test(NAME BasicTests COMMAND basic_tests)

# VM Emulator tests
add_executable(vm_emulator_tests
    vm_emulator_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/emulator/vm_emulator.cpp
)

target_include_directories(vm_emulator_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add VM emulator test to CTest
add_test(NAME VMEmulatorTests COMMAND vm_emulator_tests)

# Version 1.3.0 feature tests
add_executable(version_1_3_0_tests
    version_1_3_0_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/ai_assistant/ai_assistant.cpp
    ${CMAKE_SOURCE_DIR}/src/collaboration/collaboration.cpp
    ${CMAKE_SOURCE_DIR}/src/testing/test_framework.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/string_utils.cpp
)

target_include_directories(version_1_3_0_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add version 1.3.0 tests to CTest
add_test(NAME Version1_3_0Tests COMMAND version_1_3_0_tests)

# Version 2.0.0 feature tests
add_executable(version_2_0_0_tests
    version_2_0_0_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/platform/platform_expansion.cpp
    ${CMAKE_SOURCE_DIR}/src/visualization/advanced_visualization.cpp
    ${CMAKE_SOURCE_DIR}/src/plugins/plugin_system.cpp
)

target_include_directories(version_2_0_0_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add version 2.0.0 tests to CTest
add_test(NAME Version2_0_0Tests COMMAND version_2_0_0_tests)
