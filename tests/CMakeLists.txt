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
