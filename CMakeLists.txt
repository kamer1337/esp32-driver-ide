cmake_minimum_required(VERSION 3.15)
project(ESP32DriverIDE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test programs" OFF)

# Source files
set(SOURCES
    src/main.cpp
    src/editor/text_editor.cpp
    src/editor/syntax_highlighter.cpp
    src/file_manager/file_manager.cpp
    src/ai_assistant/ai_assistant.cpp
    src/compiler/esp32_compiler.cpp
    src/serial/serial_monitor.cpp
    src/gui/main_window.cpp
    src/gui/console_widget.cpp
    src/gui/terminal_window.cpp
    src/utils/string_utils.cpp
)

# Header files
set(HEADERS
    src/editor/text_editor.h
    src/editor/syntax_highlighter.h
    src/file_manager/file_manager.h
    src/ai_assistant/ai_assistant.h
    src/compiler/esp32_compiler.h
    src/serial/serial_monitor.h
    src/gui/main_window.h
    src/gui/console_widget.h
    src/gui/terminal_window.h
    src/utils/string_utils.h
)

# Create executable
add_executable(esp32-driver-ide ${SOURCES} ${HEADERS})

# Create demo executable
add_executable(esp32-driver-ide-demo 
    src/demo.cpp
    src/editor/text_editor.cpp
    src/editor/syntax_highlighter.cpp
    src/file_manager/file_manager.cpp
    src/ai_assistant/ai_assistant.cpp
    src/compiler/esp32_compiler.cpp
    src/serial/serial_monitor.cpp
    src/gui/console_widget.cpp
    src/utils/string_utils.cpp
)

# Include directories
target_include_directories(esp32-driver-ide PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(esp32-driver-ide-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_WINDOWS)
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_MACOS)
elseif(UNIX)
    # Linux-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_LINUX)
endif()

# Install rules
install(TARGETS esp32-driver-ide esp32-driver-ide-demo DESTINATION bin)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
