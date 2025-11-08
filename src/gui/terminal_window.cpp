#include "gui/terminal_window.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>

namespace esp32_ide {
namespace gui {

// ANSI color codes
const char* TerminalWindow::COLOR_RESET = "\033[0m";
const char* TerminalWindow::COLOR_RED = "\033[31m";
const char* TerminalWindow::COLOR_GREEN = "\033[32m";
const char* TerminalWindow::COLOR_YELLOW = "\033[33m";
const char* TerminalWindow::COLOR_BLUE = "\033[34m";
const char* TerminalWindow::COLOR_MAGENTA = "\033[35m";
const char* TerminalWindow::COLOR_CYAN = "\033[36m";
const char* TerminalWindow::COLOR_WHITE = "\033[37m";
const char* TerminalWindow::COLOR_BOLD = "\033[1m";

TerminalWindow::TerminalWindow()
    : text_editor_(nullptr),
      file_manager_(nullptr),
      compiler_(nullptr),
      serial_monitor_(nullptr),
      syntax_highlighter_(nullptr),
      width_(80),
      height_(24),
      running_(false),
      current_view_(ViewMode::MAIN_MENU) {
}

TerminalWindow::~TerminalWindow() = default;

bool TerminalWindow::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    running_ = true;
    
    AddConsoleMessage("ESP32 Driver IDE - Terminal Version");
    AddConsoleMessage("Initialized successfully");
    
