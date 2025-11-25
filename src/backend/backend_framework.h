#ifndef ESP32_IDE_BACKEND_FRAMEWORK_H
#define ESP32_IDE_BACKEND_FRAMEWORK_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace esp32_ide {

// Forward declarations
class TextEditor;
class SyntaxHighlighter;
class FileManager;
class AIAssistant;
class ESP32Compiler;
class SerialMonitor;
class VMEmulator;

namespace gui {
class DeviceLibrary;
class IntegratedTerminal;
class ConsoleWidget;
}

namespace blueprint {
class BlueprintEditor;
}

namespace ml {
class MLDeviceDetector;
}

/**
 * @brief Backend framework that centralizes IDE component management
 * 
 * Inspired by Arduino IDE's architecture, this framework:
 * - Manages all backend components (editor, compiler, serial, etc.)
 * - Provides event-based communication between components
 * - Supports multiple UI frontends (GUI and Terminal)
 * - Handles IDE workflow (edit → compile → upload → monitor)
 */
class BackendFramework {
public:
    /**
     * @brief Event types for component communication
     */
    enum class EventType {
        // File events
        FILE_NEW,
        FILE_OPENED,
        FILE_SAVED,
        FILE_CLOSED,
        FILE_MODIFIED,
        
        // Editor events
        EDITOR_TEXT_CHANGED,
        EDITOR_CURSOR_MOVED,
        EDITOR_SELECTION_CHANGED,
        
        // Compile events
        COMPILE_STARTED,
        COMPILE_PROGRESS,
        COMPILE_SUCCESS,
        COMPILE_ERROR,
        COMPILE_WARNING,
        
        // Upload events
        UPLOAD_STARTED,
        UPLOAD_PROGRESS,
        UPLOAD_SUCCESS,
        UPLOAD_ERROR,
        
        // Serial events
        SERIAL_CONNECTED,
        SERIAL_DISCONNECTED,
        SERIAL_DATA_RECEIVED,
        SERIAL_DATA_SENT,
        
        // Board events
        BOARD_CHANGED,
        PORT_CHANGED,
        BOARD_DETECTED,
        
        // AI events
        AI_QUERY_STARTED,
        AI_RESPONSE_READY,
        AI_CODE_GENERATED,
        
        // Emulator events
        EMULATOR_STARTED,
        EMULATOR_STOPPED,
        EMULATOR_STATE_CHANGED,
        
        // Device library events
        DEVICE_ADDED,
        DEVICE_REMOVED,
        DEVICE_CONFIGURED,
        
        // General events
        STATUS_MESSAGE,
        ERROR_MESSAGE,
        WARNING_MESSAGE
    };
    
    /**
     * @brief Event data structure
     */
    struct Event {
        EventType type;
        std::string source;
        std::string message;
        std::map<std::string, std::string> data;
    };
    
    /**
     * @brief Event handler callback type
     */
    using EventHandler = std::function<void(const Event& event)>;
    
    /**
     * @brief Board configuration
     */
    struct BoardConfig {
        std::string name;
        std::string fqbn;          // Fully Qualified Board Name
        std::string port;
        int baudRate;
        std::string programmer;
        std::map<std::string, std::string> options;
    };
    
    /**
     * @brief Project configuration
     */
    struct ProjectConfig {
        std::string name;
        std::string path;
        std::string mainFile;
        BoardConfig board;
        std::vector<std::string> libraries;
        std::map<std::string, std::string> buildFlags;
    };
    
    // Singleton access
    static BackendFramework& GetInstance();
    
    // Lifecycle
    bool Initialize();
    void Shutdown();
    
    // Component access
    TextEditor* GetTextEditor() { return text_editor_.get(); }
    SyntaxHighlighter* GetSyntaxHighlighter() { return syntax_highlighter_.get(); }
    FileManager* GetFileManager() { return file_manager_.get(); }
    AIAssistant* GetAIAssistant() { return ai_assistant_.get(); }
    ESP32Compiler* GetCompiler() { return compiler_.get(); }
    SerialMonitor* GetSerialMonitor() { return serial_monitor_.get(); }
    VMEmulator* GetEmulator() { return vm_emulator_.get(); }
    gui::DeviceLibrary* GetDeviceLibrary() { return device_library_.get(); }
    gui::IntegratedTerminal* GetTerminal() { return terminal_.get(); }
    blueprint::BlueprintEditor* GetBlueprintEditor() { return blueprint_editor_.get(); }
    ml::MLDeviceDetector* GetDeviceDetector() { return device_detector_.get(); }
    
    // Event system
    void AddEventHandler(EventType type, EventHandler handler);
    void RemoveEventHandler(EventType type);
    void EmitEvent(const Event& event);
    
