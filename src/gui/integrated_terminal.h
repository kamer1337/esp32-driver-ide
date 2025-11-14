#ifndef INTEGRATED_TERMINAL_H
#define INTEGRATED_TERMINAL_H

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <functional>

namespace esp32_ide {
namespace gui {

/**
 * @brief Integrated terminal widget for command execution
 * 
 * Provides a terminal interface within the IDE for running commands,
 * compiling, and interacting with the ESP32 device
 */
class IntegratedTerminal {
public:
    struct TerminalLine {
        std::string content;
        std::string color;  // ANSI color code or RGB hex
        bool is_input;
        long long timestamp;
    };
    
    enum class TerminalTheme {
        DARK,
        LIGHT,
        SOLARIZED,
        MONOKAI
    };
    
    IntegratedTerminal();
    ~IntegratedTerminal();
    
    // Terminal lifecycle
    void Initialize();
    void Clear();
    void Reset();
    
    // Content management
    void WriteLine(const std::string& line, const std::string& color = "");
    void WriteOutput(const std::string& text);
    void WriteError(const std::string& text);
    void WriteSuccess(const std::string& text);
    void WriteWarning(const std::string& text);
    
    // Input handling
    void SetInput(const std::string& input);
    std::string GetInput() const { return current_input_; }
    void ClearInput();
    void ExecuteCommand(const std::string& command);
    
    // History management
    std::vector<TerminalLine> GetLines() const;
    std::deque<std::string> GetCommandHistory() const { return command_history_; }
    void ClearHistory();
    
    // Command history navigation
    std::string GetPreviousCommand();
    std::string GetNextCommand();
    void AddToHistory(const std::string& command);
    
    // Appearance
    void SetTheme(TerminalTheme theme);
    TerminalTheme GetTheme() const { return theme_; }
    void SetMaxLines(size_t max_lines) { max_lines_ = max_lines; }
    
    // Auto-scroll
    void SetAutoScroll(bool enabled) { auto_scroll_ = enabled; }
    bool GetAutoScroll() const { return auto_scroll_; }
    
    // Command execution callback
    using CommandCallback = std::function<std::string(const std::string& command)>;
    void SetCommandCallback(CommandCallback callback) { command_callback_ = callback; }
    
    // Built-in commands
    std::vector<std::string> GetBuiltInCommands() const;
    std::string ExecuteBuiltInCommand(const std::string& command);
    
    // Working directory
    void SetWorkingDirectory(const std::string& dir) { working_directory_ = dir; }
    std::string GetWorkingDirectory() const { return working_directory_; }
    
    // Environment variables
    void SetEnvironmentVariable(const std::string& name, const std::string& value);
    std::string GetEnvironmentVariable(const std::string& name) const;
    std::map<std::string, std::string> GetEnvironmentVariables() const { return env_vars_; }
    
    // Process management
    bool IsProcessRunning() const { return is_running_process_; }
    void StopCurrentProcess();
    
private:
    // Terminal state
    std::vector<TerminalLine> lines_;
    std::string current_input_;
    std::deque<std::string> command_history_;
    size_t history_index_;
    size_t max_lines_;
    bool auto_scroll_;
    TerminalTheme theme_;
    
    // Process state
    bool is_running_process_;
    std::string current_process_;
    
    // Working directory and environment
    std::string working_directory_;
    std::map<std::string, std::string> env_vars_;
    
    // Callback
    CommandCallback command_callback_;
    
    // Theme colors
    struct ThemeColors {
        std::string background;
        std::string foreground;
        std::string error;
        std::string success;
        std::string warning;
        std::string info;
    };
    ThemeColors GetThemeColors() const;
    
    // Helper methods
    void AddLine(const std::string& content, const std::string& color, bool is_input);
    void TrimLines();
    long long GetCurrentTimestamp() const;
    
    // Built-in command handlers
    std::string HandleClearCommand();
    std::string HandleHelpCommand();
    std::string HandleEchoCommand(const std::string& args);
    std::string HandleCdCommand(const std::string& path);
    std::string HandlePwdCommand();
    std::string HandleEnvCommand();
};

} // namespace gui
} // namespace esp32_ide

#endif // INTEGRATED_TERMINAL_H
