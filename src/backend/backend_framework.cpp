#include "backend/backend_framework.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "ai_assistant/ai_assistant.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include "emulator/vm_emulator.h"
#include "gui/device_library.h"
#include "gui/integrated_terminal.h"
#include "gui/console_widget.h"
#include "blueprint/blueprint_editor.h"
#include "utils/ml_device_detector.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace esp32_ide {

// =============================================================================
// BackendFramework Implementation
// =============================================================================

BackendFramework& BackendFramework::GetInstance() {
    static BackendFramework instance;
    return instance;
}

BackendFramework::BackendFramework()
    : initialized_(false),
      is_compiling_(false),
      is_uploading_(false),
      status_message_("Ready") {
}

BackendFramework::~BackendFramework() {
    Shutdown();
}

bool BackendFramework::Initialize() {
    if (initialized_) {
        return true;
    }
    
    try {
        // Initialize all components
        text_editor_ = std::make_unique<TextEditor>();
        syntax_highlighter_ = std::make_unique<SyntaxHighlighter>();
        file_manager_ = std::make_unique<FileManager>();
        ai_assistant_ = std::make_unique<AIAssistant>();
        compiler_ = std::make_unique<ESP32Compiler>();
        serial_monitor_ = std::make_unique<SerialMonitor>();
        vm_emulator_ = std::make_unique<VMEmulator>();
        device_library_ = std::make_unique<gui::DeviceLibrary>();
        terminal_ = std::make_unique<gui::IntegratedTerminal>();
        console_ = std::make_unique<gui::ConsoleWidget>();
        blueprint_editor_ = std::make_unique<blueprint::BlueprintEditor>();
        device_detector_ = std::make_unique<ml::MLDeviceDetector>();
        
        // Initialize device library
        device_library_->Initialize();
        
        // Initialize terminal
        terminal_->Initialize();
        
        // Initialize default board configuration
        InitializeDefaultBoard();
        
        // Load preferences and recent files
        LoadPreferences();
        LoadRecentFiles();
        
        // Create default sketch
        file_manager_->CreateFile("sketch.ino", FileManager::GetDefaultSketch());
        current_file_ = "sketch.ino";
        text_editor_->SetText(FileManager::GetDefaultSketch());
        
        initialized_ = true;
        SetStatusMessage("ESP32 Driver IDE initialized");
        
        EmitEvent({EventType::STATUS_MESSAGE, "system", "IDE initialized", {}});
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize BackendFramework: " << e.what() << std::endl;
        return false;
    }
}

void BackendFramework::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Save preferences
    SavePreferences();
    SaveRecentFiles();
    
    // Cleanup components
    device_detector_.reset();
    blueprint_editor_.reset();
    console_.reset();
    terminal_.reset();
    device_library_.reset();
    vm_emulator_.reset();
    serial_monitor_.reset();
    compiler_.reset();
    ai_assistant_.reset();
    file_manager_.reset();
    syntax_highlighter_.reset();
    text_editor_.reset();
    
    initialized_ = false;
}

void BackendFramework::InitializeDefaultBoard() {
    current_board_.name = "ESP32 Dev Module";
    current_board_.fqbn = "esp32:esp32:esp32";
    current_board_.port = "/dev/ttyUSB0";
    current_board_.baudRate = 115200;
    current_board_.programmer = "";
    current_board_.options["flash_size"] = "4MB";
    current_board_.options["upload_speed"] = "921600";
}

// Event system
void BackendFramework::AddEventHandler(EventType type, EventHandler handler) {
    event_handlers_[type].push_back(handler);
}

void BackendFramework::RemoveEventHandler(EventType type) {
    event_handlers_.erase(type);
}

void BackendFramework::EmitEvent(const Event& event) {
    auto it = event_handlers_.find(event.type);
    if (it != event_handlers_.end()) {
        for (const auto& handler : it->second) {
            handler(event);
        }
    }
}

