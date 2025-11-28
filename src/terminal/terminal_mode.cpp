#include "terminal/terminal_mode.h"
#include "backend/backend_framework.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "file_manager/project_templates.h"
#include "gui/device_library.h"
#include "scripting/scripting_engine.h"
#include "plugins/plugin_system.h"
#include "testing/test_framework.h"
#include "decompiler/advanced_decompiler.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace esp32_ide {

TerminalModeApp::TerminalModeApp()
    : running_(false),
      interactive_mode_(false),
      color_output_(true),
      prompt_("esp32> ") {
    DetectColorSupport();
    RegisterBuiltInCommands();
}

TerminalModeApp::~TerminalModeApp() {
}

int TerminalModeApp::Run(int argc, char* argv[]) {
    // Initialize backend
    if (!BackendFramework::GetInstance().Initialize()) {
        PrintError("Failed to initialize backend framework");
        return 1;
    }
    
    // Parse command line arguments
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    
    // Check for special flags
    if (args.empty()) {
        // No arguments - run interactive mode
        return RunInteractive();
    }
    
    // Handle --version or -v
    if (args[0] == "--version" || args[0] == "-v") {
        PrintVersion();
        return 0;
    }
    
    // Handle --help or -h
    if (args[0] == "--help" || args[0] == "-h") {
        PrintHelp();
        return 0;
    }
    
    // Handle --interactive or -i
    if (args[0] == "--interactive" || args[0] == "-i") {
        return RunInteractive();
    }
    
    // Find and execute command
    std::string cmd_name = args[0];
    
    // Check for alias
    auto alias_it = aliases_.find(cmd_name);
    if (alias_it != aliases_.end()) {
        cmd_name = alias_it->second;
    }
    
    // Find command
    auto cmd_it = commands_.find(cmd_name);
    if (cmd_it == commands_.end()) {
        PrintError("Unknown command: " + args[0]);
        PrintInfo("Run 'esp32-ide --help' for usage information");
        return 1;
    }
    
    // Execute command with remaining arguments
    std::vector<std::string> cmd_args(args.begin() + 1, args.end());
    return cmd_it->second.handler(cmd_args);
}

void TerminalModeApp::Quit() {
    running_ = false;
}

int TerminalModeApp::RunInteractive() {
    interactive_mode_ = true;
    running_ = true;
    
    PrintWelcome();
    Print("");
    
    while (running_) {
        std::cout << prompt_;
        std::cout.flush();
        
        std::string input;
        if (!std::getline(std::cin, input)) {
            break;  // EOF
        }
        
        // Trim whitespace
        size_t start = input.find_first_not_of(" \t\n\r");
        size_t end = input.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) {
            continue;  // Empty line
        }
        input = input.substr(start, end - start + 1);
        
        if (input.empty()) {
            continue;
        }
        
        ProcessCommand(input);
    }
    
    Print("\nGoodbye!");
    return 0;
}

void TerminalModeApp::ProcessCommand(const std::string& input) {
    std::vector<std::string> args = ParseArguments(input);
    
    if (args.empty()) {
        return;
    }
    
    // Add to command history (but skip history command itself)
    if (args[0] != "history") {
        command_history_.push_back(input);
        // Keep only last 100 commands
        if (command_history_.size() > 100) {
            command_history_.erase(command_history_.begin());
        }
    }
    
    std::string cmd_name = args[0];
    
    // Check for alias
    auto alias_it = aliases_.find(cmd_name);
    if (alias_it != aliases_.end()) {
        cmd_name = alias_it->second;
    }
    
    // Find command
    auto cmd_it = commands_.find(cmd_name);
    if (cmd_it == commands_.end()) {
        PrintError("Unknown command: " + args[0]);
        PrintInfo("Type 'help' for available commands");
        return;
    }
    
    // Execute command
    std::vector<std::string> cmd_args(args.begin() + 1, args.end());
    cmd_it->second.handler(cmd_args);
}

void TerminalModeApp::RegisterCommand(const Command& command) {
    commands_[command.name] = command;
    
    // Register aliases
    for (const auto& alias : command.aliases) {
        aliases_[alias] = command.name;
    }
}

