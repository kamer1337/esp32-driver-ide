#ifndef IMGUI_WINDOW_H
#define IMGUI_WINDOW_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

// Forward declarations
struct GLFWwindow;

namespace esp32_ide {

// Forward declarations
class TextEditor;
class FileManager;
class ESP32Compiler;
class SerialMonitor;

namespace gui {

/**
 * @brief ImGui-based window implementation for ESP32 Driver IDE
 * 
 * Provides a three-panel layout:
 * - Left: File Explorer
 * - Center: Editor, Debugger, Reverse-engineering tabs
 * - Right: Editor Properties
 * - Top: Toolbar with USB port, upload, download, debug, RE buttons
 */
class ImGuiWindow {
public:
    ImGuiWindow();
    ~ImGuiWindow();
    
    // Window lifecycle
    bool Initialize(int width = 1600, int height = 900);
    void Run();
    void Shutdown();
    
    // Connect to backend components
    void SetTextEditor(TextEditor* editor);
    void SetFileManager(FileManager* file_manager);
    void SetCompiler(ESP32Compiler* compiler);
    void SetSerialMonitor(SerialMonitor* serial_monitor);
    
private:
    // GLFW window
    GLFWwindow* window_;
    
    // Backend components (not owned)
    TextEditor* text_editor_;
    FileManager* file_manager_;
    ESP32Compiler* compiler_;
    SerialMonitor* serial_monitor_;
    
    // UI state
    bool show_file_explorer_;
    bool show_properties_panel_;
    int current_center_tab_;  // 0=Editor, 1=Debugger, 2=Reverse-engineering
    std::string current_file_;
    std::string editor_content_;
    char editor_buffer_[1024 * 1024];  // 1MB buffer for text editor
    
    // USB/Serial state
    std::vector<std::string> available_ports_;
    int selected_port_index_;
    std::string selected_port_;
    int selected_baud_rate_;
    std::vector<int> baud_rates_;
    
    // Console/output
    std::vector<std::string> console_messages_;
    bool scroll_to_bottom_;
    
    // File explorer state
    std::vector<std::string> file_list_;
    int selected_file_index_;
    
    // UI rendering methods
    void RenderMainMenuBar();
    void RenderToolbar();
    void RenderFileExplorer();
    void RenderCenterPanel();
    void RenderPropertiesPanel();
    void RenderConsole();
    
    // Tab rendering
    void RenderEditorTab();
    void RenderDebuggerTab();
    void RenderReverseEngineeringTab();
    
    // Helper methods
    void RefreshFileList();
    void RefreshPortList();
    void LoadFile(const std::string& filename);
    void SaveFile();
    void CompileCode();
    void UploadCode();
    void DebugCode();
    void ReverseEngineerCode();
    void AddConsoleMessage(const std::string& message);
    
    // ImGui setup
    void SetupImGuiStyle();
};

} // namespace gui
} // namespace esp32_ide

#endif // IMGUI_WINDOW_H