// File operations
bool BackendFramework::NewFile(const std::string& filename) {
    std::string name = filename.empty() ? "untitled.ino" : filename;
    
    file_manager_->CreateFile(name, FileManager::GetDefaultSketch());
    current_file_ = name;
    text_editor_->SetText(FileManager::GetDefaultSketch());
    
    EmitEvent({EventType::FILE_NEW, "file_manager", name, {}});
    SetStatusMessage("New file: " + name);
    
    return true;
}

bool BackendFramework::OpenFile(const std::string& filename) {
    try {
        if (!file_manager_->FileExists(filename)) {
            EmitEvent({EventType::ERROR_MESSAGE, "file_manager", "File not found: " + filename, {}});
            return false;
        }
        
        std::string content = file_manager_->GetFileContent(filename);
        current_file_ = filename;
        text_editor_->SetText(content);
        
        AddToRecentFiles(filename);
        
        EmitEvent({EventType::FILE_OPENED, "file_manager", filename, {}});
        SetStatusMessage("Opened: " + filename);
        
        return true;
    } catch (const std::exception& e) {
        EmitEvent({EventType::ERROR_MESSAGE, "file_manager", e.what(), {}});
        return false;
    }
}

bool BackendFramework::SaveFile() {
    if (current_file_.empty()) {
        return false;
    }
    
    file_manager_->SetFileContent(current_file_, text_editor_->GetText());
    file_manager_->SaveFile(current_file_);
    
    EmitEvent({EventType::FILE_SAVED, "file_manager", current_file_, {}});
    SetStatusMessage("Saved: " + current_file_);
    
    return true;
}

bool BackendFramework::SaveFileAs(const std::string& filename) {
    file_manager_->CreateFile(filename, text_editor_->GetText());
    file_manager_->SaveFile(filename);
    current_file_ = filename;
    
    AddToRecentFiles(filename);
    
    EmitEvent({EventType::FILE_SAVED, "file_manager", filename, {}});
    SetStatusMessage("Saved as: " + filename);
    
    return true;
}

bool BackendFramework::CloseFile() {
    if (current_file_.empty()) {
        return false;
    }
    
    EmitEvent({EventType::FILE_CLOSED, "file_manager", current_file_, {}});
    current_file_.clear();
    text_editor_->SetText("");
    
    return true;
}

std::vector<std::string> BackendFramework::GetRecentFiles() const {
    return recent_files_;
}

void BackendFramework::AddToRecentFiles(const std::string& filename) {
    // Remove if already exists
    recent_files_.erase(
        std::remove(recent_files_.begin(), recent_files_.end(), filename),
        recent_files_.end()
    );
    
    // Add to front
    recent_files_.insert(recent_files_.begin(), filename);
    
    // Keep only last 10
    if (recent_files_.size() > 10) {
        recent_files_.resize(10);
    }
}

void BackendFramework::LoadRecentFiles() {
    // Would load from preferences file
    recent_files_.clear();
}

void BackendFramework::SaveRecentFiles() {
    // Would save to preferences file
}

// Board operations
void BackendFramework::SetBoard(const BoardConfig& config) {
    current_board_ = config;
    
    // Update compiler settings
    switch (config.name[0]) {
        case 'E':  // ESP32 variants
            if (config.name.find("S2") != std::string::npos) {
                compiler_->SetBoard(ESP32Compiler::BoardType::ESP32_S2);
            } else if (config.name.find("S3") != std::string::npos) {
                compiler_->SetBoard(ESP32Compiler::BoardType::ESP32_S3);
            } else if (config.name.find("C3") != std::string::npos) {
                compiler_->SetBoard(ESP32Compiler::BoardType::ESP32_C3);
            } else {
                compiler_->SetBoard(ESP32Compiler::BoardType::ESP32);
            }
            break;
        default:
            compiler_->SetBoard(ESP32Compiler::BoardType::ESP32);
    }
    
    EmitEvent({EventType::BOARD_CHANGED, "board", config.name, {{"port", config.port}}});
    SetStatusMessage("Board: " + config.name + " on " + config.port);
}

