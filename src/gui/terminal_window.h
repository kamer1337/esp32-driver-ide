#ifndef TERMINAL_WINDOW_H
#define TERMINAL_WINDOW_H

#include <string>
#include <vector>
#include <memory>

namespace esp32_ide {

// Forward declarations
class TextEditor;
class FileManager;
class ESP32Compiler;
class SerialMonitor;
class SyntaxHighlighter;

namespace gui {

/**
 * @brief Terminal-based window implementation for ESP32 Driver IDE
 * 
 * Provides a pure C++ terminal interface with no external GUI dependencies.
 * Uses ANSI escape codes for colors and formatting.
 */
class TerminalWindow {
public:
    TerminalWindow();
    ~TerminalWindow();
    
    // Window lifecycle
    bool Initialize(int width = 80, int height = 24);
    void Run();
    void Shutdown();
    
    // Connect to backend components
    void SetTextEditor(TextEditor* editor);
    void SetFileManager(FileManager* file_manager);
    void SetCompiler(ESP32Compiler* compiler);
    void SetSerialMonitor(SerialMonitor* serial_monitor);
    void SetSyntaxHighlighter(SyntaxHighlighter* highlighter);
    
private:
    // Backend components (not owned)
    TextEditor* text_editor_;
    FileManager* file_manager_;
    ESP32Compiler* compiler_;
    SerialMonitor* serial_monitor_;
    SyntaxHighlighter* syntax_highlighter_;
    
    // Terminal state
    int width_;
    int height_;
    bool running_;
    
    // Current state
    enum class ViewMode {
        MAIN_MENU,
        FILE_BROWSER,
        EDITOR,
        CONSOLE,
        SERIAL_MONITOR,
        SETTINGS
    };
    ViewMode current_view_;
    std::string current_file_;
    std::vector<std::string> console_messages_;
    
    // UI rendering methods
    void RenderMainMenu();
    void RenderFileBrowser();
    void RenderEditor();
    void RenderConsole();
    void RenderSerialMonitor();
    void RenderSettings();
    void RenderStatusBar();
    
    // Input handling
    std::string GetUserInput(const std::string& prompt);
    int GetMenuChoice(int max_option);
    
    // Helper methods
    void ClearScreen();
    void PrintHeader(const std::string& title);
    void PrintSeparator();
    void AddConsoleMessage(const std::string& message);
    
    // File operations
    void NewFile();
    void OpenFile();
    void SaveFile();
    void CloseFile();
    
    // Compilation operations
    void CompileCode();
    void UploadCode();
    
    // Serial operations
    void ConnectSerial();
    void DisconnectSerial();
    void SendSerialData();
    
    // ANSI color codes
    static const char* COLOR_RESET;
    static const char* COLOR_RED;
    static const char* COLOR_GREEN;
    static const char* COLOR_YELLOW;
    static const char* COLOR_BLUE;
    static const char* COLOR_MAGENTA;
    static const char* COLOR_CYAN;
    static const char* COLOR_WHITE;
    static const char* COLOR_BOLD;
};

} // namespace gui
} // namespace esp32_ide

#endif // TERMINAL_WINDOW_H
