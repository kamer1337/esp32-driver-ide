#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "ai_assistant/ai_assistant.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include "gui/console_widget.h"
#include <memory>
#include <string>

namespace esp32_ide {
namespace gui {

/**
 * @brief Main window for the ESP32 Driver IDE
 * 
 * Manages the GUI and coordinates between components
 */
class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    // Window management
    bool Initialize();
    void Show();
    void Hide();
    int Run();
    void Close();
    
    // UI Actions
    void OnNewFile();
    void OnOpenFile();
    void OnSaveFile();
    void OnCloseFile();
    void OnCompile();
    void OnUpload();
    void OnFormatCode();
    
    // Chat actions
    void OnSendChatMessage(const std::string& message);
    
    // File actions
    void OnFileSelected(const std::string& filename);
    
    // Board selection
    void OnBoardChanged(ESP32Compiler::BoardType board);
    
private:
    // Components
    std::unique_ptr<TextEditor> editor_;
    std::unique_ptr<SyntaxHighlighter> highlighter_;
    std::unique_ptr<FileManager> file_manager_;
    std::unique_ptr<AIAssistant> ai_assistant_;
    std::unique_ptr<ESP32Compiler> compiler_;
    std::unique_ptr<SerialMonitor> serial_monitor_;
    std::unique_ptr<ConsoleWidget> console_;
    
    // Window state
    bool is_running_;
    std::string window_title_;
    
    // UI Update methods
    void UpdateTitle();
    void UpdateFileList();
    void UpdateEditor();
    void UpdateConsole(const std::string& message, ESP32Compiler::CompileStatus status);
    void UpdateChat(const std::string& message, bool is_user);
    
    // Helper methods
    void SetupCallbacks();
    void LoadDefaultFile();
};

} // namespace gui
} // namespace esp32_ide

#endif // MAIN_WINDOW_H
