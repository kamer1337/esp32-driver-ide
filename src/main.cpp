#include "gui/terminal_window.h"
#ifdef USE_SIMPLE_GUI
#include "gui/simple_gui_window.h"
#endif
#ifndef USE_TERMINAL_UI
#include "gui/enhanced_gui_window.h"
#endif
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
#ifdef USE_TERMINAL_UI
    std::cout << "Starting ESP32 Driver IDE (Terminal Version)...\n\n";
#else
    std::cout << "Starting ESP32 Driver IDE (Enhanced GUI Version)...\n\n";
#endif
    
    try {
        // Create backend components
        auto text_editor = std::make_unique<esp32_ide::TextEditor>();
        auto file_manager = std::make_unique<esp32_ide::FileManager>();
        auto compiler = std::make_unique<esp32_ide::ESP32Compiler>();
        auto serial_monitor = std::make_unique<esp32_ide::SerialMonitor>();
        auto syntax_highlighter = std::make_unique<esp32_ide::SyntaxHighlighter>();
        
        // Create default sketch
        file_manager->CreateFile("sketch.ino", esp32_ide::FileManager::GetDefaultSketch());
        
#ifdef USE_TERMINAL_UI
        // Create terminal window
        auto window = std::make_unique<esp32_ide::gui::TerminalWindow>();
        
        // Initialize terminal window
        if (!window->Initialize(80, 24)) {
            std::cerr << "Failed to initialize terminal window\n";
            return 1;
        }
#else
        // Create enhanced GUI window
        auto window = std::make_unique<esp32_ide::gui::EnhancedGuiWindow>();
        
        // Initialize enhanced GUI window
        if (!window->Initialize(1280, 800)) {
            std::cerr << "Failed to initialize enhanced GUI window\n";
            return 1;
        }
#endif
        
        // Connect backend components to UI
        window->SetTextEditor(text_editor.get());
        window->SetFileManager(file_manager.get());
        window->SetCompiler(compiler.get());
        window->SetSerialMonitor(serial_monitor.get());
        window->SetSyntaxHighlighter(syntax_highlighter.get());
        
        // Run the application
        window->Run();
        
        // Cleanup
        window->Shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
