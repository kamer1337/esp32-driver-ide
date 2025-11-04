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
class SyntaxHighlighter;

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
    void SetSyntaxHighlighter(SyntaxHighlighter* highlighter);
    
private:
    // GLFW window
    GLFWwindow* window_;
    
    // Backend components (not owned)
    TextEditor* text_editor_;
    FileManager* file_manager_;
    ESP32Compiler* compiler_;
    SerialMonitor* serial_monitor_;
    SyntaxHighlighter* syntax_highlighter_;
    
    // UI state
    bool show_file_explorer_;
    bool show_properties_panel_;
    bool show_ai_assistant_;
    int current_center_tab_;  // 0=Editor, 1=Debugger, 2=Reverse-engineering
    std::string current_file_;
    std::string editor_content_;
    static constexpr size_t EDITOR_BUFFER_SIZE = 1024 * 1024; // 1MB buffer
    char editor_buffer_[EDITOR_BUFFER_SIZE];
    
    // Multi-tab editor state
    struct EditorTab {
        std::string filename;
        std::string content;
        bool is_modified;
        char buffer[EDITOR_BUFFER_SIZE];
    };
    std::vector<EditorTab> editor_tabs_;
    int active_editor_tab_;
    
    // Cached UI values
    int cached_line_count_;
    bool line_count_dirty_;
    
    // USB/Serial state
    std::vector<std::string> available_ports_;
    int selected_port_index_;
    std::string selected_port_;
    int selected_baud_rate_;
    std::vector<int> baud_rates_;
    bool is_connected_;
    std::string connection_status_;
    bool connection_attempted_;
    
    // Console/output
    std::vector<std::string> console_messages_;
    bool scroll_to_bottom_;
    
    // File explorer state
    struct FileNode {
        std::string name;
        std::string path;
        bool is_folder;
        std::vector<FileNode> children;
    };
    FileNode root_folder_;
    std::vector<std::string> file_list_;
    int selected_file_index_;
    
    // AI Assistant state
    char ai_input_buffer_[512];
    std::vector<std::pair<std::string, std::string>> ai_chat_history_; // (user, assistant) pairs
    bool ai_scroll_to_bottom_;
    
    // Code detection markers
    static constexpr const char* CODE_MARKER_SETUP = "void setup()";
    static constexpr const char* CODE_MARKER_LOOP = "void loop()";
    static constexpr const char* CODE_MARKER_INCLUDE = "#include";
    
    // Reverse Engineering state
    struct REAnalysisResult {
        std::string architecture;    // CPU architecture detected (e.g., "Xtensa LX6")
        std::string flash_size;       // Flash memory size (e.g., "4MB")
        std::string entry_point;      // Entry point address (e.g., "0x40080000")
        int functions_detected;       // Number of functions found during analysis
        int strings_found;            // Number of strings extracted from firmware
        bool has_data;                // Whether valid analysis data is available
    };
    REAnalysisResult re_analysis_result_;      // Stores analysis results for display
    bool re_analysis_performed_;                // Tracks if analysis has been run
    bool re_disassembly_performed_;             // Tracks if disassembly has been run
    std::vector<std::string> re_disassembly_data_;  // Disassembled instruction lines
    
    // Terminal state
    std::vector<std::string> terminal_history_;     // Command history
    char terminal_input_buffer_[512];              // Input buffer for terminal
    bool terminal_scroll_to_bottom_;                // Auto-scroll terminal
    bool show_terminal_;                            // Show/hide terminal panel
    
    // Device/Board information state
    struct BoardInfo {
        std::string name;           // Board name (e.g., "ESP32-DevKit")
        std::string chip;           // Chip type (e.g., "ESP32", "ESP32-S3")
        std::string port;           // Serial port
        int flash_size_mb;          // Flash size in MB
        int ram_size_kb;            // RAM size in KB
        bool is_connected;          // Connection status
    };
    std::vector<BoardInfo> detected_boards_;        // List of detected boards
    int selected_board_index_;                       // Currently selected board
    bool show_board_list_;                          // Show/hide board list panel
    bool show_device_schematic_;                    // Show/hide device schematic
    int current_schematic_view_;                    // 0=pinout, 1=block diagram
    
    // Syntax highlighting state
    bool enable_syntax_highlighting_;               // Enable/disable syntax highlighting
    
    // UI rendering methods
    void RenderMainMenuBar();
    void RenderToolbar();
    void RenderFileExplorer();
    void RenderCenterPanel();
    void RenderPropertiesPanel();
    void RenderConsole();
    void RenderAIAssistant();
    
    // Tab rendering
    void RenderEditorTab();
    void RenderDebuggerTab();
    void RenderReverseEngineeringTab();
    void RenderTerminalPanel();
    void RenderBoardListPanel();
    void RenderDeviceSchematic();
    
    // Helper methods
    void RefreshFileList();
    void RefreshPortList();
    void LoadFile(const std::string& filename);
    void SaveFile();
    void SaveCurrentTab();
    void CloseTab(int tab_index);
    void CompileCode();
    void UploadCode();
    void DownloadFirmware();
    void ConnectToDevice();
    void DisconnectFromDevice();
    void DebugCode();
    void ReverseEngineerCode();
    void AddConsoleMessage(const std::string& message);
    void SendAIMessage(const std::string& message);
    void RenderFileNode(FileNode& node, const std::string& parent_path);
    void InsertCodeIntoEditor(const std::string& code);
    bool IsValidTabIndex(int index) const;
    bool ContainsCode(const std::string& text) const;
    void ExecuteTerminalCommand(const std::string& command);
    void RefreshBoardList();
    void RenderSyntaxHighlightedText(const std::string& code);
    
    // ImGui setup
    void SetupImGuiStyle();
};

} // namespace gui
} // namespace esp32_ide

#endif // IMGUI_WINDOW_H
