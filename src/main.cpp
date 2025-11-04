#include "gui/imgui_window.h"
#include "gui/main_window.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    std::cout << "Starting ESP32 Driver IDE with ImGui...\n\n";
    
    try {
        // Create backend components
        auto text_editor = std::make_unique<esp32_ide::TextEditor>();
        auto file_manager = std::make_unique<esp32_ide::FileManager>();
        auto compiler = std::make_unique<esp32_ide::ESP32Compiler>();
        auto serial_monitor = std::make_unique<esp32_ide::SerialMonitor>();
        auto syntax_highlighter = std::make_unique<esp32_ide::SyntaxHighlighter>();
        
        // Create default sketch
        file_manager->CreateFile("sketch.ino", esp32_ide::FileManager::GetDefaultSketch());
        
        // Create ImGui window
        auto imgui_window = std::make_unique<esp32_ide::gui::ImGuiWindow>();
        
        // Initialize ImGui window
        if (!imgui_window->Initialize(1600, 900)) {
            std::cerr << "Failed to initialize ImGui window\n";
            return 1;
        }
        
        // Connect backend components to UI
        imgui_window->SetTextEditor(text_editor.get());
        imgui_window->SetFileManager(file_manager.get());
        imgui_window->SetCompiler(compiler.get());
        imgui_window->SetSerialMonitor(serial_monitor.get());
        imgui_window->SetSyntaxHighlighter(syntax_highlighter.get());
        
        // Run the application
        imgui_window->Run();
        
        // Cleanup
        imgui_window->Shutdown();
        
        std::cout << "\nESP32 Driver IDE closed successfully.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
