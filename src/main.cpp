#include "gui/terminal_window.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    std::cout << "Starting ESP32 Driver IDE (Terminal Version)...\n\n";
    
    try {
        // Create backend components
        auto text_editor = std::make_unique<esp32_ide::TextEditor>();
        auto file_manager = std::make_unique<esp32_ide::FileManager>();
        auto compiler = std::make_unique<esp32_ide::ESP32Compiler>();
        auto serial_monitor = std::make_unique<esp32_ide::SerialMonitor>();
        auto syntax_highlighter = std::make_unique<esp32_ide::SyntaxHighlighter>();
        
        // Create default sketch
        file_manager->CreateFile("sketch.ino", esp32_ide::FileManager::GetDefaultSketch());
        
        // Create terminal window
        auto terminal_window = std::make_unique<esp32_ide::gui::TerminalWindow>();
        
        // Initialize terminal window
        if (!terminal_window->Initialize(80, 24)) {
            std::cerr << "Failed to initialize terminal window\n";
            return 1;
        }
        
        // Connect backend components to UI
        terminal_window->SetTextEditor(text_editor.get());
        terminal_window->SetFileManager(file_manager.get());
        terminal_window->SetCompiler(compiler.get());
        terminal_window->SetSerialMonitor(serial_monitor.get());
        terminal_window->SetSyntaxHighlighter(syntax_highlighter.get());
        
        // Run the application
        terminal_window->Run();
        
        // Cleanup
        terminal_window->Shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
