cmake_minimum_required(VERSION 3.15)
project(ESP32DriverIDE VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_WITH_SIMPLE_GUI "Build with simple native GUI (X11/Win32)" ON)
option(BUILD_TERMINAL_MODE "Build terminal mode executable" ON)

# Common source files
set(COMMON_SOURCES
    src/editor/text_editor.cpp
    src/editor/syntax_highlighter.cpp
    src/editor/tab_manager.cpp
    src/editor/autocomplete_engine.cpp
    src/editor/collaboration.cpp
    src/file_manager/file_manager.cpp
    src/file_manager/file_tree.cpp
    src/file_manager/project_templates.cpp
    src/collaboration/collaboration.cpp
    src/ai_assistant/ai_assistant.cpp
    src/compiler/esp32_compiler.cpp
    src/serial/serial_monitor.cpp
    src/emulator/vm_emulator.cpp
    src/gui/main_window.cpp
    src/gui/console_widget.cpp
    src/gui/integrated_terminal.cpp
    src/gui/device_library.cpp
    src/gui/panel_system.cpp
    src/gui/widgets.cpp
    src/gui/gui_wired_framework.cpp
    src/utils/string_utils.cpp
    src/utils/pretrained_model.cpp
    src/utils/ml_device_detector.cpp
    src/renderer/pure_c_renderer.cpp
    src/blueprint/blueprint_editor.cpp
    src/scripting/scripting_engine.cpp
    src/decompiler/advanced_decompiler.cpp
    src/testing/test_framework.cpp
    src/backend/backend_framework.cpp
    # Version 2.0.0 features
    src/platform/platform_expansion.cpp
    src/visualization/advanced_visualization.cpp
    src/plugins/plugin_system.cpp
)

# GUI-specific sources
if(BUILD_WITH_SIMPLE_GUI)
    set(GUI_SOURCES 
        src/gui/simple_gui_window.cpp
        src/gui/enhanced_gui_window.cpp
    )
else()
    set(GUI_SOURCES 
        src/gui/enhanced_gui_window.cpp
    )
endif()

# Source files
set(SOURCES
    src/main.cpp
    ${COMMON_SOURCES}
    ${GUI_SOURCES}
)

# Header files
set(HEADERS
    src/editor/text_editor.h
    src/editor/syntax_highlighter.h
    src/editor/tab_manager.h
    src/editor/autocomplete_engine.h
    src/editor/collaboration.h
    src/file_manager/file_manager.h
    src/file_manager/file_tree.h
    src/file_manager/project_templates.h
    src/collaboration/collaboration.h
    src/ai_assistant/ai_assistant.h
    src/compiler/esp32_compiler.h
    src/serial/serial_monitor.h
    src/emulator/vm_emulator.h
    src/gui/main_window.h
    src/gui/console_widget.h
    src/gui/integrated_terminal.h
    src/gui/simple_gui_window.h
    src/gui/enhanced_gui_window.h
    src/gui/device_library.h
    src/gui/panel_system.h
    src/gui/widgets.h
    src/gui/gui_wired_framework.h
    src/utils/string_utils.h
    src/utils/pretrained_model.h
    src/utils/ml_device_detector.h
    src/renderer/pure_c_renderer.h
    src/blueprint/blueprint_editor.h
    src/scripting/scripting_engine.h
    src/decompiler/advanced_decompiler.h
    src/testing/test_framework.h
    src/backend/backend_framework.h
    src/terminal/terminal_mode.h
    # Version 2.0.0 features
    src/platform/platform_expansion.h
    src/visualization/advanced_visualization.h
    src/plugins/plugin_system.h
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
    if(BUILD_WITH_SIMPLE_GUI)
        # Link with GDI32 for Windows GUI
        target_link_libraries(esp32-driver-ide gdi32 user32)
    endif()
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_MACOS)
    if(BUILD_WITH_SIMPLE_GUI)
        # Link with Cocoa framework for macOS GUI
        target_link_libraries(esp32-driver-ide "-framework Cocoa" "-framework CoreGraphics")
    endif()
elseif(UNIX)
    # Linux-specific settings
    target_compile_definitions(esp32-driver-ide PRIVATE PLATFORM_LINUX)
    # Note: Simple GUI on Linux uses framebuffer, no X11 required
endif()

# Add GUI build definition
if(BUILD_WITH_SIMPLE_GUI)
    target_compile_definitions(esp32-driver-ide PRIVATE USE_SIMPLE_GUI)
endif()

# Install rules
install(TARGETS esp32-driver-ide esp32-driver-ide-demo DESTINATION bin)

# VM Emulator demo executable
add_executable(esp32-vm-emulator-demo
    src/vm_emulator_demo.cpp
    src/emulator/vm_emulator.cpp
)

target_include_directories(esp32-vm-emulator-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Feature test executable
add_executable(esp32-driver-ide-feature-test
    src/feature_test.cpp
    src/editor/text_editor.cpp
    src/editor/tab_manager.cpp
    src/editor/autocomplete_engine.cpp
    src/file_manager/file_tree.cpp
    src/file_manager/project_templates.cpp
    src/collaboration/collaboration.cpp
)

target_include_directories(esp32-driver-ide-feature-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enhanced GUI test executable
add_executable(esp32-driver-ide-enhanced-test
    src/enhanced_gui_test.cpp
    ${COMMON_SOURCES}
    src/gui/device_library.cpp
    src/gui/panel_system.cpp
    src/gui/enhanced_gui_window.cpp
)

target_include_directories(esp32-driver-ide-enhanced-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ML Device Detection test executable
add_executable(esp32-ml-device-detection-test
    src/ml_device_detection_test.cpp
    src/utils/pretrained_model.cpp
    src/utils/ml_device_detector.cpp
)

target_include_directories(esp32-ml-device-detection-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Decompiler test executable
add_executable(esp32-decompiler-test
    src/decompiler_test.cpp
    src/decompiler/advanced_decompiler.cpp
)

target_include_directories(esp32-decompiler-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# GUI Wired Framework test executable
add_executable(esp32-gui-wired-framework-test
    src/gui_wired_framework_test.cpp
    ${COMMON_SOURCES}
    src/gui/enhanced_gui_window.cpp
)

target_include_directories(esp32-gui-wired-framework-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# GUI Wired Framework demo executable
add_executable(esp32-gui-wired-framework-demo
    src/gui_wired_framework_demo.cpp
    ${COMMON_SOURCES}
    src/gui/enhanced_gui_window.cpp
)

target_include_directories(esp32-gui-wired-framework-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Terminal Mode executable
if(BUILD_TERMINAL_MODE)
    add_executable(esp32-driver-ide-terminal
        src/terminal/terminal_main.cpp
        src/terminal/terminal_mode.cpp
        ${COMMON_SOURCES}
        src/gui/enhanced_gui_window.cpp
    )
    
    target_include_directories(esp32-driver-ide-terminal PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    target_compile_definitions(esp32-driver-ide-terminal PRIVATE TERMINAL_MODE)
    
    install(TARGETS esp32-driver-ide-terminal DESTINATION bin)
endif()
