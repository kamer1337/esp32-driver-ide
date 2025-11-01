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
    src/utils/string_utils.h
)

# Create executable
add_executable(esp32-driver-ide ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(esp32-driver-ide PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_WINDOWS)
    # Link against Windows GUI libraries (optional - comment out for console build)
    # target_link_libraries(esp32-driver-ide PRIVATE gdi32 user32 comctl32 comdlg32)
    # set_target_properties(esp32-driver-ide PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_MACOS)
    # find_library(COCOA_LIBRARY Cocoa)
    # target_link_libraries(esp32-driver-ide PRIVATE ${COCOA_LIBRARY})
elseif(UNIX)
    # Linux-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_LINUX)
    # GTK+3 is optional - for full GUI support, install: sudo apt-get install libgtk-3-dev
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)
        if(GTK3_FOUND)
            message(STATUS "GTK+3 found - GUI support enabled")
            target_include_directories(esp32-driver-ide PRIVATE ${GTK3_INCLUDE_DIRS})
            target_link_libraries(esp32-driver-ide PRIVATE ${GTK3_LIBRARIES})
            target_compile_definitions(esp32-driver-ide PRIVATE HAS_GTK3)
        else()
            message(STATUS "GTK+3 not found - building console-only version")
        endif()
    endif()
endif()

# Install rules
install(TARGETS esp32-driver-ide DESTINATION bin)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
