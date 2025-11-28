#ifndef ESP32_IDE_GUI_WIRED_FRAMEWORK_H
#define ESP32_IDE_GUI_WIRED_FRAMEWORK_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <mutex>

namespace esp32_ide {

// Forward declarations for backend components
class TextEditor;
class SyntaxHighlighter;
class FileManager;
class AIAssistant;
class ESP32Compiler;
class SerialMonitor;
class VMEmulator;
class BackendFramework;

namespace gui {

// Forward declarations for GUI components
class Widget;
class Panel;
class PanelLayout;
class DeviceLibrary;
class IntegratedTerminal;
class EnhancedGuiWindow;

/**
 * @brief Action types that can be triggered from the GUI
 */
enum class GuiAction {
    // File actions
    FILE_NEW,
    FILE_OPEN,
    FILE_SAVE,
    FILE_SAVE_AS,
    FILE_CLOSE,
    
    // Edit actions
    EDIT_UNDO,
    EDIT_REDO,
    EDIT_CUT,
    EDIT_COPY,
    EDIT_PASTE,
    EDIT_SELECT_ALL,
    EDIT_FIND,
    EDIT_REPLACE,
    
    // Build actions
    BUILD_VERIFY,
    BUILD_UPLOAD,
    BUILD_UPLOAD_PROGRAMMER,
    BUILD_STOP,
    
    // Tools actions
    TOOLS_SERIAL_MONITOR,
    TOOLS_SERIAL_PLOTTER,
    TOOLS_BOARD_MANAGER,
    TOOLS_LIBRARY_MANAGER,
    TOOLS_AUTO_FORMAT,
    
    // Emulator actions
    EMULATOR_START,
    EMULATOR_STOP,
    EMULATOR_RESET,
    EMULATOR_STEP,
    
    // Panel actions
    PANEL_TOGGLE_CONSOLE,
    PANEL_TOGGLE_TERMINAL,
    PANEL_TOGGLE_FILES,
    PANEL_TOGGLE_DEVICES,
    PANEL_TOGGLE_PREVIEW,
    
    // Device library actions
    DEVICE_ADD,
    DEVICE_REMOVE,
    DEVICE_CONFIGURE,
    DEVICE_UPLOAD_CONFIG,
    DEVICE_DOWNLOAD_CONFIG,
    
    // Help actions
    HELP_GETTING_STARTED,
    HELP_REFERENCE,
    HELP_ABOUT,
    
    // Custom action
    CUSTOM
};

/**
 * @brief GUI event data structure
 */
struct GuiEvent {
    GuiAction action;
    std::string source;         // Widget or panel ID that triggered the event
    std::string data;           // Additional data (e.g., filename, device ID)
    std::map<std::string, std::string> params;  // Additional parameters
    
    GuiEvent() : action(GuiAction::CUSTOM) {}
    GuiEvent(GuiAction act, const std::string& src = "", const std::string& d = "")
        : action(act), source(src), data(d) {}
};

/**
 * @brief GUI state update notification
 */
struct GuiStateUpdate {
    enum class Type {
        EDITOR_CONTENT,
        CONSOLE_MESSAGE,
        STATUS_MESSAGE,
        FILE_LIST,
        DEVICE_LIST,
        COMPILATION_STATUS,
        UPLOAD_STATUS,
        SERIAL_DATA,
        EMULATOR_STATE,
        PANEL_VISIBILITY
    };
    
    Type type;
    std::string data;
    std::map<std::string, std::string> params;
    
    GuiStateUpdate(Type t, const std::string& d = "") : type(t), data(d) {}
};

/**
 * @brief Callback types for GUI-backend communication
 */
using ActionCallback = std::function<void(const GuiEvent&)>;
using StateUpdateCallback = std::function<void(const GuiStateUpdate&)>;
using WidgetBindingCallback = std::function<void()>;

/**
 * @brief Widget binding configuration
 */
struct WidgetBinding {
    std::string widget_id;
    GuiAction action;
    std::string action_data;
    WidgetBindingCallback callback;
    bool enabled;
    
