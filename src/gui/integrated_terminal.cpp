#include "gui/integrated_terminal.h"
#include <algorithm>
#include <sstream>
#include <chrono>

namespace esp32_ide {
namespace gui {

IntegratedTerminal::IntegratedTerminal()
    : history_index_(0), max_lines_(1000), auto_scroll_(true), 
      theme_(TerminalTheme::DARK), is_running_process_(false) {
    working_directory_ = "/";
}

IntegratedTerminal::~IntegratedTerminal() = default;

void IntegratedTerminal::Initialize() {
    Clear();
    WriteLine("ESP32 Driver IDE - Integrated Terminal", GetThemeColors().info);
    WriteLine("Type 'help' for available commands", GetThemeColors().info);
    WriteLine("");
}

void IntegratedTerminal::Clear() {
    lines_.clear();
}

void IntegratedTerminal::Reset() {
    Clear();
    Initialize();
}

void IntegratedTerminal::WriteLine(const std::string& line, const std::string& color) {
    AddLine(line, color, false);
}

void IntegratedTerminal::WriteOutput(const std::string& text) {
    WriteLine(text, GetThemeColors().foreground);
}

void IntegratedTerminal::WriteError(const std::string& text) {
    WriteLine(text, GetThemeColors().error);
}

void IntegratedTerminal::WriteSuccess(const std::string& text) {
    WriteLine(text, GetThemeColors().success);
}

void IntegratedTerminal::WriteWarning(const std::string& text) {
    WriteLine(text, GetThemeColors().warning);
}

void IntegratedTerminal::SetInput(const std::string& input) {
    current_input_ = input;
}

void IntegratedTerminal::ClearInput() {
    current_input_ = "";
}

void IntegratedTerminal::ExecuteCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }
    
    // Add to display
    AddLine("> " + command, GetThemeColors().info, true);
    
    // Add to history
    AddToHistory(command);
    
    // Execute built-in command or callback
    std::string result = ExecuteBuiltInCommand(command);
    
    if (result.empty() && command_callback_) {
        result = command_callback_(command);
    }
    
    if (!result.empty()) {
        WriteOutput(result);
    }
    
    ClearInput();
}

std::vector<IntegratedTerminal::TerminalLine> IntegratedTerminal::GetLines() const {
    return lines_;
}

void IntegratedTerminal::ClearHistory() {
    command_history_.clear();
    history_index_ = 0;
}

std::string IntegratedTerminal::GetPreviousCommand() {
    if (command_history_.empty()) {
        return "";
    }
    
    if (history_index_ > 0) {
        history_index_--;
    }
    
    if (history_index_ < command_history_.size()) {
        return command_history_[history_index_];
    }
    
    return "";
}

std::string IntegratedTerminal::GetNextCommand() {
    if (command_history_.empty()) {
        return "";
    }
    
    if (history_index_ < command_history_.size() - 1) {
        history_index_++;
        return command_history_[history_index_];
    }
    
    history_index_ = command_history_.size();
    return "";
}

void IntegratedTerminal::AddToHistory(const std::string& command) {
    // Don't add duplicates or empty commands
    if (command.empty() || (!command_history_.empty() && command_history_.back() == command)) {
        return;
    }
    
    command_history_.push_back(command);
    history_index_ = command_history_.size();
    
    // Limit history size
    if (command_history_.size() > 100) {
        command_history_.pop_front();
    }
}

void IntegratedTerminal::SetTheme(TerminalTheme theme) {
    theme_ = theme;
}

std::vector<std::string> IntegratedTerminal::GetBuiltInCommands() const {
    return {
        "help", "clear", "cls", "echo", "cd", "pwd", "env",
        "compile", "upload", "monitor", "list-ports", "version"
    };
}

std::string IntegratedTerminal::ExecuteBuiltInCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    std::string args;
    std::getline(iss, args);
    if (!args.empty() && args[0] == ' ') {
        args = args.substr(1);
    }
    
    if (cmd == "help") {
        return HandleHelpCommand();
    } else if (cmd == "clear" || cmd == "cls") {
        return HandleClearCommand();
    } else if (cmd == "echo") {
        return HandleEchoCommand(args);
    } else if (cmd == "cd") {
        return HandleCdCommand(args);
    } else if (cmd == "pwd") {
        return HandlePwdCommand();
    } else if (cmd == "env") {
        return HandleEnvCommand();
    } else if (cmd == "version") {
        return "ESP32 Driver IDE v1.2.0";
    }
    
    return ""; // Not a built-in command
}

