#ifndef SIMPLE_GUI_WINDOW_H
#define SIMPLE_GUI_WINDOW_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace esp32_ide {

// Forward declarations
class TextEditor;
class FileManager;
class ESP32Compiler;
class SerialMonitor;
class SyntaxHighlighter;

namespace gui {

/**
 * @brief Simple cross-platform GUI window implementation
 * 
 * Provides a lightweight GUI using native platform APIs:
 * - X11 on Linux
 * - Win32 API on Windows
 * - Cocoa/AppKit on macOS
 * 
 * No external dependencies required (no ImGui, GLFW, or OpenGL).
 */
class SimpleGuiWindow {
public:
    SimpleGuiWindow();
    ~SimpleGuiWindow();
    
    // Window lifecycle
    bool Initialize(int width = 1024, int height = 768);
    void Run();
    void Shutdown();
    
    // Connect to backend components
    void SetTextEditor(TextEditor* editor);
    void SetFileManager(FileManager* file_manager);
    void SetCompiler(ESP32Compiler* compiler);
    void SetSerialMonitor(SerialMonitor* serial_monitor);
    void SetSyntaxHighlighter(SyntaxHighlighter* highlighter);
    
private:
    // Platform-specific window handle
    void* window_handle_;
    
    // Backend components (not owned)
    TextEditor* text_editor_;
    FileManager* file_manager_;
    ESP32Compiler* compiler_;
    SerialMonitor* serial_monitor_;
    SyntaxHighlighter* syntax_highlighter_;
    
    // Window state
    int width_;
    int height_;
    bool running_;
    std::string title_;
    
    // UI state
    std::string current_file_;
    std::string editor_content_;
    std::vector<std::string> console_messages_;
    std::vector<std::string> file_list_;
    
    // Platform-specific initialization
    bool InitializePlatform();
    void ShutdownPlatform();
    
    // Event loop
    void ProcessEvents();
    void Render();
    
    // Drawing primitives
    void DrawText(int x, int y, const std::string& text, uint32_t color = 0xFFFFFF);
    void DrawRect(int x, int y, int width, int height, uint32_t color = 0x808080);
    void DrawButton(int x, int y, int width, int height, const std::string& label);
    void ClearWindow(uint32_t color = 0x2B2B2B);
    
    // UI components
    void RenderMenuBar();
    void RenderToolbar();
    void RenderEditor();
    void RenderConsole();
    void RenderFileList();
    
    // Actions
    void CompileCode();
    void UploadCode();
    void SaveFile();
    void LoadFile(const std::string& filename);
    void AddConsoleMessage(const std::string& message);
    
    // Input handling
    void HandleMouseClick(int x, int y);
    void HandleKeyPress(int keycode);
};

} // namespace gui
} // namespace esp32_ide

#endif // SIMPLE_GUI_WINDOW_H