    WidgetBinding() : action(GuiAction::CUSTOM), enabled(true) {}
};

/**
 * @brief Frontend interface abstraction
 * 
 * This interface defines what the GUI framework expects from a frontend implementation.
 * It allows for different frontend implementations (native GUI, web, terminal) to be
 * plugged in without changing the core wiring logic.
 */
class IFrontendInterface {
public:
    virtual ~IFrontendInterface() = default;
    
    // Lifecycle
    virtual bool Initialize(int width, int height) = 0;
    virtual void Shutdown() = 0;
    virtual void ProcessEvents() = 0;
    virtual void Render() = 0;
    virtual bool IsRunning() const = 0;
    
    // UI Updates
    virtual void SetEditorContent(const std::string& content) = 0;
    virtual void AddConsoleMessage(const std::string& message, const std::string& type = "info") = 0;
    virtual void SetStatusMessage(const std::string& message) = 0;
    virtual void UpdateFileList(const std::vector<std::string>& files) = 0;
    virtual void UpdateDeviceList(const std::vector<std::string>& devices) = 0;
    
    // Panel management
    virtual void ShowPanel(const std::string& panel_id) = 0;
    virtual void HidePanel(const std::string& panel_id) = 0;
    virtual void TogglePanel(const std::string& panel_id) = 0;
    
    // Widget state
    virtual void SetWidgetEnabled(const std::string& widget_id, bool enabled) = 0;
    virtual void SetWidgetVisible(const std::string& widget_id, bool visible) = 0;
    
    // Dialogs
    virtual std::string ShowOpenFileDialog(const std::string& title, const std::string& filter) = 0;
    virtual std::string ShowSaveFileDialog(const std::string& title, const std::string& filter) = 0;
    virtual bool ShowConfirmDialog(const std::string& title, const std::string& message) = 0;
    virtual void ShowMessageDialog(const std::string& title, const std::string& message) = 0;
    
    // Progress indication
    // @param progress: Value from 0.0 to 1.0 for determinate progress, 
    //                  or PROGRESS_INDETERMINATE (-1.0f) for indeterminate progress
    static constexpr float PROGRESS_INDETERMINATE = -1.0f;
    virtual void ShowProgress(const std::string& message, float progress = PROGRESS_INDETERMINATE) = 0;
    virtual void HideProgress() = 0;
    
    // Event binding
    virtual void RegisterActionCallback(ActionCallback callback) = 0;
};

/**
 * @brief Backend interface abstraction
 * 
 * This interface defines what the GUI framework expects from the backend.
 * It abstracts the backend operations to allow for testing and alternative implementations.
 */
class IBackendInterface {
public:
    virtual ~IBackendInterface() = default;
    
    // File operations
    virtual bool NewFile(const std::string& filename = "") = 0;
    virtual bool OpenFile(const std::string& filename) = 0;
    virtual bool SaveFile() = 0;
    virtual bool SaveFileAs(const std::string& filename) = 0;
    virtual bool CloseFile() = 0;
    virtual std::string GetCurrentFileName() const = 0;
    virtual std::string GetEditorContent() const = 0;
    virtual void SetEditorContent(const std::string& content) = 0;
    
    // Build operations
    virtual bool Verify() = 0;
    virtual bool Upload() = 0;
    virtual void StopBuild() = 0;
    virtual bool IsBuilding() const = 0;
    
    // Serial operations
    virtual bool OpenSerialMonitor() = 0;
    virtual void CloseSerialMonitor() = 0;
    virtual bool IsSerialOpen() const = 0;
    virtual void SendSerialData(const std::string& data) = 0;
    
    // Emulator operations
    virtual bool StartEmulator() = 0;
    virtual void StopEmulator() = 0;
    virtual bool IsEmulatorRunning() const = 0;
    