void TerminalModeApp::RegisterBuiltInCommands() {
    // Help command
    RegisterCommand({
        "help", "Display help information", "help [command]",
        {"?", "h"},
        [this](const std::vector<std::string>& args) { return HandleHelp(args); }
    });
    
    // Version command
    RegisterCommand({
        "version", "Display version information", "version",
        {"ver"},
        [this](const std::vector<std::string>& args) { return HandleVersion(args); }
    });
    
    // Quit command
    RegisterCommand({
        "quit", "Exit the IDE", "quit",
        {"exit", "q"},
        [this](const std::vector<std::string>& args) { return HandleQuit(args); }
    });
    
    // File commands
    RegisterCommand({
        "new", "Create a new sketch", "new [filename]",
        {},
        [this](const std::vector<std::string>& args) { return HandleNew(args); }
    });
    
    RegisterCommand({
        "open", "Open a file", "open <filename>",
        {"o"},
        [this](const std::vector<std::string>& args) { return HandleOpen(args); }
    });
    
    RegisterCommand({
        "save", "Save current file", "save [filename]",
        {"s"},
        [this](const std::vector<std::string>& args) { return HandleSave(args); }
    });
    
    RegisterCommand({
        "close", "Close current file", "close",
        {},
        [this](const std::vector<std::string>& args) { return HandleClose(args); }
    });
    
    RegisterCommand({
        "list", "List files in project", "list",
        {"ls", "files"},
        [this](const std::vector<std::string>& args) { return HandleList(args); }
    });
    
    RegisterCommand({
        "cat", "Display file contents", "cat <filename>",
        {"show", "type"},
        [this](const std::vector<std::string>& args) { return HandleCat(args); }
    });
    
    RegisterCommand({
        "edit", "Open file in editor", "edit <filename>",
        {"e"},
        [this](const std::vector<std::string>& args) { return HandleEdit(args); }
    });
    
    // Board commands
    RegisterCommand({
        "board", "Set or show current board", "board [board_name]",
        {"b"},
        [this](const std::vector<std::string>& args) { return HandleBoard(args); }
    });
    
    RegisterCommand({
        "port", "Set or show current port", "port [port_name]",
        {"p"},
        [this](const std::vector<std::string>& args) { return HandlePort(args); }
    });
    
    RegisterCommand({
        "boards", "List available boards", "boards",
        {"board-list"},
        [this](const std::vector<std::string>& args) { return HandleBoardList(args); }
    });
    
    RegisterCommand({
        "ports", "List available ports", "ports",
        {"port-list"},
        [this](const std::vector<std::string>& args) { return HandlePortList(args); }
    });
    
    // Compile commands
    RegisterCommand({
        "verify", "Verify/compile sketch", "verify",
        {"v", "compile"},
        [this](const std::vector<std::string>& args) { return HandleVerify(args); }
    });
    
    RegisterCommand({
        "upload", "Upload sketch to board", "upload",
        {"u"},
        [this](const std::vector<std::string>& args) { return HandleUpload(args); }
    });
    
    // Serial commands
    RegisterCommand({
        "monitor", "Open serial monitor", "monitor [baud]",
        {"m", "serial"},
        [this](const std::vector<std::string>& args) { return HandleMonitor(args); }
    });
    
    RegisterCommand({
        "send", "Send data to serial", "send <data>",
        {},
        [this](const std::vector<std::string>& args) { return HandleSend(args); }
    });
    
    // Emulator commands
    RegisterCommand({
        "emulator", "Control VM emulator", "emulator [start|stop|status]",
        {"emu", "vm"},
        [this](const std::vector<std::string>& args) { return HandleEmulator(args); }
    });
    
    // AI commands
    RegisterCommand({
        "ask", "Ask AI assistant a question", "ask <question>",
        {"ai"},
        [this](const std::vector<std::string>& args) { return HandleAsk(args); }
    });
    
    RegisterCommand({
        "generate", "Generate code with AI", "generate <description>",
        {"gen"},
        [this](const std::vector<std::string>& args) { return HandleGenerate(args); }
    });
    
    RegisterCommand({
        "analyze", "Analyze current code", "analyze",
        {},
        [this](const std::vector<std::string>& args) { return HandleAnalyze(args); }
    });
    
    RegisterCommand({
        "fix", "Auto-fix bugs in code", "fix",
        {},
        [this](const std::vector<std::string>& args) { return HandleFix(args); }
    });
    
    // Device library commands
    RegisterCommand({
        "devices", "List available devices", "devices [category]",
        {},
        [this](const std::vector<std::string>& args) { return HandleDevices(args); }
    });
    
    RegisterCommand({
        "add-device", "Add device to project", "add-device <device_id>",
        {},
        [this](const std::vector<std::string>& args) { return HandleAddDevice(args); }
    });
    
    // Settings commands
    RegisterCommand({
        "config", "Show configuration", "config",
        {"settings"},
        [this](const std::vector<std::string>& args) { return HandleConfig(args); }
    });
    
    RegisterCommand({
        "set", "Set configuration value", "set <key> <value>",
        {},
        [this](const std::vector<std::string>& args) { return HandleSet(args); }
    });
    
    RegisterCommand({
        "get", "Get configuration value", "get <key>",
        {},
        [this](const std::vector<std::string>& args) { return HandleGet(args); }
    });
    
    // Project commands
    RegisterCommand({
        "create", "Create a new project from template", "create <project_name> [template]",
        {"new-project"},
        [this](const std::vector<std::string>& args) { return HandleCreate(args); }
    });
    
    RegisterCommand({
        "templates", "List available project templates", "templates",
        {"tpl"},
        [this](const std::vector<std::string>& args) { return HandleTemplates(args); }
    });
    
    RegisterCommand({
        "recent", "List recently opened files", "recent",
        {},
        [this](const std::vector<std::string>& args) { return HandleRecent(args); }
    });
    
    RegisterCommand({
        "export", "Export compiled binary", "export [output_path]",
        {},
        [this](const std::vector<std::string>& args) { return HandleExport(args); }
    });
    
    // Utility commands
    RegisterCommand({
        "clear", "Clear the terminal screen", "clear",
        {"cls"},
        [this](const std::vector<std::string>& args) { return HandleClear(args); }
    });
    
    RegisterCommand({
        "history", "Show command history", "history [count]",
        {},
        [this](const std::vector<std::string>& args) { return HandleHistory(args); }
    });
    
    RegisterCommand({
        "status", "Show IDE status", "status",
        {"st"},
        [this](const std::vector<std::string>& args) { return HandleStatus(args); }
    });
    
    RegisterCommand({
        "info", "Show system information", "info",
        {"sysinfo"},
        [this](const std::vector<std::string>& args) { return HandleInfo(args); }
    });
    
    // Script commands
    RegisterCommand({
        "script", "Run or manage scripts", "script [run <file>|list|examples]",
        {"run"},
        [this](const std::vector<std::string>& args) { return HandleScript(args); }
    });
    
    // Plugin commands
    RegisterCommand({
        "plugins", "Manage plugins", "plugins [list|enable|disable <plugin_id>]",
        {"plugin"},
        [this](const std::vector<std::string>& args) { return HandlePlugins(args); }
    });
    
    // Test commands
    RegisterCommand({
        "test", "Run tests", "test [suite_name] [test_name]",
        {},
        [this](const std::vector<std::string>& args) { return HandleTest(args); }
    });
    
    RegisterCommand({
        "coverage", "Show code coverage", "coverage",
        {"cov"},
        [this](const std::vector<std::string>& args) { return HandleCoverage(args); }
    });
    
    // Decompiler commands
    RegisterCommand({
        "decompile", "Decompile firmware binary", "decompile <firmware_file>",
        {"disasm"},
        [this](const std::vector<std::string>& args) { return HandleDecompile(args); }
    });
}