void IntegratedTerminal::SetEnvironmentVariable(const std::string& name, const std::string& value) {
    env_vars_[name] = value;
}

std::string IntegratedTerminal::GetEnvironmentVariable(const std::string& name) const {
    auto it = env_vars_.find(name);
    if (it != env_vars_.end()) {
        return it->second;
    }
    return "";
}

void IntegratedTerminal::StopCurrentProcess() {
    if (is_running_process_) {
        WriteWarning("Process stopped by user");
        is_running_process_ = false;
        current_process_ = "";
    }
}

IntegratedTerminal::ThemeColors IntegratedTerminal::GetThemeColors() const {
    ThemeColors colors;
    
    switch (theme_) {
        case TerminalTheme::DARK:
            colors.background = "#1e1e1e";
            colors.foreground = "#d4d4d4";
            colors.error = "#f48771";
            colors.success = "#89d185";
            colors.warning = "#dcdcaa";
            colors.info = "#569cd6";
            break;
            
        case TerminalTheme::LIGHT:
            colors.background = "#ffffff";
            colors.foreground = "#000000";
            colors.error = "#cd3131";
            colors.success = "#00bc00";
            colors.warning = "#949800";
            colors.info = "#0070c1";
            break;
            
        case TerminalTheme::SOLARIZED:
            colors.background = "#002b36";
            colors.foreground = "#839496";
            colors.error = "#dc322f";
            colors.success = "#859900";
            colors.warning = "#b58900";
            colors.info = "#268bd2";
            break;
            
        case TerminalTheme::MONOKAI:
            colors.background = "#272822";
            colors.foreground = "#f8f8f2";
            colors.error = "#f92672";
            colors.success = "#a6e22e";
            colors.warning = "#e6db74";
            colors.info = "#66d9ef";
            break;
    }
    
    return colors;
}

void IntegratedTerminal::AddLine(const std::string& content, const std::string& color, bool is_input) {
    TerminalLine line;
    line.content = content;
    line.color = color.empty() ? GetThemeColors().foreground : color;
    line.is_input = is_input;
    line.timestamp = GetCurrentTimestamp();
    
    lines_.push_back(line);
    TrimLines();
}

void IntegratedTerminal::TrimLines() {
    while (lines_.size() > max_lines_) {
        lines_.erase(lines_.begin());
    }
}

long long IntegratedTerminal::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string IntegratedTerminal::HandleClearCommand() {
    Clear();
    return "";
}

std::string IntegratedTerminal::HandleHelpCommand() {
    std::string help = "Available commands:\n";
    help += "  help          - Show this help message\n";
    help += "  clear/cls     - Clear the terminal\n";
    help += "  echo <text>   - Print text to terminal\n";
    help += "  cd <dir>      - Change working directory\n";
    help += "  pwd           - Print working directory\n";
    help += "  env           - Show environment variables\n";
    help += "  compile       - Compile current sketch\n";
    help += "  upload        - Upload to ESP32 device\n";
    help += "  monitor       - Open serial monitor\n";
    help += "  list-ports    - List available serial ports\n";
    help += "  version       - Show IDE version\n";
    return help;
}

std::string IntegratedTerminal::HandleEchoCommand(const std::string& args) {
    return args;
}

std::string IntegratedTerminal::HandleCdCommand(const std::string& path) {
    if (path.empty()) {
        return working_directory_;
    }
    
    // Simple path handling (would need proper implementation)
    if (path[0] == '/') {
        working_directory_ = path;
    } else {
        if (working_directory_.back() != '/') {
            working_directory_ += '/';
        }
        working_directory_ += path;
    }
    
    return "";
}

std::string IntegratedTerminal::HandlePwdCommand() {
    return working_directory_;
}

std::string IntegratedTerminal::HandleEnvCommand() {
    std::string result = "Environment variables:\n";
    for (const auto& pair : env_vars_) {
        result += "  " + pair.first + "=" + pair.second + "\n";
    }
    return result;
}

} // namespace gui
} // namespace esp32_ide
