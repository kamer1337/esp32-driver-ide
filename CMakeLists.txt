cmake_minimum_required(VERSION 3.15)
project(ESP32DriverIDE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_GUI "Build GUI version with ImGui" ON)

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

# GUI version with ImGui
if(BUILD_GUI)
    include(FetchContent)
    
    # Fetch GLFW
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
    
    # Fetch ImGui
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.1
    )
    FetchContent_MakeAvailable(imgui)
    
    # Fetch ImGui Node Editor
    FetchContent_Declare(
        imgui_node_editor
        GIT_REPOSITORY https://github.com/thedmd/imgui-node-editor.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(imgui_node_editor)
    
    # Create ImGui library
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    
    target_link_libraries(imgui PUBLIC glfw)
    
    # Find OpenGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(imgui PUBLIC OpenGL::GL)
    
    # Create ImGui Node Editor library
    add_library(imgui_node_editor STATIC
        ${imgui_node_editor_SOURCE_DIR}/imgui_node_editor.cpp
        ${imgui_node_editor_SOURCE_DIR}/imgui_node_editor_api.cpp
        ${imgui_node_editor_SOURCE_DIR}/imgui_canvas.cpp
        ${imgui_node_editor_SOURCE_DIR}/crude_json.cpp
    )
    
    target_include_directories(imgui_node_editor PUBLIC
        ${imgui_node_editor_SOURCE_DIR}
    )
    
    target_link_libraries(imgui_node_editor PUBLIC imgui)
    
    # GUI executable sources
    set(GUI_SOURCES
        src/editor/text_editor.cpp
        src/editor/syntax_highlighter.cpp
        src/file_manager/file_manager.cpp
        src/ai_assistant/ai_assistant.cpp
        src/compiler/esp32_compiler.cpp
        src/serial/serial_monitor.cpp
        src/gui/console_widget.cpp
        src/gui/imgui_window.cpp
        src/gui/blueprint_editor.cpp
        src/utils/string_utils.cpp
    )
    
    # Create GUI executable
    add_executable(esp32-driver-ide-gui 
        src/main_gui.cpp
        ${GUI_SOURCES}
    )
    
    target_include_directories(esp32-driver-ide-gui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_node_editor_SOURCE_DIR}
    )
    
    target_link_libraries(esp32-driver-ide-gui PRIVATE imgui imgui_node_editor glfw OpenGL::GL)
    
    # Platform-specific settings for GUI
    if(WIN32)
        target_compile_definitions(esp32-driver-ide-gui PRIVATE PLATFORM_WINDOWS)
    elseif(APPLE)
        target_compile_definitions(esp32-driver-ide-gui PRIVATE PLATFORM_MACOS)
    elseif(UNIX)
        target_compile_definitions(esp32-driver-ide-gui PRIVATE PLATFORM_LINUX)
    endif()
    
    install(TARGETS esp32-driver-ide-gui DESTINATION bin)
endif()

# Install rules
install(TARGETS esp32-driver-ide esp32-driver-ide-demo DESTINATION bin)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