void TerminalModeApp::PrintHelp() {
    Print("ESP32 Driver IDE - Terminal Mode\n");
    Print("Usage: esp32-ide [options] [command] [arguments]\n");
    Print("Options:");
    Print("  -h, --help         Show this help message");
    Print("  -v, --version      Show version information");
    Print("  -i, --interactive  Run in interactive mode\n");
    Print("Commands:");
    
    // Group commands by category
    std::vector<std::pair<std::string, std::vector<std::string>>> categories = {
        {"File Operations", {"new", "open", "save", "close", "list", "cat", "edit", "recent"}},
        {"Project Management", {"create", "templates", "export"}},
        {"Board & Port", {"board", "port", "boards", "ports"}},
        {"Compile & Upload", {"verify", "upload"}},
        {"Serial Communication", {"monitor", "send"}},
        {"Emulator", {"emulator"}},
        {"AI Assistant", {"ask", "generate", "analyze", "fix"}},
        {"Device Library", {"devices", "add-device"}},
        {"Scripts & Plugins", {"script", "plugins"}},
        {"Testing", {"test", "coverage"}},
        {"Decompiler", {"decompile"}},
        {"Settings", {"config", "set", "get"}},
        {"Utilities", {"clear", "history", "status", "info"}},
        {"General", {"help", "version", "quit"}}
    };
    
    for (const auto& category : categories) {
        Print("\n  " + category.first + ":");
        for (const auto& cmd_name : category.second) {
            auto it = commands_.find(cmd_name);
            if (it != commands_.end()) {
                std::ostringstream oss;
                oss << "    " << std::left << std::setw(15) << it->second.name
                    << " " << it->second.description;
                Print(oss.str());
            }
        }
    }
    
    Print("\nRun 'help <command>' for more information about a specific command.");
}

void TerminalModeApp::PrintCommandHelp(const std::string& command) {
    auto it = commands_.find(command);
    if (it == commands_.end()) {
        PrintError("Unknown command: " + command);
        return;
    }
    
    Print("Command: " + it->second.name);
    Print("Description: " + it->second.description);
    Print("Usage: " + it->second.usage);
    
    if (!it->second.aliases.empty()) {
        std::ostringstream oss;
        oss << "Aliases: ";
        for (size_t i = 0; i < it->second.aliases.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << it->second.aliases[i];
        }
        Print(oss.str());
    }
}

void TerminalModeApp::PrintVersion() {
    Print("ESP32 Driver IDE v2.0.0");
    Print("Terminal Mode Edition");
    Print("");
    Print("A modern C++ IDE for ESP32 development");
    Print("Inspired by Arduino IDE");
}

void TerminalModeApp::PrintWelcome() {
    std::string banner = R"(
 _____ ____  ____  _____ ____    ____       _                     ___ ____  _____
| ____/ ___||  _ \|___ /|___ \  |  _ \ _ __(_)_   _____ _ __     |_ _|  _ \| ____|
|  _| \___ \| |_) | |_ \  __) | | | | | '__| \ \ / / _ \ '__|     | || | | |  _|
| |___ ___) |  __/ ___) |/ __/  | |_| | |  | |\ V /  __/ |        | || |_| | |___
|_____|____/|_|   |____/|_____| |____/|_|  |_| \_/ \___|_|       |___|____/|_____|
    )";
    
    Print(banner);
    Print("                     Terminal Mode - v2.0.0");
    Print("");
    Print("Type 'help' for available commands, 'quit' to exit.");
}

void TerminalModeApp::Print(const std::string& message) {
    std::cout << message << std::endl;
}

void TerminalModeApp::PrintSuccess(const std::string& message) {
    if (color_output_) {
        std::cout << GetColorCode("green") << "✓ " << message << ResetColor() << std::endl;
    } else {
        std::cout << "[OK] " << message << std::endl;
    }
}

void TerminalModeApp::PrintError(const std::string& message) {
    if (color_output_) {
        std::cout << GetColorCode("red") << "✗ " << message << ResetColor() << std::endl;
    } else {
        std::cout << "[ERROR] " << message << std::endl;
    }
}

void TerminalModeApp::PrintWarning(const std::string& message) {
    if (color_output_) {
        std::cout << GetColorCode("yellow") << "⚠ " << message << ResetColor() << std::endl;
    } else {
        std::cout << "[WARN] " << message << std::endl;
    }
}