    return true;
}

void TerminalWindow::Run() {
    while (running_) {
        ClearScreen();
        
        switch (current_view_) {
            case ViewMode::MAIN_MENU:
                RenderMainMenu();
                break;
            case ViewMode::FILE_BROWSER:
                RenderFileBrowser();
                break;
            case ViewMode::EDITOR:
                RenderEditor();
                break;
            case ViewMode::CONSOLE:
                RenderConsole();
                break;
            case ViewMode::SERIAL_MONITOR:
                RenderSerialMonitor();
                break;
            case ViewMode::SETTINGS:
                RenderSettings();
                break;
        }
    }
}

void TerminalWindow::Shutdown() {
    running_ = false;
    std::cout << COLOR_GREEN << "\nESP32 Driver IDE closed successfully.\n" << COLOR_RESET;
}

void TerminalWindow::SetTextEditor(TextEditor* editor) {
    text_editor_ = editor;
}

void TerminalWindow::SetFileManager(FileManager* file_manager) {
    file_manager_ = file_manager;
}

void TerminalWindow::SetCompiler(ESP32Compiler* compiler) {
    compiler_ = compiler;
}

void TerminalWindow::SetSerialMonitor(SerialMonitor* serial_monitor) {
    serial_monitor_ = serial_monitor;
}

void TerminalWindow::SetSyntaxHighlighter(SyntaxHighlighter* highlighter) {
    syntax_highlighter_ = highlighter;
}

void TerminalWindow::ClearScreen() {
    std::cout << "\033[2J\033[H";
}

void TerminalWindow::PrintHeader(const std::string& title) {
    std::cout << COLOR_BOLD << COLOR_CYAN;
    PrintSeparator();
    std::cout << "  " << title << "\n";
    PrintSeparator();
    std::cout << COLOR_RESET;
}

void TerminalWindow::PrintSeparator() {
    std::cout << std::string(width_, '=') << "\n";
}

void TerminalWindow::AddConsoleMessage(const std::string& message) {
    console_messages_.push_back(message);
    if (console_messages_.size() > 100) {
        console_messages_.erase(console_messages_.begin());
    }
}

std::string TerminalWindow::GetUserInput(const std::string& prompt) {
    std::cout << COLOR_YELLOW << prompt << COLOR_RESET;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int TerminalWindow::GetMenuChoice(int max_option) {
    int choice = -1;
    while (true) {
        std::cout << COLOR_YELLOW << "\nEnter choice (1-" << max_option << "): " << COLOR_RESET;
        if (std::cin >> choice && choice >= 1 && choice <= max_option) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << COLOR_RED << "Invalid input. Please try again.\n" << COLOR_RESET;
    }
}

void TerminalWindow::RenderMainMenu() {
    PrintHeader("ESP32 DRIVER IDE - MAIN MENU");
    
    std::cout << "\n";
    std::cout << COLOR_GREEN << "  1. " << COLOR_RESET << "New File\n";
    std::cout << COLOR_GREEN << "  2. " << COLOR_RESET << "Open File\n";
    std::cout << COLOR_GREEN << "  3. " << COLOR_RESET << "File Browser\n";
    std::cout << COLOR_GREEN << "  4. " << COLOR_RESET << "Edit Current File\n";
    std::cout << COLOR_GREEN << "  5. " << COLOR_RESET << "Compile Code\n";
    std::cout << COLOR_GREEN << "  6. " << COLOR_RESET << "Upload to ESP32\n";
    std::cout << COLOR_GREEN << "  7. " << COLOR_RESET << "Serial Monitor\n";
    std::cout << COLOR_GREEN << "  8. " << COLOR_RESET << "View Console\n";
    std::cout << COLOR_GREEN << "  9. " << COLOR_RESET << "Settings\n";
    std::cout << COLOR_GREEN << " 10. " << COLOR_RESET << "Exit\n";
    
    std::cout << "\n";
    if (!current_file_.empty()) {
        std::cout << COLOR_CYAN << "Current file: " << current_file_ << COLOR_RESET << "\n";
    }
    
    int choice = GetMenuChoice(10);
    
    switch (choice) {
        case 1:
            NewFile();
            break;
        case 2:
            OpenFile();
            break;
        case 3:
            current_view_ = ViewMode::FILE_BROWSER;
            break;
        case 4:
            if (!current_file_.empty()) {
                current_view_ = ViewMode::EDITOR;
            } else {
                AddConsoleMessage("No file is currently open");
                std::cout << COLOR_RED << "No file is currently open. Press Enter to continue..." << COLOR_RESET;
                std::cin.get();
            }
            break;
        case 5:
            CompileCode();
            break;
        case 6:
            UploadCode();
            break;
        case 7:
            current_view_ = ViewMode::SERIAL_MONITOR;
            break;
        case 8:
            current_view_ = ViewMode::CONSOLE;
            break;
        case 9:
            current_view_ = ViewMode::SETTINGS;
            break;
        case 10:
            Shutdown();
            break;
    }
}

void TerminalWindow::RenderFileBrowser() {
    PrintHeader("FILE BROWSER");
    
    if (!file_manager_) {
        std::cout << COLOR_RED << "File manager not available\n" << COLOR_RESET;
        std::cout << "Press Enter to return...";
        std::cin.get();
        current_view_ = ViewMode::MAIN_MENU;
        return;
    }
    
    auto files = file_manager_->GetFileList();
    
    std::cout << "\n" << COLOR_BOLD << "Available Files:\n" << COLOR_RESET;
    if (files.empty()) {
        std::cout << COLOR_YELLOW << "  No files found\n" << COLOR_RESET;
    } else {
        for (size_t i = 0; i < files.size(); ++i) {
            std::cout << COLOR_GREEN << "  " << (i + 1) << ". " << COLOR_RESET << files[i] << "\n";
        }
    }
    
    std::cout << "\n";
    std::cout << COLOR_GREEN << "  0. " << COLOR_RESET << "Back to Main Menu\n";
    
    if (!files.empty()) {
        std::cout << "\n" << COLOR_YELLOW << "Enter file number to open (0 to go back): " << COLOR_RESET;
        int choice;
        if (std::cin >> choice) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (choice == 0) {
                current_view_ = ViewMode::MAIN_MENU;
            } else if (choice > 0 && choice <= static_cast<int>(files.size())) {
                current_file_ = files[choice - 1];
                if (file_manager_->OpenFile(current_file_)) {
                    if (text_editor_) {
                        text_editor_->SetText(file_manager_->GetFileContent(current_file_));
                    }
                    AddConsoleMessage("Opened file: " + current_file_);
                    current_view_ = ViewMode::EDITOR;
                } else {
                    AddConsoleMessage("Failed to open file: " + current_file_);
                    std::cout << COLOR_RED << "Failed to open file. Press Enter to continue..." << COLOR_RESET;
                    std::cin.get();
                }
            }
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } else {
        std::cout << "Press Enter to return...";
        std::cin.get();
        current_view_ = ViewMode::MAIN_MENU;
    }
}

void TerminalWindow::RenderEditor() {
    PrintHeader("EDITOR: " + current_file_);
    
    if (!text_editor_) {
        std::cout << COLOR_RED << "Text editor not available\n" << COLOR_RESET;
        std::cout << "Press Enter to return...";
        std::cin.get();
        current_view_ = ViewMode::MAIN_MENU;
        return;
    }
    
    std::string content = text_editor_->GetText();
    
    std::cout << "\n" << COLOR_BOLD << "Content:\n" << COLOR_RESET;
    std::cout << COLOR_WHITE << "----------------------------------------\n";
    
    // Display content with line numbers
    std::istringstream iss(content);
    std::string line;
    int line_num = 1;
    while (std::getline(iss, line)) {
        std::cout << COLOR_CYAN << std::setw(4) << line_num << " | " << COLOR_RESET << line << "\n";
        line_num++;
        if (line_num > height_ - 15) {  // Leave room for menu
            std::cout << COLOR_YELLOW << "  ... (truncated)\n" << COLOR_RESET;
            break;
        }
    }
    std::cout << COLOR_WHITE << "----------------------------------------\n" << COLOR_RESET;
    
    std::cout << "\n";
    std::cout << COLOR_GREEN << "  1. " << COLOR_RESET << "Append Text\n";
    std::cout << COLOR_GREEN << "  2. " << COLOR_RESET << "Replace All Text\n";
    std::cout << COLOR_GREEN << "  3. " << COLOR_RESET << "Save File\n";
    std::cout << COLOR_GREEN << "  4. " << COLOR_RESET << "Close File\n";
    std::cout << COLOR_GREEN << "  5. " << COLOR_RESET << "Back to Main Menu\n";
    
    int choice = GetMenuChoice(5);
    
    switch (choice) {
        case 1: {
            std::cout << COLOR_YELLOW << "Enter text to append (end with CTRL+D or empty line):\n" << COLOR_RESET;
            std::string append_text;
            std::string input_line;
            while (std::getline(std::cin, input_line)) {
                if (input_line.empty()) break;
                append_text += input_line + "\n";
            }
            if (!append_text.empty()) {
                text_editor_->InsertText(append_text, text_editor_->GetText().length());
                AddConsoleMessage("Text appended");
            }
            break;
        }
        case 2: {
            std::cout << COLOR_YELLOW << "Enter new text (end with CTRL+D or empty line):\n" << COLOR_RESET;
            std::string new_text;
            std::string input_line;
            while (std::getline(std::cin, input_line)) {
                if (input_line.empty()) break;
                new_text += input_line + "\n";
            }
            text_editor_->SetText(new_text);
            AddConsoleMessage("Text replaced");
            break;
        }
        case 3:
            SaveFile();
            break;
        case 4:
            CloseFile();
            break;
        case 5:
            current_view_ = ViewMode::MAIN_MENU;
            break;
    }
}

void TerminalWindow::RenderConsole() {
    PrintHeader("CONSOLE OUTPUT");
    
    std::cout << "\n";
    if (console_messages_.empty()) {
        std::cout << COLOR_YELLOW << "No messages\n" << COLOR_RESET;
    } else {
        size_t start = console_messages_.size() > 20 ? console_messages_.size() - 20 : 0;
        for (size_t i = start; i < console_messages_.size(); ++i) {
            std::cout << COLOR_WHITE << "  " << console_messages_[i] << "\n" << COLOR_RESET;
        }
    }
    
    std::cout << "\nPress Enter to return to main menu...";
    std::cin.get();
    current_view_ = ViewMode::MAIN_MENU;
}

void TerminalWindow::RenderSerialMonitor() {
    PrintHeader("SERIAL MONITOR");
    
    if (!serial_monitor_) {
        std::cout << COLOR_RED << "Serial monitor not available\n" << COLOR_RESET;
        std::cout << "Press Enter to return...";
        std::cin.get();
        current_view_ = ViewMode::MAIN_MENU;
        return;
    }
    
    std::cout << "\n";
    std::cout << COLOR_GREEN << "  1. " << COLOR_RESET << "Connect to Serial Port\n";
    std::cout << COLOR_GREEN << "  2. " << COLOR_RESET << "Disconnect\n";
    std::cout << COLOR_GREEN << "  3. " << COLOR_RESET << "Send Data\n";
    std::cout << COLOR_GREEN << "  4. " << COLOR_RESET << "View Messages\n";
    std::cout << COLOR_GREEN << "  5. " << COLOR_RESET << "Back to Main Menu\n";
    
    int choice = GetMenuChoice(5);
    
    switch (choice) {
        case 1:
            ConnectSerial();
            break;
        case 2:
            DisconnectSerial();
            break;
        case 3:
            SendSerialData();
            break;
        case 4: {
            auto messages = serial_monitor_->GetMessages();
            std::cout << "\n" << COLOR_BOLD << "Serial Messages:\n" << COLOR_RESET;
            if (messages.empty()) {
                std::cout << COLOR_YELLOW << "No messages\n" << COLOR_RESET;
            } else {
                for (const auto& msg : messages) {
                    std::cout << COLOR_WHITE << "  " << msg.content << "\n" << COLOR_RESET;
                }
            }
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            break;
        }
        case 5:
            current_view_ = ViewMode::MAIN_MENU;
            break;
    }
}

void TerminalWindow::RenderSettings() {
    PrintHeader("SETTINGS");
    
    std::cout << "\n";
    std::cout << COLOR_CYAN << "Terminal Width: " << COLOR_RESET << width_ << "\n";
    std::cout << COLOR_CYAN << "Terminal Height: " << COLOR_RESET << height_ << "\n";
    std::cout << COLOR_CYAN << "Current File: " << COLOR_RESET << (current_file_.empty() ? "None" : current_file_) << "\n";
    
    std::cout << "\nPress Enter to return to main menu...";
    std::cin.get();
    current_view_ = ViewMode::MAIN_MENU;
}

void TerminalWindow::RenderStatusBar() {
    std::cout << COLOR_CYAN;
    PrintSeparator();
    std::cout << COLOR_RESET;
}

void TerminalWindow::NewFile() {
    std::string filename = GetUserInput("Enter filename: ");
    if (!filename.empty() && file_manager_) {
        if (file_manager_->CreateFile(filename)) {
            current_file_ = filename;
            if (text_editor_) {
                text_editor_->SetText("");
            }
            AddConsoleMessage("Created file: " + filename);
            std::cout << COLOR_GREEN << "File created successfully!\n" << COLOR_RESET;
        } else {
            AddConsoleMessage("Failed to create file: " + filename);
            std::cout << COLOR_RED << "Failed to create file.\n" << COLOR_RESET;
        }
    }
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

void TerminalWindow::OpenFile() {
    current_view_ = ViewMode::FILE_BROWSER;
}

void TerminalWindow::SaveFile() {
    if (current_file_.empty()) {
        AddConsoleMessage("No file is currently open");
        std::cout << COLOR_RED << "No file is currently open.\n" << COLOR_RESET;
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    if (file_manager_ && text_editor_) {
        file_manager_->SetFileContent(current_file_, text_editor_->GetText());
        if (file_manager_->SaveFile(current_file_)) {
            AddConsoleMessage("Saved file: " + current_file_);
            std::cout << COLOR_GREEN << "File saved successfully!\n" << COLOR_RESET;
        } else {
            AddConsoleMessage("Failed to save file: " + current_file_);
            std::cout << COLOR_RED << "Failed to save file.\n" << COLOR_RESET;
        }
    }
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

void TerminalWindow::CloseFile() {
    current_file_.clear();
    if (text_editor_) {
        text_editor_->SetText("");
    }
    AddConsoleMessage("File closed");
    current_view_ = ViewMode::MAIN_MENU;
}

void TerminalWindow::CompileCode() {
    if (!compiler_ || !text_editor_) {
        AddConsoleMessage("Compiler or editor not available");
        std::cout << COLOR_RED << "Compiler or editor not available.\n" << COLOR_RESET;
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    std::cout << COLOR_CYAN << "\nCompiling code...\n" << COLOR_RESET;
    
    auto result = compiler_->Compile(text_editor_->GetText(), compiler_->GetBoard());
    
    if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
        AddConsoleMessage("Compilation successful");
        std::cout << COLOR_GREEN << "✓ Compilation successful!\n" << COLOR_RESET;
        std::cout << COLOR_CYAN << result.message << COLOR_RESET << "\n";
    } else {
        AddConsoleMessage("Compilation failed");
        std::cout << COLOR_RED << "✗ Compilation failed:\n" << COLOR_RESET;
        std::cout << COLOR_YELLOW << result.message << COLOR_RESET << "\n";
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void TerminalWindow::UploadCode() {
    if (!compiler_) {
        AddConsoleMessage("Compiler not available");
        std::cout << COLOR_RED << "Compiler not available.\n" << COLOR_RESET;
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    std::cout << COLOR_CYAN << "\nUploading to ESP32...\n" << COLOR_RESET;
    
    if (compiler_->Upload(compiler_->GetBoard())) {
        AddConsoleMessage("Upload successful");
        std::cout << COLOR_GREEN << "✓ Upload successful!\n" << COLOR_RESET;
    } else {
        AddConsoleMessage("Upload failed");
        std::cout << COLOR_RED << "✗ Upload failed.\n" << COLOR_RESET;
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void TerminalWindow::ConnectSerial() {
    if (!serial_monitor_) {
        std::cout << COLOR_RED << "Serial monitor not available.\n" << COLOR_RESET;
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    std::string port = GetUserInput("Enter serial port (e.g., /dev/ttyUSB0): ");
    int baud = 115200;
    
    std::cout << COLOR_YELLOW << "Enter baud rate (default 115200): " << COLOR_RESET;
    std::string baud_str;
    std::getline(std::cin, baud_str);
    if (!baud_str.empty()) {
        try {
            baud = std::stoi(baud_str);
        } catch (...) {
            baud = 115200;
        }
    }
    
    if (serial_monitor_->Connect(port, baud)) {
        AddConsoleMessage("Connected to " + port + " at " + std::to_string(baud) + " baud");
        std::cout << COLOR_GREEN << "✓ Connected successfully!\n" << COLOR_RESET;
    } else {
        AddConsoleMessage("Failed to connect to " + port);
        std::cout << COLOR_RED << "✗ Connection failed.\n" << COLOR_RESET;
    }
    
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

void TerminalWindow::DisconnectSerial() {
    if (serial_monitor_) {
        serial_monitor_->Disconnect();
        AddConsoleMessage("Disconnected from serial port");
        std::cout << COLOR_GREEN << "Disconnected.\n" << COLOR_RESET;
    }
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

void TerminalWindow::SendSerialData() {
    if (!serial_monitor_) {
        std::cout << COLOR_RED << "Serial monitor not available.\n" << COLOR_RESET;
        std::cout << "Press Enter to continue...";
        std::cin.get();
        return;
    }
    
    std::string data = GetUserInput("Enter data to send: ");
    if (!data.empty()) {
        serial_monitor_->SendData(data);
        AddConsoleMessage("Sent: " + data);
        std::cout << COLOR_GREEN << "Data sent.\n" << COLOR_RESET;
    }
    std::cout << "Press Enter to continue...";
    std::cin.get();
}

} // namespace gui
} // namespace esp32_ide