    // Device operations
    virtual std::vector<std::string> GetDeviceList() const = 0;
    virtual bool AddDevice(const std::string& device_id, const std::string& instance_id) = 0;
    virtual bool RemoveDevice(const std::string& instance_id) = 0;
    virtual bool ConfigureDevice(const std::string& instance_id, const std::map<std::string, std::string>& params) = 0;
    
    // File list
    virtual std::vector<std::string> GetFileList() const = 0;
    
    // Status
    virtual std::string GetStatusMessage() const = 0;
    
    // Event notification registration
    virtual void RegisterStateUpdateCallback(StateUpdateCallback callback) = 0;
};

/**
 * @brief GUI Wired Framework
 * 
 * The central framework that wires the GUI frontend to the backend.
 * It manages:
 * - Action routing from GUI widgets to backend operations
 * - State updates from backend to GUI
 * - Widget bindings and event handling
 * - Lifecycle management
 * 
 * Usage:
 *   auto framework = std::make_unique<GuiWiredFramework>();
 *   framework->SetFrontend(my_frontend);
 *   framework->SetBackend(my_backend);
 *   framework->Initialize();
 *   framework->Run();
 */
class GuiWiredFramework {
public:
    GuiWiredFramework();
    ~GuiWiredFramework();
    
    // Configuration
    void SetFrontend(std::shared_ptr<IFrontendInterface> frontend);
    void SetBackend(std::shared_ptr<IBackendInterface> backend);
    
    // Lifecycle
    bool Initialize(int width = 1280, int height = 800);
    void Run();
    void Shutdown();
    bool IsRunning() const;
    
    // Widget bindings
    void BindWidget(const std::string& widget_id, GuiAction action, const std::string& action_data = "");
    void BindWidget(const std::string& widget_id, WidgetBindingCallback callback);
    void UnbindWidget(const std::string& widget_id);
    void SetWidgetEnabled(const std::string& widget_id, bool enabled);
    
    // Action handling
    void TriggerAction(GuiAction action, const std::string& data = "");
    void TriggerAction(const GuiEvent& event);
    void RegisterActionHandler(GuiAction action, ActionCallback handler);
    
    // State updates (for custom backend integration)
    void NotifyStateUpdate(const GuiStateUpdate& update);
    
    // Panel management shortcuts
    void ToggleConsole();
    void ToggleTerminal();
    void ToggleFiles();
    void ToggleDevices();
    void TogglePreview();
    
    // Direct access (for advanced use cases)
    IFrontendInterface* GetFrontend() { return frontend_.get(); }
    IBackendInterface* GetBackend() { return backend_.get(); }
    
private:
    std::shared_ptr<IFrontendInterface> frontend_;
    std::shared_ptr<IBackendInterface> backend_;
    
    std::map<std::string, WidgetBinding> widget_bindings_;
    std::map<GuiAction, std::vector<ActionCallback>> action_handlers_;
    
    bool initialized_;
    bool running_;
    
    mutable std::recursive_mutex mutex_;
    
    // Internal handlers
    void HandleAction(const GuiEvent& event);
    void HandleStateUpdate(const GuiStateUpdate& update);
    void SetupDefaultBindings();
    void SetupDefaultHandlers();
    
    // Default action handlers
    void OnFileNew(const GuiEvent& event);
    void OnFileOpen(const GuiEvent& event);
    void OnFileSave(const GuiEvent& event);
    void OnFileSaveAs(const GuiEvent& event);
    void OnFileClose(const GuiEvent& event);
    
    void OnBuildVerify(const GuiEvent& event);
    void OnBuildUpload(const GuiEvent& event);
    void OnBuildStop(const GuiEvent& event);
    
    void OnSerialMonitor(const GuiEvent& event);
    void OnEmulatorStart(const GuiEvent& event);
    void OnEmulatorStop(const GuiEvent& event);
    