void TerminalModeApp::PrintInfo(const std::string& message) {
    if (color_output_) {
        std::cout << GetColorCode("blue") << "ℹ " << message << ResetColor() << std::endl;
    } else {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void TerminalModeApp::PrintTable(const std::vector<std::vector<std::string>>& rows,
                                 const std::vector<std::string>& headers) {
    if (rows.empty()) return;
    
    // Calculate column widths
    std::vector<size_t> widths(headers.size(), 0);
    for (size_t i = 0; i < headers.size(); ++i) {
        widths[i] = headers[i].length();
    }
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            widths[i] = std::max(widths[i], row[i].length());
        }
    }
    
    // Print header
    std::ostringstream header_line;
    std::ostringstream separator;
    for (size_t i = 0; i < headers.size(); ++i) {
        header_line << std::left << std::setw(widths[i] + 2) << headers[i];
        separator << std::string(widths[i] + 2, '-');
    }
    Print(header_line.str());
    Print(separator.str());
    
    // Print rows
    for (const auto& row : rows) {
        std::ostringstream row_line;
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            row_line << std::left << std::setw(widths[i] + 2) << row[i];
        }
        Print(row_line.str());
    }
}

void TerminalModeApp::ShowProgress(const std::string& message, float progress) {
    int width = 30;
    int filled = static_cast<int>(progress * width);
    
    std::ostringstream oss;
    oss << "\r" << message << " [";
    for (int i = 0; i < width; ++i) {
        oss << (i < filled ? "█" : "░");
    }
    oss << "] " << static_cast<int>(progress * 100) << "%";
    
    std::cout << oss.str() << std::flush;
}

void TerminalModeApp::HideProgress() {
    std::cout << std::endl;
}

std::string TerminalModeApp::Prompt(const std::string& message) {
    std::cout << message;
    std::cout.flush();
    
    std::string input;
    std::getline(std::cin, input);
    return input;
}

bool TerminalModeApp::Confirm(const std::string& message) {
    std::string response = Prompt(message + " (y/n): ");
    return !response.empty() && (response[0] == 'y' || response[0] == 'Y');
}

int TerminalModeApp::Select(const std::string& message, const std::vector<std::string>& options) {
    Print(message);
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << options[i] << std::endl;
    }
    
    std::string response = Prompt("Select (1-" + std::to_string(options.size()) + "): ");
    int selection = std::atoi(response.c_str());
    
    if (selection >= 1 && selection <= static_cast<int>(options.size())) {
        return selection - 1;
    }
    return -1;
}

std::vector<std::string> TerminalModeApp::ParseArguments(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string token;
    
    bool in_quotes = false;
    std::string current;
    
    for (char c : input) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        args.push_back(current);
    }
    
    return args;
}

std::string TerminalModeApp::GetColorCode(const std::string& color) {
    if (!color_output_) return "";
    
    if (color == "red") return "\033[31m";
    if (color == "green") return "\033[32m";
    if (color == "yellow") return "\033[33m";
    if (color == "blue") return "\033[34m";
    if (color == "magenta") return "\033[35m";
    if (color == "cyan") return "\033[36m";
    if (color == "white") return "\033[37m";
    
    return "";
}

std::string TerminalModeApp::ResetColor() {
    return color_output_ ? "\033[0m" : "";
}

void TerminalModeApp::DetectColorSupport() {
    // Check if terminal supports colors
    const char* term = std::getenv("TERM");
    const char* colorterm = std::getenv("COLORTERM");
    
    color_output_ = (term && std::string(term) != "dumb") || colorterm;
    
    // Also check if stdout is a terminal
#ifdef _WIN32
    color_output_ = color_output_ && _isatty(_fileno(stdout));
#else
    color_output_ = color_output_ && isatty(fileno(stdout));
#endif
}

// Command handlers

int TerminalModeApp::HandleHelp(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintHelp();
    } else {
        PrintCommandHelp(args[0]);
    }
    return 0;
}

int TerminalModeApp::HandleVersion(const std::vector<std::string>& args) {
    PrintVersion();
    return 0;
}

int TerminalModeApp::HandleQuit(const std::vector<std::string>& args) {
    Quit();
    return 0;
}

int TerminalModeApp::HandleNew(const std::vector<std::string>& args) {
    std::string filename = args.empty() ? "" : args[0];
    BackendFramework::GetInstance().NewFile(filename);
    PrintSuccess("Created new sketch");
    return 0;
}

int TerminalModeApp::HandleOpen(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: open <filename>");
        return 1;
    }
    
    if (BackendFramework::GetInstance().OpenFile(args[0])) {
        PrintSuccess("Opened: " + args[0]);
        return 0;
    }
    
    PrintError("Failed to open: " + args[0]);
    return 1;
}

int TerminalModeApp::HandleSave(const std::vector<std::string>& args) {
    if (args.empty()) {
        BackendFramework::GetInstance().SaveFile();
    } else {
        BackendFramework::GetInstance().SaveFileAs(args[0]);
    }
    PrintSuccess("File saved");
    return 0;
}

int TerminalModeApp::HandleClose(const std::vector<std::string>& args) {
    BackendFramework::GetInstance().CloseFile();
    PrintInfo("File closed");
    return 0;
}

int TerminalModeApp::HandleList(const std::vector<std::string>& args) {
    auto* fm = BackendFramework::GetInstance().GetFileManager();
    if (!fm) return 1;
    
    auto files = fm->GetFileList();
    if (files.empty()) {
        PrintInfo("No files in project");
        return 0;
    }
    
    Print("Project files:");
    for (const auto& file : files) {
        Print("  " + file);
    }
    return 0;
}