std::vector<BackendFramework::BoardConfig> BackendFramework::GetAvailableBoards() const {
    std::vector<BoardConfig> boards;
    
    // ESP32 variants
    boards.push_back({"ESP32 Dev Module", "esp32:esp32:esp32", "", 115200, "", {}});
    boards.push_back({"ESP32-S2 Dev Module", "esp32:esp32:esp32s2", "", 115200, "", {}});
    boards.push_back({"ESP32-S3 Dev Module", "esp32:esp32:esp32s3", "", 115200, "", {}});
    boards.push_back({"ESP32-C3 Dev Module", "esp32:esp32:esp32c3", "", 115200, "", {}});
    boards.push_back({"ESP32-C6 Dev Module", "esp32:esp32:esp32c6", "", 115200, "", {}});
    boards.push_back({"ESP32-H2 Dev Module", "esp32:esp32:esp32h2", "", 115200, "", {}});
    
    return boards;
}

std::vector<std::string> BackendFramework::GetAvailablePorts() const {
    std::vector<std::string> ports;
    
    // Common Linux ports
    ports.push_back("/dev/ttyUSB0");
    ports.push_back("/dev/ttyUSB1");
    ports.push_back("/dev/ttyACM0");
    ports.push_back("/dev/ttyACM1");
    
    // Common Windows ports would be COM1, COM2, etc.
    // Common macOS ports would be /dev/cu.usbserial-*, etc.
    
    return ports;
}

void BackendFramework::RefreshPorts() {
    // Would scan for available serial ports
    EmitEvent({EventType::STATUS_MESSAGE, "serial", "Ports refreshed", {}});
}

// Compile operations
bool BackendFramework::Verify() {
    if (is_compiling_) {
        return false;
    }
    
    is_compiling_ = true;
    EmitEvent({EventType::COMPILE_STARTED, "compiler", "Verification started", {}});
    SetStatusMessage("Compiling...");
    
    auto result = compiler_->Compile(text_editor_->GetText(), 
                                     compiler_->GetBoard());
    
    is_compiling_ = false;
    
    if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
        EmitEvent({EventType::COMPILE_SUCCESS, "compiler", "Compilation successful", {}});
        SetStatusMessage("Compilation successful");
        return true;
    } else {
        std::string errors;
        for (const auto& error : result.errors) {
            errors += error + "\n";
        }
        EmitEvent({EventType::COMPILE_ERROR, "compiler", errors, {}});
        SetStatusMessage("Compilation failed");
        return false;
    }
}

bool BackendFramework::Upload() {
    if (is_compiling_ || is_uploading_) {
        return false;
    }
    
    // First verify
    if (!Verify()) {
        return false;
    }
    
    is_uploading_ = true;
    EmitEvent({EventType::UPLOAD_STARTED, "compiler", "Upload started", {}});
    SetStatusMessage("Uploading to " + current_board_.port + "...");
    
    bool success = compiler_->Upload(compiler_->GetBoard());
    
    is_uploading_ = false;
    
    if (success) {
        EmitEvent({EventType::UPLOAD_SUCCESS, "compiler", "Upload successful", {}});
        SetStatusMessage("Upload complete");
        return true;
    } else {
        EmitEvent({EventType::UPLOAD_ERROR, "compiler", "Upload failed", {}});
        SetStatusMessage("Upload failed");
        return false;
    }
}

bool BackendFramework::UploadWithProgrammer() {
    // Similar to Upload but uses external programmer
    return Upload();
}

void BackendFramework::StopCompile() {
    is_compiling_ = false;
    is_uploading_ = false;
    SetStatusMessage("Compile stopped");
}

// Serial operations
bool BackendFramework::OpenSerialMonitor() {
    bool success = serial_monitor_->Connect(current_board_.port, current_board_.baudRate);
    
    if (success) {
        EmitEvent({EventType::SERIAL_CONNECTED, "serial", current_board_.port, {}});
        SetStatusMessage("Serial connected: " + current_board_.port);
    }
    
    return success;
}

void BackendFramework::CloseSerialMonitor() {
    serial_monitor_->Disconnect();
    EmitEvent({EventType::SERIAL_DISCONNECTED, "serial", "", {}});
    SetStatusMessage("Serial disconnected");
}