    void OnDeviceAdd(const GuiEvent& event);
    void OnDeviceRemove(const GuiEvent& event);
    void OnDeviceConfigure(const GuiEvent& event);
};

/**
 * @brief Default backend adapter that wraps BackendFramework
 * 
 * This adapter implements IBackendInterface using the existing BackendFramework.
 */
class BackendAdapter : public IBackendInterface {
public:
    BackendAdapter();
    ~BackendAdapter() override;
    
    // File operations
    bool NewFile(const std::string& filename = "") override;
    bool OpenFile(const std::string& filename) override;
    bool SaveFile() override;
    bool SaveFileAs(const std::string& filename) override;
    bool CloseFile() override;
    std::string GetCurrentFileName() const override;
    std::string GetEditorContent() const override;
    void SetEditorContent(const std::string& content) override;
    
    // Build operations
    bool Verify() override;
    bool Upload() override;
    void StopBuild() override;
    bool IsBuilding() const override;
    
    // Serial operations
    bool OpenSerialMonitor() override;
    void CloseSerialMonitor() override;
    bool IsSerialOpen() const override;
    void SendSerialData(const std::string& data) override;
    
    // Emulator operations
    bool StartEmulator() override;
    void StopEmulator() override;
    bool IsEmulatorRunning() const override;
    
    // Device operations
    std::vector<std::string> GetDeviceList() const override;
    bool AddDevice(const std::string& device_id, const std::string& instance_id) override;
    bool RemoveDevice(const std::string& instance_id) override;
    bool ConfigureDevice(const std::string& instance_id, const std::map<std::string, std::string>& params) override;
    
    // File list
    std::vector<std::string> GetFileList() const override;
    
    // Status
    std::string GetStatusMessage() const override;
    
    // Event notification
    void RegisterStateUpdateCallback(StateUpdateCallback callback) override;
    
private:
    StateUpdateCallback state_callback_;
    std::string current_file_;
};

/**
 * @brief Default frontend adapter that wraps EnhancedGuiWindow
 * 
 * This adapter implements IFrontendInterface using the existing EnhancedGuiWindow.
 */
class FrontendAdapter : public IFrontendInterface {
public:
    FrontendAdapter();
    ~FrontendAdapter() override;
    
    // Lifecycle
    bool Initialize(int width, int height) override;
    void Shutdown() override;
    void ProcessEvents() override;
    void Render() override;
    bool IsRunning() const override;
    
    // UI Updates
    void SetEditorContent(const std::string& content) override;
    void AddConsoleMessage(const std::string& message, const std::string& type = "info") override;
    void SetStatusMessage(const std::string& message) override;
    void UpdateFileList(const std::vector<std::string>& files) override;
    void UpdateDeviceList(const std::vector<std::string>& devices) override;
    
    // Panel management
    void ShowPanel(const std::string& panel_id) override;
    void HidePanel(const std::string& panel_id) override;
    void TogglePanel(const std::string& panel_id) override;
    
    // Widget state
    void SetWidgetEnabled(const std::string& widget_id, bool enabled) override;
    void SetWidgetVisible(const std::string& widget_id, bool visible) override;
    
    // Dialogs
    std::string ShowOpenFileDialog(const std::string& title, const std::string& filter) override;
    std::string ShowSaveFileDialog(const std::string& title, const std::string& filter) override;
    bool ShowConfirmDialog(const std::string& title, const std::string& message) override;
    void ShowMessageDialog(const std::string& title, const std::string& message) override;
    
    // Progress indication
    void ShowProgress(const std::string& message, float progress = -1.0f) override;
    void HideProgress() override;
    
    // Event binding
    void RegisterActionCallback(ActionCallback callback) override;
    
    // Access to underlying window (for testing and advanced use)
    EnhancedGuiWindow* GetWindow() { return window_.get(); }
    
private:
    std::unique_ptr<EnhancedGuiWindow> window_;
    ActionCallback action_callback_;
    bool running_;
    
    std::vector<std::string> console_messages_;
    std::string status_message_;
};

} // namespace gui
} // namespace esp32_ide

#endif // ESP32_IDE_GUI_WIRED_FRAMEWORK_H