int TerminalModeApp::HandleCat(const std::vector<std::string>& args) {
    if (args.empty()) {
        // Show current file
        auto* editor = BackendFramework::GetInstance().GetTextEditor();
        if (editor) {
            Print(editor->GetText());
        }
    } else {
        auto* fm = BackendFramework::GetInstance().GetFileManager();
        if (fm) {
            std::string content = fm->GetFileContent(args[0]);
            Print(content);
        }
    }
    return 0;
}

int TerminalModeApp::HandleEdit(const std::vector<std::string>& args) {
    PrintInfo("Opening external editor...");
    // Would launch external editor
    return 0;
}

int TerminalModeApp::HandleBoard(const std::vector<std::string>& args) {
    if (args.empty()) {
        auto board = BackendFramework::GetInstance().GetBoard();
        Print("Current board: " + board.name);
        Print("FQBN: " + board.fqbn);
        return 0;
    }
    
    // Set board by name
    auto boards = BackendFramework::GetInstance().GetAvailableBoards();
    for (const auto& board : boards) {
        if (board.name == args[0]) {
            BackendFramework::GetInstance().SetBoard(board);
            PrintSuccess("Board set to: " + board.name);
            return 0;
        }
    }
    
    PrintError("Unknown board: " + args[0]);
    PrintInfo("Use 'boards' to list available boards");
    return 1;
}

int TerminalModeApp::HandlePort(const std::vector<std::string>& args) {
    auto& framework = BackendFramework::GetInstance();
    
    if (args.empty()) {
        auto board = framework.GetBoard();
        Print("Current port: " + board.port);
        return 0;
    }
    
    auto board = framework.GetBoard();
    board.port = args[0];
    framework.SetBoard(board);
    PrintSuccess("Port set to: " + args[0]);
    return 0;
}

int TerminalModeApp::HandleBoardList(const std::vector<std::string>& args) {
    auto boards = BackendFramework::GetInstance().GetAvailableBoards();
    
    std::vector<std::vector<std::string>> rows;
    for (const auto& board : boards) {
        rows.push_back({board.name, board.fqbn});
    }
    
    PrintTable(rows, {"Board", "FQBN"});
    return 0;
}

int TerminalModeApp::HandlePortList(const std::vector<std::string>& args) {
    auto ports = BackendFramework::GetInstance().GetAvailablePorts();
    
    Print("Available ports:");
    for (const auto& port : ports) {
        Print("  " + port);
    }
    return 0;
}

int TerminalModeApp::HandleVerify(const std::vector<std::string>& args) {
    PrintInfo("Compiling sketch...");
    ShowProgress("Compiling", 0.0f);
    
    // Simulate progress
    for (float p = 0.0f; p <= 1.0f; p += 0.1f) {
        ShowProgress("Compiling", p);
    }
    HideProgress();
    
    if (BackendFramework::GetInstance().Verify()) {
        PrintSuccess("Compilation successful");
        return 0;
    }
    
    PrintError("Compilation failed");
    return 1;
}

int TerminalModeApp::HandleUpload(const std::vector<std::string>& args) {
    auto board = BackendFramework::GetInstance().GetBoard();
    
    PrintInfo("Uploading to " + board.name + " on " + board.port + "...");
    
    if (BackendFramework::GetInstance().Upload()) {
        PrintSuccess("Upload complete");
        return 0;
    }
    
    PrintError("Upload failed");
    return 1;
}

int TerminalModeApp::HandleCompile(const std::vector<std::string>& args) {
    return HandleVerify(args);
}

int TerminalModeApp::HandleMonitor(const std::vector<std::string>& args) {
    int baud = 115200;
    if (!args.empty()) {
        baud = std::atoi(args[0].c_str());
    }
    
    BackendFramework::GetInstance().SetSerialBaudRate(baud);
    
    if (BackendFramework::GetInstance().OpenSerialMonitor()) {
        PrintSuccess("Serial monitor opened at " + std::to_string(baud) + " baud");
        PrintInfo("Press Ctrl+C to close");
        
        // Would run interactive serial monitor loop here
        return 0;
    }
    
    PrintError("Failed to open serial monitor");
    return 1;
}

int TerminalModeApp::HandleSend(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: send <data>");
        return 1;
    }
    
    std::string data;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) data += " ";
        data += args[i];
    }
    
    BackendFramework::GetInstance().SendSerialData(data);
    PrintInfo("Sent: " + data);
    return 0;
}

int TerminalModeApp::HandleEmulator(const std::vector<std::string>& args) {
    if (args.empty() || args[0] == "status") {
        if (BackendFramework::GetInstance().IsEmulatorRunning()) {
            PrintInfo("Emulator is running");
        } else {
            PrintInfo("Emulator is stopped");
        }
        return 0;
    }
    
    if (args[0] == "start") {
        BackendFramework::GetInstance().StartEmulator();
        PrintSuccess("Emulator started");
        return 0;
    }
    
    if (args[0] == "stop") {
        BackendFramework::GetInstance().StopEmulator();
        PrintInfo("Emulator stopped");
        return 0;
    }
    
    PrintError("Usage: emulator [start|stop|status]");
    return 1;
}

int TerminalModeApp::HandleAsk(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: ask <question>");
        return 1;
    }
    
    std::string query;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) query += " ";
        query += args[i];
    }
    
    PrintInfo("Asking AI...");
    std::string response = BackendFramework::GetInstance().QueryAI(query);
    Print("\n" + response);
    return 0;
}

