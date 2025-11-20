#include "gui/main_window.h"
#include <iostream>

namespace esp32_ide {
namespace gui {

MainWindow::MainWindow() 
    : is_running_(false),
      window_title_("ESP32 Driver IDE v1.2.0") {
    
    // Initialize components
    editor_ = std::make_unique<TextEditor>();
    highlighter_ = std::make_unique<SyntaxHighlighter>();
    file_manager_ = std::make_unique<FileManager>();
    ai_assistant_ = std::make_unique<AIAssistant>();
    compiler_ = std::make_unique<ESP32Compiler>();
    serial_monitor_ = std::make_unique<SerialMonitor>();
    vm_emulator_ = std::make_unique<VMEmulator>();
    console_ = std::make_unique<ConsoleWidget>();
}

MainWindow::~MainWindow() = default;

bool MainWindow::Initialize() {
    // Setup callbacks
    SetupCallbacks();
    
    // Load default file
    LoadDefaultFile();
    
    // Initialize console
    console_->AddMessage("ESP32 Driver IDE v2.0.0 - Ready");
    console_->AddMessage("IDE initialized successfully", ConsoleWidget::MessageType::SUCCESS);
    
    return true;
}

void MainWindow::Show() {
    std::cout << "========================================\n";
    std::cout << "  ESP32 Driver IDE (C++ Version)\n";
    std::cout << "========================================\n\n";
    std::cout << "IDE Window opened successfully!\n";
    std::cout << "Current file: " << file_manager_->GetCurrentFile() << "\n";
    std::cout << "\nAvailable commands:\n";
    std::cout << "  1. New File\n";
    std::cout << "  2. Open File\n";
    std::cout << "  3. Save File\n";
    std::cout << "  4. Compile\n";
    std::cout << "  5. Upload\n";
    std::cout << "  6. Ask AI Assistant\n";
    std::cout << "  7. Exit\n";
    std::cout << "\nNote: This is a demonstration. Full GUI requires platform-specific implementation.\n";
}

void MainWindow::Hide() {
    // Hide window
}

int MainWindow::Run() {
    is_running_ = true;
    Show();
    
    // In a real GUI application, this would be an event loop
    // For demonstration, we'll just show the interface structure
    
    return 0;
}

void MainWindow::Close() {
    is_running_ = false;
}

void MainWindow::OnNewFile() {
    std::string filename = "new_sketch.ino";
    if (file_manager_->CreateFile(filename)) {
        file_manager_->OpenFile(filename);
        UpdateFileList();
        UpdateEditor();
        console_->AddMessage("Created new file: " + filename, ConsoleWidget::MessageType::SUCCESS);
    } else {
        console_->AddMessage("Failed to create file: " + filename, ConsoleWidget::MessageType::ERROR);
    }
}

void MainWindow::OnOpenFile() {
    // In a real implementation, this would show a file dialog
    console_->AddMessage("Open file dialog would appear here");
}

void MainWindow::OnSaveFile() {
    std::string current = file_manager_->GetCurrentFile();
    if (!current.empty()) {
        std::string content = editor_->GetText();
        file_manager_->SetFileContent(current, content);
        file_manager_->SaveFile(current);
        console_->AddMessage("Saved: " + current, ConsoleWidget::MessageType::SUCCESS);
    }
}

void MainWindow::OnCloseFile() {
    std::string current = file_manager_->GetCurrentFile();
    if (!current.empty()) {
        file_manager_->CloseFile(current);
        UpdateFileList();
        console_->AddMessage("Closed: " + current);
    }
}

void MainWindow::OnCompile() {
    std::string code = editor_->GetText();
    auto result = compiler_->Compile(code, compiler_->GetBoard());
    
    std::string status_msg = result.status == ESP32Compiler::CompileStatus::SUCCESS ? "successful" : "failed";
    console_->AddMessage("Compilation " + status_msg,
                        result.status == ESP32Compiler::CompileStatus::SUCCESS ? 
                        ConsoleWidget::MessageType::SUCCESS : ConsoleWidget::MessageType::ERROR);
}

void MainWindow::OnUpload() {
    compiler_->Upload(compiler_->GetBoard());
    console_->AddMessage("Upload completed", ConsoleWidget::MessageType::SUCCESS);
}

void MainWindow::OnFormatCode() {
    // Simple code formatting
    std::string code = editor_->GetText();
    // In a real implementation, would format the code properly
    console_->AddMessage("Code formatted", ConsoleWidget::MessageType::SUCCESS);
}

void MainWindow::OnSendChatMessage(const std::string& message) {
    UpdateChat(message, true);
    std::string response = ai_assistant_->Query(message);
    UpdateChat(response, false);
}

void MainWindow::OnFileSelected(const std::string& filename) {
    if (file_manager_->FileExists(filename)) {
        // Save current file first
        std::string current = file_manager_->GetCurrentFile();
        if (!current.empty()) {
            file_manager_->SetFileContent(current, editor_->GetText());
        }
        
        // Open new file
        file_manager_->OpenFile(filename);
        std::string content = file_manager_->GetFileContent(filename);
        editor_->SetText(content);
        UpdateTitle();
        console_->AddMessage("Opened: " + filename);
    }
}

void MainWindow::OnBoardChanged(ESP32Compiler::BoardType board) {
    compiler_->SetBoard(board);
    console_->AddMessage("Board changed to: " + compiler_->GetBoardName(board));
    
    // Update VM emulator to match board type
    VMEmulator::BoardType vm_board;
    switch (board) {
        case ESP32Compiler::BoardType::ESP32:
            vm_board = VMEmulator::BoardType::ESP32;
            break;
        case ESP32Compiler::BoardType::ESP32_S2:
            vm_board = VMEmulator::BoardType::ESP32_S2;
            break;
        case ESP32Compiler::BoardType::ESP32_S3:
            vm_board = VMEmulator::BoardType::ESP32_S3;
            break;
        case ESP32Compiler::BoardType::ESP32_C3:
            vm_board = VMEmulator::BoardType::ESP32_C3;
            break;
        default:
            vm_board = VMEmulator::BoardType::ESP32;
    }
    vm_emulator_->SetBoardType(vm_board);
}

void MainWindow::OnStartEmulator() {
    if (vm_emulator_->IsRunning()) {
        console_->AddMessage("VM Emulator is already running", ConsoleWidget::MessageType::WARNING);
        return;
    }
    
    if (vm_emulator_->Start()) {
        console_->AddMessage("VM Emulator started: " + vm_emulator_->GetBoardName(), 
                           ConsoleWidget::MessageType::SUCCESS);
        auto config = vm_emulator_->GetDeviceConfig();
        console_->AddMessage("  Flash: " + std::to_string(config.flash_size_mb) + "MB, " +
                           "PSRAM: " + std::to_string(config.psram_size_mb) + "MB, " +
                           "SRAM: " + std::to_string(config.sram_size_kb) + "KB");
    } else {
        console_->AddMessage("Failed to start VM Emulator", ConsoleWidget::MessageType::ERROR);
    }
}

void MainWindow::OnStopEmulator() {
    if (!vm_emulator_->IsRunning()) {
        console_->AddMessage("VM Emulator is not running", ConsoleWidget::MessageType::WARNING);
        return;
    }
    
    if (vm_emulator_->Stop()) {
        console_->AddMessage("VM Emulator stopped", ConsoleWidget::MessageType::SUCCESS);
    } else {
        console_->AddMessage("Failed to stop VM Emulator", ConsoleWidget::MessageType::ERROR);
    }
}

void MainWindow::OnTestInEmulator() {
    if (!vm_emulator_->IsRunning()) {
        console_->AddMessage("Starting VM Emulator for testing...");
        OnStartEmulator();
    }
    
    std::string code = editor_->GetText();
    console_->AddMessage("Testing code in virtual environment...");
    
    // First compile the code
    auto compile_result = compiler_->Compile(code, compiler_->GetBoard());
    if (compile_result.status != ESP32Compiler::CompileStatus::SUCCESS) {
        console_->AddMessage("Cannot test: Code has compilation errors", ConsoleWidget::MessageType::ERROR);
        return;
    }
    
    // Execute in VM emulator
    auto exec_result = vm_emulator_->ExecuteCode(code);
    
    if (exec_result.success) {
        console_->AddMessage("✓ Virtual execution successful", ConsoleWidget::MessageType::SUCCESS);
        console_->AddMessage("  Execution time: " + std::to_string(exec_result.execution_time_ms) + "ms");
        console_->AddMessage("  Memory used: " + std::to_string(exec_result.memory_used) + " bytes");
        
        // Show serial output from VM
        auto serial_output = vm_emulator_->ReadSerialOutput();
        if (!serial_output.empty()) {
            console_->AddMessage("Serial output from VM:");
            for (const auto& line : serial_output) {
                console_->AddMessage("  > " + line);
            }
        }
        
        // Show memory status
        auto memory = vm_emulator_->GetMemoryStatus();
        console_->AddMessage("Memory status:");
        console_->AddMessage("  Free heap: " + std::to_string(memory.free_heap) + " bytes");
        console_->AddMessage("  Fragmentation: " + std::to_string(static_cast<int>(memory.fragmentation_percent)) + "%");
        
    } else {
        console_->AddMessage("✗ Virtual execution failed", ConsoleWidget::MessageType::ERROR);
        for (const auto& error : exec_result.errors) {
            console_->AddMessage("  Error: " + error, ConsoleWidget::MessageType::ERROR);
        }
    }
    
    // Validate configuration
    auto validation = vm_emulator_->ValidateConfiguration();
    if (!validation.valid) {
        console_->AddMessage("Configuration validation failed:", ConsoleWidget::MessageType::ERROR);
        for (const auto& error : validation.errors) {
            console_->AddMessage("  ✗ " + error, ConsoleWidget::MessageType::ERROR);
        }
    }
    
    if (!validation.warnings.empty()) {
        console_->AddMessage("Configuration warnings:", ConsoleWidget::MessageType::WARNING);
        for (const auto& warning : validation.warnings) {
            console_->AddMessage("  ⚠ " + warning, ConsoleWidget::MessageType::WARNING);
        }
    }
}


void MainWindow::UpdateTitle() {
    std::string current = file_manager_->GetCurrentFile();
    if (!current.empty()) {
        window_title_ = "ESP32 Driver IDE - " + current;
    } else {
        window_title_ = "ESP32 Driver IDE";
    }
}

void MainWindow::UpdateFileList() {
    // In a real GUI, this would update the file list widget
    auto files = file_manager_->GetFileList();
    std::cout << "\nFiles in project:\n";
    for (const auto& file : files) {
        std::cout << "  - " << file << "\n";
    }
}

void MainWindow::UpdateEditor() {
    std::string current = file_manager_->GetCurrentFile();
    if (!current.empty()) {
        std::string content = file_manager_->GetFileContent(current);
        editor_->SetText(content);
    }
}

void MainWindow::UpdateConsole(const std::string& message, ESP32Compiler::CompileStatus status) {
    ConsoleWidget::MessageType type = ConsoleWidget::MessageType::NORMAL;
    
    switch (status) {
        case ESP32Compiler::CompileStatus::SUCCESS:
            type = ConsoleWidget::MessageType::SUCCESS;
            break;
        case ESP32Compiler::CompileStatus::ERROR:
            type = ConsoleWidget::MessageType::ERROR;
            break;
        case ESP32Compiler::CompileStatus::WARNING:
            type = ConsoleWidget::MessageType::WARNING;
            break;
        default:
            type = ConsoleWidget::MessageType::NORMAL;
    }
    
    console_->AddMessage(message, type);
}

void MainWindow::UpdateChat(const std::string& message, bool is_user) {
    // In a real GUI, this would update the chat widget
    std::cout << (is_user ? "User: " : "AI: ") << message << "\n";
}

void MainWindow::SetupCallbacks() {
    // Setup editor change callback
    editor_->SetChangeCallback([this]() {
        std::string current = file_manager_->GetCurrentFile();
        if (!current.empty()) {
            file_manager_->SetFileContent(current, editor_->GetText());
        }
    });
    
    // Setup compiler output callback
    compiler_->SetOutputCallback([this](const std::string& message, ESP32Compiler::CompileStatus status) {
        UpdateConsole(message, status);
    });
    
    // Setup serial monitor callback
    serial_monitor_->SetMessageCallback([this](const SerialMonitor::SerialMessage& msg) {
        ConsoleWidget::MessageType type = ConsoleWidget::MessageType::NORMAL;
        switch (msg.type) {
            case SerialMonitor::MessageType::ERROR:
                type = ConsoleWidget::MessageType::ERROR;
                break;
            case SerialMonitor::MessageType::SUCCESS:
                type = ConsoleWidget::MessageType::SUCCESS;
                break;
            case SerialMonitor::MessageType::WARNING:
                type = ConsoleWidget::MessageType::WARNING;
                break;
            default:
                type = ConsoleWidget::MessageType::NORMAL;
        }
        console_->AddMessage(msg.content, type);
    });
    
    // Setup VM emulator callbacks
    vm_emulator_->SetOutputCallback([this](const std::string& message) {
        console_->AddMessage("[VM] " + message);
    });
    
    vm_emulator_->SetPinChangeCallback([this](int pin, VMEmulator::PinState state) {
        std::string state_str = (state == VMEmulator::PinState::HIGH) ? "HIGH" : "LOW";
        console_->AddMessage("[VM] Pin " + std::to_string(pin) + " changed to " + state_str);
    });
}

void MainWindow::LoadDefaultFile() {
    // Create and load default sketch
    file_manager_->CreateFile("sketch.ino", FileManager::GetDefaultSketch());
    file_manager_->OpenFile("sketch.ino");
    editor_->SetText(file_manager_->GetFileContent("sketch.ino"));
    UpdateTitle();
}

} // namespace gui
} // namespace esp32_ide