    // File operations (Arduino IDE style)
    bool NewFile(const std::string& filename = "");
    bool OpenFile(const std::string& filename);
    bool SaveFile();
    bool SaveFileAs(const std::string& filename);
    bool CloseFile();
    std::vector<std::string> GetRecentFiles() const;
    
    // Board operations
    void SetBoard(const BoardConfig& config);
    BoardConfig GetBoard() const { return current_board_; }
    std::vector<BoardConfig> GetAvailableBoards() const;
    std::vector<std::string> GetAvailablePorts() const;
    void RefreshPorts();
    
    // Compile operations
    bool Verify();           // Arduino IDE: Compile without upload
    bool Upload();           // Arduino IDE: Compile and upload
    bool UploadWithProgrammer();
    void StopCompile();
    bool IsCompiling() const { return is_compiling_; }
    
    // Serial operations
    bool OpenSerialMonitor();
    void CloseSerialMonitor();
    void SetSerialBaudRate(int baud);
    void SendSerialData(const std::string& data);
    bool IsSerialOpen() const;
    
    // Emulator operations
    bool StartEmulator();
    void StopEmulator();
    bool IsEmulatorRunning() const;
    
    // Project operations
    bool CreateProject(const std::string& name, const std::string& template_name = "");
    bool OpenProject(const std::string& path);
    bool SaveProject();
    bool CloseProject();
    ProjectConfig GetProjectConfig() const { return project_; }
    
    // AI operations
    std::string QueryAI(const std::string& query);
    std::string GenerateCode(const std::string& description);
    std::string AnalyzeCode();
    std::string FixBugs();
    
    // Preferences
    void SetPreference(const std::string& key, const std::string& value);
    std::string GetPreference(const std::string& key, const std::string& default_value = "") const;
    bool SavePreferences();
    bool LoadPreferences();
    
    // Status
    std::string GetStatusMessage() const { return status_message_; }
    void SetStatusMessage(const std::string& message);
    
private:
    BackendFramework();
    ~BackendFramework();
    
    // Prevent copying
    BackendFramework(const BackendFramework&) = delete;
    BackendFramework& operator=(const BackendFramework&) = delete;
    
    // Components
    std::unique_ptr<TextEditor> text_editor_;
    std::unique_ptr<SyntaxHighlighter> syntax_highlighter_;
    std::unique_ptr<FileManager> file_manager_;
    std::unique_ptr<AIAssistant> ai_assistant_;
    std::unique_ptr<ESP32Compiler> compiler_;
    std::unique_ptr<SerialMonitor> serial_monitor_;
    std::unique_ptr<VMEmulator> vm_emulator_;
    std::unique_ptr<gui::DeviceLibrary> device_library_;
    std::unique_ptr<gui::IntegratedTerminal> terminal_;
    std::unique_ptr<gui::ConsoleWidget> console_;
    std::unique_ptr<blueprint::BlueprintEditor> blueprint_editor_;
    std::unique_ptr<ml::MLDeviceDetector> device_detector_;
    
    // Event handlers
    std::map<EventType, std::vector<EventHandler>> event_handlers_;
    
    // State
    bool initialized_;
    bool is_compiling_;
    bool is_uploading_;
    std::string status_message_;
    std::string current_file_;
    
    // Configuration
    BoardConfig current_board_;
    ProjectConfig project_;
    std::map<std::string, std::string> preferences_;
    std::vector<std::string> recent_files_;
    
    // Helper methods
    void InitializeDefaultBoard();
    void LoadRecentFiles();
    void SaveRecentFiles();
    void AddToRecentFiles(const std::string& filename);
};

/**
 * @brief Arduino IDE-style menu actions
 * 
 * These mirror the menu structure of the Arduino IDE
 */
namespace ArduinoActions {
    // File menu
    void New();
    void Open();
    void OpenRecent(const std::string& filename);
    void Save();
    void SaveAs();
    void Close();
    void Quit();
    
    // Edit menu
    void Undo();
    void Redo();
    void Cut();
    void Copy();
    void Paste();
    void SelectAll();
    void Find();
    void FindAndReplace();
    void GoToLine(int line);
    
    // Sketch menu
    void Verify();
    void Upload();
    void UploadWithProgrammer();
    void ExportCompiledBinary();
    void ShowSketchFolder();
    void IncludeLibrary(const std::string& library);
    void AddFile();
    
    // Tools menu
    void AutoFormat();
    void ArchiveSketch();
    void FixEncoding();
    void SerialMonitor();
    void SerialPlotter();
    void ManageLibraries();
    void BoardManager();
    void GetBoardInfo();
    
    // Help menu
    void GettingStarted();
    void Reference();
    void FindInReference();
    void About();
}

} // namespace esp32_ide

#endif // ESP32_IDE_BACKEND_FRAMEWORK_H