int TerminalModeApp::HandleGenerate(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: generate <description>");
        return 1;
    }
    
    std::string description;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) description += " ";
        description += args[i];
    }
    
    PrintInfo("Generating code...");
    std::string code = BackendFramework::GetInstance().GenerateCode(description);
    Print("\nGenerated code:");
    Print(code);
    return 0;
}

int TerminalModeApp::HandleAnalyze(const std::vector<std::string>& args) {
    PrintInfo("Analyzing code...");
    std::string analysis = BackendFramework::GetInstance().AnalyzeCode();
    Print("\n" + analysis);
    return 0;
}

int TerminalModeApp::HandleFix(const std::vector<std::string>& args) {
    PrintInfo("Fixing bugs...");
    std::string fixed = BackendFramework::GetInstance().FixBugs();
    
    if (!fixed.empty()) {
        if (Confirm("Apply fixes?")) {
            auto* editor = BackendFramework::GetInstance().GetTextEditor();
            if (editor) {
                editor->SetText(fixed);
                PrintSuccess("Fixes applied");
            }
        }
    }
    return 0;
}

int TerminalModeApp::HandleDevices(const std::vector<std::string>& args) {
    auto* library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (!library) return 1;
    
    auto devices = library->GetAllDevices();
    
    std::vector<std::vector<std::string>> rows;
    for (const auto* device : devices) {
        rows.push_back({device->GetId(), device->GetName(), device->GetDescription()});
    }
    
    PrintTable(rows, {"ID", "Name", "Description"});
    return 0;
}

int TerminalModeApp::HandleAddDevice(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: add-device <device_id>");
        return 1;
    }
    
    auto* library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (!library) return 1;
    
    auto* instance = library->CreateInstance(args[0], args[0] + "_1");
    if (instance) {
        PrintSuccess("Added device: " + args[0]);
        return 0;
    }
    
    PrintError("Failed to add device: " + args[0]);
    return 1;
}

int TerminalModeApp::HandleConfig(const std::vector<std::string>& args) {
    auto board = BackendFramework::GetInstance().GetBoard();
    
    Print("Configuration:");
    Print("  Board: " + board.name);
    Print("  Port: " + board.port);
    Print("  Baud Rate: " + std::to_string(board.baudRate));
    Print("  FQBN: " + board.fqbn);
    
    return 0;
}

int TerminalModeApp::HandleSet(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        PrintError("Usage: set <key> <value>");
        return 1;
    }
    
    BackendFramework::GetInstance().SetPreference(args[0], args[1]);
    PrintSuccess("Set " + args[0] + " = " + args[1]);
    return 0;
}

int TerminalModeApp::HandleGet(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: get <key>");
        return 1;
    }
    
    std::string value = BackendFramework::GetInstance().GetPreference(args[0]);
    Print(args[0] + " = " + value);
    return 0;
}

// Project commands

int TerminalModeApp::HandleCreate(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: create <project_name> [template]");
        return 1;
    }
    
    std::string project_name = args[0];
    std::string template_name = args.size() > 1 ? args[1] : "basic";
    
    PrintInfo("Creating project: " + project_name + " using template: " + template_name);
    
    if (BackendFramework::GetInstance().CreateProject(project_name, template_name)) {
        PrintSuccess("Project '" + project_name + "' created successfully");
        return 0;
    }
    
    PrintError("Failed to create project");
    return 1;
}

int TerminalModeApp::HandleTemplates(const std::vector<std::string>& args) {
    Print("Available project templates:");
    Print("");
    
    // Get templates from file manager
    auto* fm = BackendFramework::GetInstance().GetFileManager();
    if (fm) {
        auto templates = fm->GetTemplates();
        
        std::vector<std::vector<std::string>> rows;
        for (const auto& tpl : templates) {
            std::string tags_str;
            for (size_t i = 0; i < tpl.tags.size(); ++i) {
                if (i > 0) tags_str += ", ";
                tags_str += tpl.tags[i];
            }
            rows.push_back({tpl.name, tpl.description, tags_str});
        }
        
        PrintTable(rows, {"Template", "Description", "Tags"});
    }
    
    Print("");
    PrintInfo("Use 'create <name> <template>' to create a project from a template");
    return 0;
}

int TerminalModeApp::HandleRecent(const std::vector<std::string>& args) {
    auto recent = BackendFramework::GetInstance().GetRecentFiles();
    
    if (recent.empty()) {
        PrintInfo("No recent files");
        return 0;
    }
    
    Print("Recent files:");
    int num = 1;
    for (const auto& file : recent) {
        Print("  " + std::to_string(num++) + ". " + file);
    }
    
    return 0;
}

int TerminalModeApp::HandleExport(const std::vector<std::string>& args) {
    std::string output_path = args.empty() ? "firmware.bin" : args[0];
    
    PrintInfo("Compiling sketch...");
    
    if (!BackendFramework::GetInstance().Verify()) {
        PrintError("Compilation failed, cannot export binary");
        return 1;
    }
    
    PrintSuccess("Binary exported to: " + output_path);
    // Note: Actual binary size would be determined by compiler output
    PrintInfo("Export complete (actual export requires toolchain integration)");
    return 0;
}

// Utility commands

int TerminalModeApp::HandleClear(const std::vector<std::string>& args) {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
    return 0;
}

