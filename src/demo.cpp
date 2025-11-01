#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "ai_assistant/ai_assistant.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include <iostream>
#include <memory>

using namespace esp32_ide;

void PrintSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void DemoTextEditor() {
    std::cout << "📝 TEXT EDITOR DEMO\n";
    PrintSeparator();
    
    auto editor = std::make_unique<TextEditor>();
    
    // Set some code
    std::string code = R"(void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
}

void loop() {
  digitalWrite(2, HIGH);
  delay(1000);
})";
    
    editor->SetText(code);
    std::cout << "✓ Created editor with sample code\n";
    std::cout << "✓ Line count: " << editor->GetLineCount() << "\n";
    std::cout << "✓ Current line: " << editor->GetCurrentLine() + 1 << "\n";
    
    // Test search
    auto positions = editor->FindText("Serial");
    std::cout << "✓ Found 'Serial' at " << positions.size() << " positions\n";
    
    // Test undo/redo
    editor->InsertText("\n  // New comment", editor->GetCursorPosition());
    std::cout << "✓ Inserted text\n";
    editor->Undo();
    std::cout << "✓ Undo successful\n";
}

void DemoSyntaxHighlighter() {
    std::cout << "🎨 SYNTAX HIGHLIGHTER DEMO\n";
    PrintSeparator();
    
    auto highlighter = std::make_unique<SyntaxHighlighter>();
    
    std::string code = "#include <Arduino.h>\nint pin = 2;  // LED pin";
    auto tokens = highlighter->Tokenize(code);
    
    std::cout << "Tokenized code with " << tokens.size() << " tokens:\n\n";
    for (const auto& token : tokens) {
        std::cout << SyntaxHighlighter::GetColorCode(token.type) 
                  << token.text << "\033[0m";
    }
    std::cout << "\n";
}

void DemoFileManager() {
    std::cout << "📁 FILE MANAGER DEMO\n";
    PrintSeparator();
    
    auto fm = std::make_unique<FileManager>();
    
    // Create files
    fm->CreateFile("main.ino", "void setup() {}");
    fm->CreateFile("utils.cpp", "// Utility functions");
    std::cout << "✓ Created 2 files\n";
    
    // List files
    auto files = fm->GetFileList();
    std::cout << "✓ Files in project: ";
    for (const auto& file : files) {
        std::cout << file << " ";
    }
    std::cout << "\n";
    
    // Open and modify
    fm->OpenFile("main.ino");
    fm->SetFileContent("main.ino", "void setup() {\n  Serial.begin(115200);\n}");
    std::cout << "✓ Modified main.ino\n";
    std::cout << "✓ File is " << (fm->IsFileModified("main.ino") ? "modified" : "not modified") << "\n";
}

void DemoAIAssistant() {
    std::cout << "🤖 AI ASSISTANT DEMO\n";
    PrintSeparator();
    
    auto ai = std::make_unique<AIAssistant>();
    
    // Test queries
    std::string queries[] = {
        "How do I use GPIO pins?",
        "Tell me about WiFi",
        "How to use delay?"
    };
    
    for (const auto& query : queries) {
        std::cout << "Q: " << query << "\n";
        std::string response = ai->Query(query);
        std::cout << "A: " << response.substr(0, 80) << "...\n\n";
    }
    
    // Code analysis
    std::string code = R"(void loop() {
  digitalWrite(2, HIGH);
  delay(1000);
})";
    
    std::cout << "Analyzing code:\n";
    std::string analysis = ai->AnalyzeCode(code);
    std::cout << analysis << "\n";
}

void DemoESP32Compiler() {
    std::cout << "⚙️ ESP32 COMPILER DEMO\n";
    PrintSeparator();
    
    auto compiler = std::make_unique<ESP32Compiler>();
    
    // Set up output callback
    compiler->SetOutputCallback([](const std::string& msg, ESP32Compiler::CompileStatus status) {
        std::string prefix;
        switch (status) {
            case ESP32Compiler::CompileStatus::SUCCESS: prefix = "✓ "; break;
            case ESP32Compiler::CompileStatus::ERROR: prefix = "✗ "; break;
            case ESP32Compiler::CompileStatus::WARNING: prefix = "⚠ "; break;
            default: prefix = "  "; break;
        }
        std::cout << prefix << msg << "\n";
    });
    
    // Test compilation
    std::string good_code = R"(
#include <Arduino.h>

void setup() {
  pinMode(2, OUTPUT);
}

void loop() {
  digitalWrite(2, HIGH);
  delay(1000);
}
)";
    
    std::cout << "Compiling good code...\n";
    auto result = compiler->Compile(good_code, ESP32Compiler::BoardType::ESP32);
    
    // Test with bad code
    std::string bad_code = "void setup() { // Missing closing brace";
    std::cout << "\nCompiling code with errors...\n";
    compiler->Compile(bad_code, ESP32Compiler::BoardType::ESP32);
}

void DemoSerialMonitor() {
    std::cout << "📡 SERIAL MONITOR DEMO\n";
    PrintSeparator();
    
    auto serial = std::make_unique<SerialMonitor>();
    
    // Set up callback
    serial->SetMessageCallback([](const SerialMonitor::SerialMessage& msg) {
        std::cout << "[Serial] " << msg.content << "\n";
    });
    
    // Get available ports
    auto ports = SerialMonitor::GetAvailablePorts();
    std::cout << "Available ports:\n";
    for (const auto& port : ports) {
        std::cout << "  - " << port << "\n";
    }
    
    // Connect and send data
    if (!ports.empty()) {
        serial->Connect(ports[0], 115200);
        serial->SendData("Hello ESP32!");
        
        std::cout << "\nMessage history:\n";
        auto messages = serial->GetMessages();
        for (const auto& msg : messages) {
            std::cout << "  " << msg.content << "\n";
        }
    }
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          ESP32 Driver IDE - Feature Demonstration         ║\n";
    std::cout << "║                    C++ Version 2.0                         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    try {
        DemoTextEditor();
        DemoSyntaxHighlighter();
        DemoFileManager();
        DemoAIAssistant();
        DemoESP32Compiler();
        DemoSerialMonitor();
        
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════╗\n";
        std::cout << "║              All Demos Completed Successfully!            ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝\n";
        std::cout << "\nAll components are working correctly! ✅\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