void BackendFramework::SetSerialBaudRate(int baud) {
    current_board_.baudRate = baud;
    // Reconnect if already open
    if (serial_monitor_->IsConnected()) {
        serial_monitor_->Disconnect();
        serial_monitor_->Connect(current_board_.port, baud);
    }
}

void BackendFramework::SendSerialData(const std::string& data) {
    serial_monitor_->SendData(data);
    EmitEvent({EventType::SERIAL_DATA_SENT, "serial", data, {}});
}

bool BackendFramework::IsSerialOpen() const {
    return serial_monitor_ && serial_monitor_->IsConnected();
}

// Emulator operations
bool BackendFramework::StartEmulator() {
    vm_emulator_->Start();
    EmitEvent({EventType::EMULATOR_STARTED, "emulator", "Emulator started", {}});
    SetStatusMessage("Emulator running");
    return true;
}

void BackendFramework::StopEmulator() {
    vm_emulator_->Reset();
    EmitEvent({EventType::EMULATOR_STOPPED, "emulator", "Emulator stopped", {}});
    SetStatusMessage("Emulator stopped");
}

bool BackendFramework::IsEmulatorRunning() const {
    return vm_emulator_ && vm_emulator_->IsRunning();
}

// Project operations
bool BackendFramework::CreateProject(const std::string& name, const std::string& template_name) {
    project_.name = name;
    project_.mainFile = name + ".ino";
    
    // Create main file with template
    std::string content;
    if (!template_name.empty()) {
        auto templates = file_manager_->GetTemplates();
        for (const auto& t : templates) {
            if (t.name == template_name) {
                content = t.code;
                break;
            }
        }
    }
    
    if (content.empty()) {
        content = FileManager::GetDefaultSketch();
    }
    
    file_manager_->CreateFile(project_.mainFile, content);
    current_file_ = project_.mainFile;
    text_editor_->SetText(content);
    
    SetStatusMessage("Created project: " + name);
    return true;
}

bool BackendFramework::OpenProject(const std::string& path) {
    project_.path = path;
    // Would scan directory for project files
    SetStatusMessage("Opened project: " + path);
    return true;
}

bool BackendFramework::SaveProject() {
    SaveFile();
    return true;
}

bool BackendFramework::CloseProject() {
    CloseFile();
    project_ = ProjectConfig();
    return true;
}

// AI operations
std::string BackendFramework::QueryAI(const std::string& query) {
    EmitEvent({EventType::AI_QUERY_STARTED, "ai", query, {}});
    
    std::string response = ai_assistant_->Query(query);
    
    EmitEvent({EventType::AI_RESPONSE_READY, "ai", response, {}});
    return response;
}

std::string BackendFramework::GenerateCode(const std::string& description) {
    std::string code = ai_assistant_->GenerateCode(description);
    EmitEvent({EventType::AI_CODE_GENERATED, "ai", code, {}});
    return code;
}

std::string BackendFramework::AnalyzeCode() {
    return ai_assistant_->AnalyzeCode(text_editor_->GetText());
}

std::string BackendFramework::FixBugs() {
    return ai_assistant_->AutoFixBugs(text_editor_->GetText());
}

// Preferences
void BackendFramework::SetPreference(const std::string& key, const std::string& value) {
    preferences_[key] = value;
}

std::string BackendFramework::GetPreference(const std::string& key, const std::string& default_value) const {
    auto it = preferences_.find(key);
    return (it != preferences_.end()) ? it->second : default_value;
}

bool BackendFramework::SavePreferences() {
    // Would save to file
    return true;
}

bool BackendFramework::LoadPreferences() {
    // Would load from file
    return true;
}

void BackendFramework::SetStatusMessage(const std::string& message) {
    status_message_ = message;
    EmitEvent({EventType::STATUS_MESSAGE, "system", message, {}});
}

// =============================================================================
// Arduino Actions Implementation
// =============================================================================

