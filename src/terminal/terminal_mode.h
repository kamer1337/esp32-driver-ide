#ifndef ESP32_IDE_TERMINAL_MODE_H
#define ESP32_IDE_TERMINAL_MODE_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace esp32_ide {

/**
 * @brief Terminal-based mode for the ESP32 Driver IDE
 * 
 * Provides a complete CLI interface for all IDE functionality,
 * as an alternative to the GUI mode. Inspired by Arduino CLI.
 */
class TerminalModeApp {
public:
    /**
     * @brief Command structure
     */
    struct Command {
        std::string name;
        std::string description;
        std::string usage;
        std::vector<std::string> aliases;
        std::function<int(const std::vector<std::string>&)> handler;
    };
    
    TerminalModeApp();
    ~TerminalModeApp();
    
    // Application lifecycle
    int Run(int argc, char* argv[]);
    void Quit();
    
    // Interactive mode
    int RunInteractive();
    void ProcessCommand(const std::string& input);
    
    // Command registration
    void RegisterCommand(const Command& command);
    void RegisterBuiltInCommands();
    
    // Help system
    void PrintHelp();
    void PrintCommandHelp(const std::string& command);
    void PrintVersion();
    void PrintWelcome();
    
    // Output formatting
    void Print(const std::string& message);
    void PrintSuccess(const std::string& message);
    void PrintError(const std::string& message);
    void PrintWarning(const std::string& message);
    void PrintInfo(const std::string& message);
    void PrintTable(const std::vector<std::vector<std::string>>& rows, 
                   const std::vector<std::string>& headers);
    
    // Progress display
    void ShowProgress(const std::string& message, float progress);
    void HideProgress();
    
    // User input
    std::string Prompt(const std::string& message);
    bool Confirm(const std::string& message);
    int Select(const std::string& message, const std::vector<std::string>& options);
    
private:
    bool running_;
    bool interactive_mode_;
    bool color_output_;
    std::map<std::string, Command> commands_;
    std::map<std::string, std::string> aliases_;
    std::string prompt_;
    
    // Command handlers
    int HandleHelp(const std::vector<std::string>& args);
    int HandleVersion(const std::vector<std::string>& args);
    int HandleQuit(const std::vector<std::string>& args);
    
    // File commands
    int HandleNew(const std::vector<std::string>& args);
    int HandleOpen(const std::vector<std::string>& args);
    int HandleSave(const std::vector<std::string>& args);
    int HandleClose(const std::vector<std::string>& args);
    int HandleList(const std::vector<std::string>& args);
    int HandleCat(const std::vector<std::string>& args);
    int HandleEdit(const std::vector<std::string>& args);
    
    // Board commands
    int HandleBoard(const std::vector<std::string>& args);
    int HandlePort(const std::vector<std::string>& args);
    int HandleBoardList(const std::vector<std::string>& args);
    int HandlePortList(const std::vector<std::string>& args);
    
    // Compile commands
    int HandleVerify(const std::vector<std::string>& args);
    int HandleUpload(const std::vector<std::string>& args);
    int HandleCompile(const std::vector<std::string>& args);
    
    // Serial commands
    int HandleMonitor(const std::vector<std::string>& args);
    int HandleSend(const std::vector<std::string>& args);
    
    // Emulator commands
    int HandleEmulator(const std::vector<std::string>& args);
    
    // AI commands
    int HandleAsk(const std::vector<std::string>& args);
    int HandleGenerate(const std::vector<std::string>& args);
    int HandleAnalyze(const std::vector<std::string>& args);
    int HandleFix(const std::vector<std::string>& args);
    
    // Device library commands
    int HandleDevices(const std::vector<std::string>& args);
    int HandleAddDevice(const std::vector<std::string>& args);
    
    // Settings commands
    int HandleConfig(const std::vector<std::string>& args);
    int HandleSet(const std::vector<std::string>& args);
    int HandleGet(const std::vector<std::string>& args);
    
    // Helper methods
    std::vector<std::string> ParseArguments(const std::string& input);
    std::string GetColorCode(const std::string& color);
    std::string ResetColor();
    void DetectColorSupport();
};

/**
 * @brief Entry point for terminal mode
 */
int TerminalMain(int argc, char* argv[]);

} // namespace esp32_ide

#endif // ESP32_IDE_TERMINAL_MODE_H
