#ifndef ESP32_IDE_ENHANCED_GUI_WINDOW_H
#define ESP32_IDE_ENHANCED_GUI_WINDOW_H

#include "gui/panel_system.h"
#include "gui/device_library.h"
#include "gui/integrated_terminal.h"
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
 * @brief Enhanced GUI window with modular panels and integrated terminal
 * 
 * Features:
 * - Modular panel system with docking/floating
 * - Integrated terminal within GUI
 * - Device library browser with preview
 * - Resizable and moveable panels
 * - Auto-layout management
 */
class EnhancedGuiWindow {
public:
    EnhancedGuiWindow();
    ~EnhancedGuiWindow();
    
    // Window lifecycle
    bool Initialize(int width = 1280, int height = 800);
    void Run();
    void Shutdown();
    
    // Connect to backend components
    void SetTextEditor(TextEditor* editor);
    void SetFileManager(FileManager* file_manager);
    void SetCompiler(ESP32Compiler* compiler);
    void SetSerialMonitor(SerialMonitor* serial_monitor);
    void SetSyntaxHighlighter(SyntaxHighlighter* highlighter);
    
    // Panel management
    void ShowPanel(const std::string& panel_id);
    void HidePanel(const std::string& panel_id);
    void TogglePanel(const std::string& panel_id);
    
    // Device library operations
    void ShowDeviceLibrary();
    void AddDeviceInstance(const std::string& device_id);
    void EditDeviceInstance(const std::string& instance_id);
    void RemoveDeviceInstance(const std::string& instance_id);
    void UploadDeviceConfiguration();
    void DownloadDeviceConfiguration();
    
    // Terminal operations
    void ShowTerminal();
    void ExecuteTerminalCommand(const std::string& command);
    void ClearTerminal();

private:
    // Platform-specific window handle
    void* window_handle_;
    
    // Backend components (not owned)
    TextEditor* text_editor_;
    FileManager* file_manager_;
    ESP32Compiler* compiler_;
    SerialMonitor* serial_monitor_;
    SyntaxHighlighter* syntax_highlighter_;
    
    // GUI components
    std::unique_ptr<PanelLayout> panel_layout_;
    std::unique_ptr<DeviceLibrary> device_library_;
    std::unique_ptr<IntegratedTerminal> terminal_;
    std::unique_ptr<DeviceLibraryPreview> device_preview_;
    
    // Window state
    int width_;
    int height_;
    bool running_;
    std::string title_;
    
    // UI state
    std::string current_file_;
    std::string selected_device_;
    std::string selected_instance_;
    
    // Platform-specific initialization
    bool InitializePlatform();
    void ShutdownPlatform();
    
    // Setup
    void SetupPanels();
    void SetupMenuBar();
    void SetupCallbacks();
    
    // Event loop
    void ProcessEvents();
    void Render();
    
    // Rendering
    void RenderMenuBar();
    void RenderToolbar();
    void RenderPanels();
    void RenderPanel(Panel* panel);
    void RenderStatusBar();
    
    // Drawing primitives
    void DrawText(int x, int y, const std::string& text, uint32_t color = 0xFFFFFF);
    void DrawRect(int x, int y, int width, int height, uint32_t color = 0x808080, bool filled = false);
    void DrawGradientRect(int x, int y, int width, int height, uint32_t color1, uint32_t color2, bool vertical = true);
    void DrawButton(int x, int y, int width, int height, const std::string& label);
    void DrawLine(int x1, int y1, int x2, int y2, uint32_t color = 0x808080);
    void ClearWindow(uint32_t color = 0x1E1E1E);
    
    // Helper functions for gradients
    uint32_t InterpolateColor(uint32_t color1, uint32_t color2, float ratio);
    
    // Actions
    void NewFile();
    void OpenFile();
    void SaveFile();
    void CompileCode();
    void UploadCode();
    
    // Device library actions
    void UpdateDeviceLibraryPanel();
    void UpdateDevicePreviewPanel();
    void ShowDeviceEditor();
    
    // Terminal actions
    void UpdateTerminalPanel();
    std::string HandleTerminalCommand(const std::string& command);
    
    // File browser actions
    void UpdateFileBrowserPanel();
    void OnFileSelected(const std::string& filename);
    
    // Editor actions
    void UpdateEditorPanel();
    
    // Console actions
    void UpdateConsolePanel();
    void AddConsoleMessage(const std::string& message, const std::string& type = "info");
    
    // Input handling
    void HandleMouseClick(int x, int y);
    void HandleMouseMove(int x, int y);
    void HandleMouseRelease(int x, int y);
    void HandleKeyPress(int keycode);
    void HandleResize(int width, int height);
    
    // Mouse state
    bool mouse_down_;
    int mouse_x_;
    int mouse_y_;
    Panel* active_panel_;
    bool dragging_panel_;
    bool resizing_panel_;
    
    // Colors with gradient support
    struct Colors {
        static constexpr uint32_t BACKGROUND = 0x1E1E1E;
        static constexpr uint32_t BACKGROUND_GRADIENT_TOP = 0x2D2D30;
        static constexpr uint32_t BACKGROUND_GRADIENT_BOTTOM = 0x1E1E1E;
        static constexpr uint32_t PANEL_BG = 0x252526;
        static constexpr uint32_t PANEL_BG_GRADIENT_TOP = 0x2D2D30;
        static constexpr uint32_t PANEL_BG_GRADIENT_BOTTOM = 0x1E1E1E;
        static constexpr uint32_t PANEL_BORDER = 0x3E3E42;
        static constexpr uint32_t PANEL_BORDER_HIGHLIGHT = 0x007ACC;
        static constexpr uint32_t TEXT = 0xCCCCCC;
        static constexpr uint32_t TEXT_DIM = 0x6A6A6A;
        static constexpr uint32_t ACCENT = 0x007ACC;
        static constexpr uint32_t ACCENT_LIGHT = 0x0098FF;
        static constexpr uint32_t ERROR = 0xF14C4C;
        static constexpr uint32_t SUCCESS = 0x89D185;
        static constexpr uint32_t WARNING = 0xCCA700;
        static constexpr uint32_t BUTTON_GRADIENT_TOP = 0x3E3E42;
        static constexpr uint32_t BUTTON_GRADIENT_BOTTOM = 0x2D2D30;
    };
};

} // namespace gui
} // namespace esp32_ide

#endif // ESP32_IDE_ENHANCED_GUI_WINDOW_H