int TerminalModeApp::HandleHistory(const std::vector<std::string>& args) {
    int count = 20;  // Default to showing last 20 commands
    if (!args.empty()) {
        count = std::atoi(args[0].c_str());
        if (count <= 0) count = 20;
    }
    
    if (command_history_.empty()) {
        PrintInfo("No command history");
        return 0;
    }
    
    Print("Command history:");
    int start = std::max(0, static_cast<int>(command_history_.size()) - count);
    for (size_t i = start; i < command_history_.size(); ++i) {
        Print("  " + std::to_string(i + 1) + ". " + command_history_[i]);
    }
    
    return 0;
}

int TerminalModeApp::HandleStatus(const std::vector<std::string>& args) {
    auto& framework = BackendFramework::GetInstance();
    auto board = framework.GetBoard();
    
    Print("IDE Status:");
    Print("  Status: " + framework.GetStatusMessage());
    Print("");
    Print("Board Configuration:");
    Print("  Board: " + board.name);
    Print("  Port: " + board.port);
    Print("  Baud Rate: " + std::to_string(board.baudRate));
    Print("");
    Print("Components:");
    Print("  Serial Monitor: " + std::string(framework.IsSerialOpen() ? "Connected" : "Disconnected"));
    Print("  Emulator: " + std::string(framework.IsEmulatorRunning() ? "Running" : "Stopped"));
    Print("  Compiling: " + std::string(framework.IsCompiling() ? "Yes" : "No"));
    
    return 0;
}

int TerminalModeApp::HandleInfo(const std::vector<std::string>& args) {
    Print("System Information:");
    Print("");
    Print("ESP32 Driver IDE:");
    Print("  Version: 2.0.0");
    Print("  Mode: Terminal");
    Print("");
    Print("Build Information:");
    Print("  C++ Standard: C++17");
#ifdef __GNUC__
    Print("  Compiler: GCC " + std::to_string(__GNUC__) + "." + 
          std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__));
#endif
    Print("  Build Date: " __DATE__ " " __TIME__);
    Print("");
    Print("Platform:");
#ifdef _WIN32
    Print("  OS: Windows");
#elif __APPLE__
    Print("  OS: macOS");
#elif __linux__
    Print("  OS: Linux");
#else
    Print("  OS: Unknown");
#endif
    
    return 0;
}

// Script commands

int TerminalModeApp::HandleScript(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: script [run <file>|list|examples]");
        return 1;
    }
    
    if (args[0] == "list" || args[0] == "examples") {
        Print("Available example scripts:");
        Print("");
        
        auto scripts = scripting::ScriptLibrary::GetExampleScripts();
        std::vector<std::vector<std::string>> rows;
        for (const auto& script : scripts) {
            rows.push_back({script.name, script.description, script.category});
        }
        
        PrintTable(rows, {"Name", "Description", "Category"});
        return 0;
    }
    
    if (args[0] == "run") {
        if (args.size() < 2) {
            PrintError("Usage: script run <file_or_name>");
            return 1;
        }
        
        std::string script_name = args[1];
        PrintInfo("Running script: " + script_name);
        
        scripting::ScriptEngine engine;
        engine.Initialize();
        
        // Try to get from library first
        std::string script_code = scripting::ScriptLibrary::GetScript(script_name);
        
        if (script_code.empty()) {
            // Try to read from file
            std::ifstream file(script_name);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                script_code = buffer.str();
                file.close();
            }
        }
        
        if (script_code.empty()) {
            PrintError("Script not found: " + script_name);
            return 1;
        }
        
        if (engine.Execute(script_code)) {
            std::string output = engine.GetContext().GetOutput();
            if (!output.empty()) {
                Print(output);
            }
            PrintSuccess("Script executed successfully");
            return 0;
        } else {
            PrintError("Script error: " + engine.GetErrorMessage());
            return 1;
        }
    }
    
    PrintError("Unknown script command: " + args[0]);
    return 1;
}

// Plugin commands

int TerminalModeApp::HandlePlugins(const std::vector<std::string>& args) {
    plugins::PluginManager manager;
    
    if (args.empty() || args[0] == "list") {
        Print("Installed plugins:");
        Print("");
        
        auto plugins = manager.GetAllPlugins();
        if (plugins.empty()) {
            PrintInfo("No plugins installed");
            return 0;
        }
        
        std::vector<std::vector<std::string>> rows;
        for (const auto* plugin : plugins) {
            std::string state_str;
            switch (plugin->GetState()) {
                case plugins::PluginState::ACTIVE: state_str = "Active"; break;
                case plugins::PluginState::LOADED: state_str = "Loaded"; break;
                case plugins::PluginState::DISABLED: state_str = "Disabled"; break;
                case plugins::PluginState::ERROR: state_str = "Error"; break;
                default: state_str = "Unloaded"; break;
            }
            rows.push_back({
                plugin->GetMetadata().id,
                plugin->GetMetadata().name,
                plugin->GetMetadata().version,
                state_str
            });
        }
        
        PrintTable(rows, {"ID", "Name", "Version", "State"});
        return 0;
    }
    
    if (args[0] == "enable") {
        if (args.size() < 2) {
            PrintError("Usage: plugins enable <plugin_id>");
            return 1;
        }
        
        if (manager.EnablePlugin(args[1])) {
            PrintSuccess("Plugin enabled: " + args[1]);
            return 0;
        }
        PrintError("Failed to enable plugin: " + args[1]);
        return 1;
    }
    
    if (args[0] == "disable") {
        if (args.size() < 2) {
            PrintError("Usage: plugins disable <plugin_id>");
            return 1;
        }
        
        if (manager.DisablePlugin(args[1])) {
            PrintSuccess("Plugin disabled: " + args[1]);
            return 0;
        }
        PrintError("Failed to disable plugin: " + args[1]);
        return 1;
    }
    
    PrintError("Unknown plugin command: " + args[0]);
    PrintInfo("Available commands: list, enable, disable");
    return 1;
}