namespace ArduinoActions {

void New() {
    BackendFramework::GetInstance().NewFile();
}

void Open() {
    // Would show file dialog
    // For now, just log
    std::cout << "File > Open\n";
}

void OpenRecent(const std::string& filename) {
    BackendFramework::GetInstance().OpenFile(filename);
}

void Save() {
    BackendFramework::GetInstance().SaveFile();
}

void SaveAs() {
    // Would show save dialog
    std::cout << "File > Save As\n";
}

void Close() {
    BackendFramework::GetInstance().CloseFile();
}

void Quit() {
    BackendFramework::GetInstance().Shutdown();
}

void Undo() {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        editor->Undo();
    }
}

void Redo() {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        editor->Redo();
    }
}

void Cut() {
    // Would implement clipboard cut
    std::cout << "Edit > Cut\n";
}

void Copy() {
    // Would implement clipboard copy
    std::cout << "Edit > Copy\n";
}

void Paste() {
    // Would implement clipboard paste
    std::cout << "Edit > Paste\n";
}

void SelectAll() {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        // Would select all text
    }
}

void Find() {
    // Would show find dialog
    std::cout << "Edit > Find\n";
}

void FindAndReplace() {
    // Would show find and replace dialog
    std::cout << "Edit > Find and Replace\n";
}

void GoToLine(int line) {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        // Calculate position from line number (simplified - assumes lines are separated by newlines)
        std::string text = editor->GetText();
        size_t pos = 0;
        int current_line = 1;
        while (current_line < line && pos < text.length()) {
            if (text[pos] == '\n') {
                current_line++;
            }
            pos++;
        }
        editor->SetCursorPosition(pos);
    }
}

void Verify() {
    BackendFramework::GetInstance().Verify();
}

void Upload() {
    BackendFramework::GetInstance().Upload();
}

void UploadWithProgrammer() {
    BackendFramework::GetInstance().UploadWithProgrammer();
}

void ExportCompiledBinary() {
    // Would export compiled binary
    std::cout << "Sketch > Export Compiled Binary\n";
}

void ShowSketchFolder() {
    // Would open file manager to sketch folder
    std::cout << "Sketch > Show Sketch Folder\n";
}

void IncludeLibrary(const std::string& library) {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        std::string include = "#include <" + library + ".h>\n";
        editor->InsertText(include, 0);
    }
}

void AddFile() {
    // Would show add file dialog
    std::cout << "Sketch > Add File\n";
}

void AutoFormat() {
    auto& framework = BackendFramework::GetInstance();
    auto* ai = framework.GetAIAssistant();
    auto* editor = framework.GetTextEditor();
    
    if (ai && editor) {
        std::string formatted = ai->RefactorCode(editor->GetText(), "readability");
        editor->SetText(formatted);
    }
}

void ArchiveSketch() {
    // Would create ZIP archive of sketch
    std::cout << "Tools > Archive Sketch\n";
}

void FixEncoding() {
    // Would fix file encoding issues
    std::cout << "Tools > Fix Encoding\n";
}

void SerialMonitor() {
    BackendFramework::GetInstance().OpenSerialMonitor();
}

void SerialPlotter() {
    // Would open serial plotter
    std::cout << "Tools > Serial Plotter\n";
}

void ManageLibraries() {
    // Would open library manager
    std::cout << "Tools > Manage Libraries\n";
}

void BoardManager() {
    // Would open board manager
    std::cout << "Tools > Board Manager\n";
}

void GetBoardInfo() {
    auto board = BackendFramework::GetInstance().GetBoard();
    std::cout << "Board Info:\n";
    std::cout << "  Name: " << board.name << "\n";
    std::cout << "  FQBN: " << board.fqbn << "\n";
    std::cout << "  Port: " << board.port << "\n";
}

void GettingStarted() {
    std::cout << "Help > Getting Started\n";
    std::cout << "Visit: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/\n";
}

void Reference() {
    std::cout << "Help > Reference\n";
    std::cout << "Visit: https://www.arduino.cc/reference/en/\n";
}

void FindInReference() {
    // Would search reference for selected text
    std::cout << "Help > Find in Reference\n";
}

void About() {
    std::cout << "ESP32 Driver IDE v2.0.0\n";
    std::cout << "A modern C++ IDE for ESP32 development\n";
    std::cout << "Inspired by Arduino IDE\n";
}

} // namespace ArduinoActions

} // namespace esp32_ide