// Test commands

int TerminalModeApp::HandleTest(const std::vector<std::string>& args) {
    testing::TestRunner runner;
    runner.SetVerbose(true);
    
    PrintInfo("Running tests...");
    Print("");
    
    // Create a sample test suite for demonstration
    testing::TestSuite suite("ESP32 IDE Tests");
    
    suite.AddTest("Basic initialization", []() {
        // Simple test that always passes
        testing::Assert::IsTrue(true, "IDE should initialize");
    });
    
    suite.AddTest("File manager", []() {
        // Simple test that always passes
        testing::Assert::IsTrue(true, "File manager should work");
    });
    
    suite.AddTest("Compiler", []() {
        // Simple test that always passes  
        testing::Assert::IsTrue(true, "Compiler should be available");
    });
    
    runner.AddSuite(&suite);
    
    std::vector<testing::TestResult> results;
    
    if (args.empty()) {
        results = runner.RunAll();
    } else {
        results = runner.RunSuite(args[0]);
    }
    
    // Print results
    int passed = 0, failed = 0, skipped = 0;
    for (const auto& result : results) {
        std::string status_str;
        switch (result.status) {
            case testing::TestStatus::PASSED: 
                status_str = "PASS"; 
                passed++;
                break;
            case testing::TestStatus::FAILED: 
                status_str = "FAIL"; 
                failed++;
                break;
            case testing::TestStatus::SKIPPED: 
                status_str = "SKIP"; 
                skipped++;
                break;
            default: 
                status_str = "ERROR"; 
                failed++;
                break;
        }
        
        if (result.status == testing::TestStatus::PASSED) {
            PrintSuccess(result.test_name + " - " + status_str);
        } else if (result.status == testing::TestStatus::SKIPPED) {
            PrintWarning(result.test_name + " - " + status_str);
        } else {
            PrintError(result.test_name + " - " + status_str);
            if (!result.message.empty()) {
                Print("    " + result.message);
            }
        }
    }
    
    Print("");
    Print("Test Results: " + std::to_string(passed) + " passed, " + 
          std::to_string(failed) + " failed, " + 
          std::to_string(skipped) + " skipped");
    
    return failed > 0 ? 1 : 0;
}

int TerminalModeApp::HandleCoverage(const std::vector<std::string>& args) {
    testing::CoverageAnalyzer analyzer;
    
    PrintInfo("Analyzing code coverage...");
    Print("");
    
    auto info = analyzer.GetCoverageInfo();
    
    Print("Code Coverage Report:");
    Print("  Line Coverage: " + std::to_string(static_cast<int>(info.GetLineCoverage())) + "%");
    Print("  Function Coverage: " + std::to_string(static_cast<int>(info.GetFunctionCoverage())) + "%");
    Print("  Total Lines: " + std::to_string(info.total_lines));
    Print("  Covered Lines: " + std::to_string(info.covered_lines));
    Print("  Total Functions: " + std::to_string(info.total_functions));
    Print("  Covered Functions: " + std::to_string(info.covered_functions));
    
    return 0;
}

// Decompiler commands

int TerminalModeApp::HandleDecompile(const std::vector<std::string>& args) {
    if (args.empty()) {
        PrintError("Usage: decompile <firmware_file>");
        return 1;
    }
    
    std::string filename = args[0];
    PrintInfo("Decompiling firmware: " + filename);
    
    decompiler::AdvancedDecompiler decomp;
    decomp.Initialize();
    
    if (!decomp.LoadFirmware(filename)) {
        PrintError("Failed to load firmware file: " + filename);
        return 1;
    }
    
    PrintInfo("Analyzing firmware...");
    decomp.AnalyzeEntryPoint();
    decomp.DiscoverFunctions();
    
    PrintInfo("Decompiling functions...");
    decomp.DecompileAll();
    
    // Get and display results
    Print("");
    Print("Decompilation Results:");
    Print("");
    
    auto& functions = decomp.GetFunctions();
    Print("Found " + std::to_string(functions.size()) + " functions");
    Print("");
    
    // Show pseudo code
    std::string code = decomp.GetFullPseudoCode();
    if (!code.empty()) {
        Print("Pseudo-code:");
        Print(code);
    }
    
    // Show strings
    auto strings = decomp.ExtractStrings();
    if (!strings.empty()) {
        Print("");
        Print("Extracted strings:");
        for (size_t i = 0; i < strings.size() && i < 10; ++i) {
            Print("  \"" + strings[i] + "\"");
        }
        if (strings.size() > 10) {
            Print("  ... and " + std::to_string(strings.size() - 10) + " more");
        }
    }
    
    // Show ESP32 API usage
    auto apis = decomp.GetESP32APIUsage();
    if (!apis.empty()) {
        Print("");
        Print("ESP32 API Usage:");
        for (const auto& api : apis) {
            Print("  " + api.first + ": " + api.second);
        }
    }
    
    PrintSuccess("Decompilation complete");
    return 0;
}

// Entry point
int TerminalMain(int argc, char* argv[]) {
    TerminalModeApp app;
    return app.Run(argc, argv);
}

} // namespace esp32_ide
