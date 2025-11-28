#include "gui/gui_wired_framework.h"
#include "gui/enhanced_gui_window.h"
#include "backend/backend_framework.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include "emulator/vm_emulator.h"
#include "gui/device_library.h"

#include <iostream>
#include <algorithm>

namespace esp32_ide {
namespace gui {

// Configuration constants
static constexpr size_t MAX_CONSOLE_MESSAGES = 100;

// =============================================================================
// GuiWiredFramework Implementation
// =============================================================================

GuiWiredFramework::GuiWiredFramework()
    : initialized_(false),
      running_(false) {
}

GuiWiredFramework::~GuiWiredFramework() {
    Shutdown();
}

void GuiWiredFramework::SetFrontend(std::shared_ptr<IFrontendInterface> frontend) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    frontend_ = std::move(frontend);
}

void GuiWiredFramework::SetBackend(std::shared_ptr<IBackendInterface> backend) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    backend_ = std::move(backend);
}

bool GuiWiredFramework::Initialize(int width, int height) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }
    
    // Create default adapters if not set
    if (!frontend_) {
        frontend_ = std::make_shared<FrontendAdapter>();
    }
    
    if (!backend_) {
        backend_ = std::make_shared<BackendAdapter>();
    }
    
    // Initialize frontend
    if (!frontend_->Initialize(width, height)) {
        std::cerr << "GuiWiredFramework: Failed to initialize frontend\n";
        return false;
    }
    
    // Register action callback with frontend
    frontend_->RegisterActionCallback([this](const GuiEvent& event) {
        HandleAction(event);
    });
    
    // Register state update callback with backend
    backend_->RegisterStateUpdateCallback([this](const GuiStateUpdate& update) {
        HandleStateUpdate(update);
    });
    
    // Setup default bindings and handlers
    SetupDefaultBindings();
    SetupDefaultHandlers();
    
    // Update initial UI state
    frontend_->UpdateFileList(backend_->GetFileList());
    frontend_->UpdateDeviceList(backend_->GetDeviceList());
    frontend_->SetStatusMessage(backend_->GetStatusMessage());
    
    initialized_ = true;
    running_ = true;
    
    std::cout << "GuiWiredFramework initialized successfully\n";
    return true;
}

void GuiWiredFramework::Run() {
    if (!initialized_) {
        std::cerr << "GuiWiredFramework: Not initialized\n";
        return;
    }
    
    std::cout << "GuiWiredFramework: Starting main loop\n";
    
    while (running_ && frontend_->IsRunning()) {
        frontend_->ProcessEvents();
        frontend_->Render();
    }
    
    std::cout << "GuiWiredFramework: Main loop ended\n";
}

void GuiWiredFramework::Shutdown() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    running_ = false;
    
    if (frontend_) {
        frontend_->Shutdown();
    }
    
    widget_bindings_.clear();
    action_handlers_.clear();
    
    initialized_ = false;
}

bool GuiWiredFramework::IsRunning() const {
    return running_ && initialized_;
}

void GuiWiredFramework::BindWidget(const std::string& widget_id, GuiAction action, const std::string& action_data) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    WidgetBinding binding;
    binding.widget_id = widget_id;
    binding.action = action;
    binding.action_data = action_data;
    binding.enabled = true;
    
    widget_bindings_[widget_id] = binding;
}

void GuiWiredFramework::BindWidget(const std::string& widget_id, WidgetBindingCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    WidgetBinding binding;
    binding.widget_id = widget_id;
    binding.action = GuiAction::CUSTOM;
    binding.callback = std::move(callback);
    binding.enabled = true;
    
    widget_bindings_[widget_id] = binding;
}

void GuiWiredFramework::UnbindWidget(const std::string& widget_id) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    widget_bindings_.erase(widget_id);
}

void GuiWiredFramework::SetWidgetEnabled(const std::string& widget_id, bool enabled) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    auto it = widget_bindings_.find(widget_id);
    if (it != widget_bindings_.end()) {
        it->second.enabled = enabled;
    }
    
    if (frontend_) {
        frontend_->SetWidgetEnabled(widget_id, enabled);
    }
}

void GuiWiredFramework::TriggerAction(GuiAction action, const std::string& data) {
    GuiEvent event(action, "", data);
    TriggerAction(event);
}

void GuiWiredFramework::TriggerAction(const GuiEvent& event) {
    HandleAction(event);
}

void GuiWiredFramework::RegisterActionHandler(GuiAction action, ActionCallback handler) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    action_handlers_[action].push_back(std::move(handler));
}

void GuiWiredFramework::NotifyStateUpdate(const GuiStateUpdate& update) {
    HandleStateUpdate(update);
}

void GuiWiredFramework::ToggleConsole() {
    TriggerAction(GuiAction::PANEL_TOGGLE_CONSOLE);
}

void GuiWiredFramework::ToggleTerminal() {
    TriggerAction(GuiAction::PANEL_TOGGLE_TERMINAL);
}

void GuiWiredFramework::ToggleFiles() {
    TriggerAction(GuiAction::PANEL_TOGGLE_FILES);
}

void GuiWiredFramework::ToggleDevices() {
    TriggerAction(GuiAction::PANEL_TOGGLE_DEVICES);
}

void GuiWiredFramework::TogglePreview() {
    TriggerAction(GuiAction::PANEL_TOGGLE_PREVIEW);
}

void GuiWiredFramework::HandleAction(const GuiEvent& event) {
    // First check for widget bindings
    if (!event.source.empty()) {
        auto it = widget_bindings_.find(event.source);
        if (it != widget_bindings_.end() && it->second.enabled) {
            if (it->second.callback) {
                it->second.callback();
                return;
            }
        }
    }
    
    // Then check for registered action handlers
    auto handlers_it = action_handlers_.find(event.action);
    if (handlers_it != action_handlers_.end()) {
        for (const auto& handler : handlers_it->second) {
            handler(event);
        }
    }
}

void GuiWiredFramework::HandleStateUpdate(const GuiStateUpdate& update) {
    if (!frontend_) return;
    
    switch (update.type) {
        case GuiStateUpdate::Type::EDITOR_CONTENT:
            frontend_->SetEditorContent(update.data);
            break;
            
        case GuiStateUpdate::Type::CONSOLE_MESSAGE: {
            std::string msg_type = "info";
            auto it = update.params.find("type");
            if (it != update.params.end()) {
                msg_type = it->second;
            }
            frontend_->AddConsoleMessage(update.data, msg_type);
            break;
        }
            
        case GuiStateUpdate::Type::STATUS_MESSAGE:
            frontend_->SetStatusMessage(update.data);
            break;
            
        case GuiStateUpdate::Type::FILE_LIST:
            if (backend_) {
                frontend_->UpdateFileList(backend_->GetFileList());
            }
            break;
            
        case GuiStateUpdate::Type::DEVICE_LIST:
            if (backend_) {
                frontend_->UpdateDeviceList(backend_->GetDeviceList());
            }
            break;
            
        case GuiStateUpdate::Type::COMPILATION_STATUS:
            // Update compilation status UI elements
            frontend_->SetStatusMessage("Compiling: " + update.data);
            break;
            
        case GuiStateUpdate::Type::UPLOAD_STATUS:
            frontend_->SetStatusMessage("Uploading: " + update.data);
            break;
            
        case GuiStateUpdate::Type::SERIAL_DATA:
            frontend_->AddConsoleMessage("[Serial] " + update.data, "info");
            break;
            
        case GuiStateUpdate::Type::EMULATOR_STATE:
            frontend_->SetStatusMessage("Emulator: " + update.data);
            break;
            
        case GuiStateUpdate::Type::PANEL_VISIBILITY: {
            auto vis_it = update.params.find("visible");
            if (vis_it != update.params.end()) {
                if (vis_it->second == "true") {
                    frontend_->ShowPanel(update.data);
                } else {
                    frontend_->HidePanel(update.data);
                }
            } else {
                frontend_->TogglePanel(update.data);
            }
            break;
        }
    }
}

void GuiWiredFramework::SetupDefaultBindings() {
    // File menu bindings
    BindWidget("btn_new", GuiAction::FILE_NEW);
    BindWidget("btn_open", GuiAction::FILE_OPEN);
    BindWidget("btn_save", GuiAction::FILE_SAVE);
    BindWidget("btn_save_as", GuiAction::FILE_SAVE_AS);
    BindWidget("btn_close", GuiAction::FILE_CLOSE);
    
    // Build toolbar bindings
    BindWidget("btn_verify", GuiAction::BUILD_VERIFY);
    BindWidget("btn_upload", GuiAction::BUILD_UPLOAD);
    BindWidget("btn_stop", GuiAction::BUILD_STOP);
    
    // Tools bindings
    BindWidget("btn_serial_monitor", GuiAction::TOOLS_SERIAL_MONITOR);
    
    // Emulator bindings
    BindWidget("btn_emulator_start", GuiAction::EMULATOR_START);
    BindWidget("btn_emulator_stop", GuiAction::EMULATOR_STOP);
    
    // Panel toggle bindings
    BindWidget("btn_toggle_console", GuiAction::PANEL_TOGGLE_CONSOLE);
    BindWidget("btn_toggle_terminal", GuiAction::PANEL_TOGGLE_TERMINAL);
    BindWidget("btn_toggle_files", GuiAction::PANEL_TOGGLE_FILES);
    BindWidget("btn_toggle_devices", GuiAction::PANEL_TOGGLE_DEVICES);
    BindWidget("btn_toggle_preview", GuiAction::PANEL_TOGGLE_PREVIEW);
    
    // Device library bindings
    BindWidget("btn_device_add", GuiAction::DEVICE_ADD);
    BindWidget("btn_device_remove", GuiAction::DEVICE_REMOVE);
    BindWidget("btn_device_configure", GuiAction::DEVICE_CONFIGURE);
}

void GuiWiredFramework::SetupDefaultHandlers() {
    // File actions
    RegisterActionHandler(GuiAction::FILE_NEW, [this](const GuiEvent& e) { OnFileNew(e); });
    RegisterActionHandler(GuiAction::FILE_OPEN, [this](const GuiEvent& e) { OnFileOpen(e); });
    RegisterActionHandler(GuiAction::FILE_SAVE, [this](const GuiEvent& e) { OnFileSave(e); });
    RegisterActionHandler(GuiAction::FILE_SAVE_AS, [this](const GuiEvent& e) { OnFileSaveAs(e); });
    RegisterActionHandler(GuiAction::FILE_CLOSE, [this](const GuiEvent& e) { OnFileClose(e); });
    
    // Build actions
    RegisterActionHandler(GuiAction::BUILD_VERIFY, [this](const GuiEvent& e) { OnBuildVerify(e); });
    RegisterActionHandler(GuiAction::BUILD_UPLOAD, [this](const GuiEvent& e) { OnBuildUpload(e); });
    RegisterActionHandler(GuiAction::BUILD_STOP, [this](const GuiEvent& e) { OnBuildStop(e); });
    
    // Tools actions
    RegisterActionHandler(GuiAction::TOOLS_SERIAL_MONITOR, [this](const GuiEvent& e) { OnSerialMonitor(e); });
    
    // Emulator actions
    RegisterActionHandler(GuiAction::EMULATOR_START, [this](const GuiEvent& e) { OnEmulatorStart(e); });
    RegisterActionHandler(GuiAction::EMULATOR_STOP, [this](const GuiEvent& e) { OnEmulatorStop(e); });
    
    // Device actions
    RegisterActionHandler(GuiAction::DEVICE_ADD, [this](const GuiEvent& e) { OnDeviceAdd(e); });
    RegisterActionHandler(GuiAction::DEVICE_REMOVE, [this](const GuiEvent& e) { OnDeviceRemove(e); });
    RegisterActionHandler(GuiAction::DEVICE_CONFIGURE, [this](const GuiEvent& e) { OnDeviceConfigure(e); });
    
    // Panel toggle actions
    RegisterActionHandler(GuiAction::PANEL_TOGGLE_CONSOLE, [this](const GuiEvent&) {
        if (frontend_) frontend_->TogglePanel("console");
    });
    RegisterActionHandler(GuiAction::PANEL_TOGGLE_TERMINAL, [this](const GuiEvent&) {
        if (frontend_) frontend_->TogglePanel("terminal");
    });
    RegisterActionHandler(GuiAction::PANEL_TOGGLE_FILES, [this](const GuiEvent&) {
        if (frontend_) frontend_->TogglePanel("files");
    });
    RegisterActionHandler(GuiAction::PANEL_TOGGLE_DEVICES, [this](const GuiEvent&) {
        if (frontend_) frontend_->TogglePanel("devices");
    });
    RegisterActionHandler(GuiAction::PANEL_TOGGLE_PREVIEW, [this](const GuiEvent&) {
        if (frontend_) frontend_->TogglePanel("preview");
    });
}

void GuiWiredFramework::OnFileNew(const GuiEvent& event) {
    if (!backend_) return;
    
    std::string filename = event.data.empty() ? "untitled.ino" : event.data;
    
    if (backend_->NewFile(filename)) {
        frontend_->AddConsoleMessage("Created new file: " + filename, "success");
        frontend_->UpdateFileList(backend_->GetFileList());
        frontend_->SetEditorContent(backend_->GetEditorContent());
    } else {
        frontend_->AddConsoleMessage("Failed to create file: " + filename, "error");
    }
}

void GuiWiredFramework::OnFileOpen(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    std::string filename = event.data;
    
    if (filename.empty()) {
        filename = frontend_->ShowOpenFileDialog("Open File", "*.ino;*.cpp;*.h");
    }
    
    if (filename.empty()) return;
    
    if (backend_->OpenFile(filename)) {
        frontend_->AddConsoleMessage("Opened: " + filename, "success");
        frontend_->SetEditorContent(backend_->GetEditorContent());
    } else {
        frontend_->AddConsoleMessage("Failed to open: " + filename, "error");
    }
}

void GuiWiredFramework::OnFileSave(const GuiEvent& event) {
    if (!backend_) return;
    
    if (backend_->SaveFile()) {
        frontend_->AddConsoleMessage("File saved", "success");
    } else {
        frontend_->AddConsoleMessage("Failed to save file", "error");
    }
}

void GuiWiredFramework::OnFileSaveAs(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    std::string filename = event.data;
    
    if (filename.empty()) {
        filename = frontend_->ShowSaveFileDialog("Save File As", "*.ino;*.cpp;*.h");
    }
    
    if (filename.empty()) return;
    
    if (backend_->SaveFileAs(filename)) {
        frontend_->AddConsoleMessage("Saved as: " + filename, "success");
        frontend_->UpdateFileList(backend_->GetFileList());
    } else {
        frontend_->AddConsoleMessage("Failed to save as: " + filename, "error");
    }
}

void GuiWiredFramework::OnFileClose(const GuiEvent& event) {
    if (!backend_) return;
    
    if (backend_->CloseFile()) {
        frontend_->AddConsoleMessage("File closed", "info");
        frontend_->SetEditorContent("");
    }
}

void GuiWiredFramework::OnBuildVerify(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    if (backend_->IsBuilding()) {
        frontend_->AddConsoleMessage("Build already in progress", "warning");
        return;
    }
    
    frontend_->AddConsoleMessage("=== Verification Started ===", "info");
    frontend_->ShowProgress("Compiling...", -1.0f);
    SetWidgetEnabled("btn_verify", false);
    SetWidgetEnabled("btn_upload", false);
    
    bool success = backend_->Verify();
    
    frontend_->HideProgress();
    SetWidgetEnabled("btn_verify", true);
    SetWidgetEnabled("btn_upload", true);
    
    if (success) {
        frontend_->AddConsoleMessage("Verification successful!", "success");
    } else {
        frontend_->AddConsoleMessage("Verification failed!", "error");
    }
    frontend_->AddConsoleMessage("=== Verification Finished ===", "info");
}

void GuiWiredFramework::OnBuildUpload(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    if (backend_->IsBuilding()) {
        frontend_->AddConsoleMessage("Build already in progress", "warning");
        return;
    }
    
    frontend_->AddConsoleMessage("=== Upload Started ===", "info");
    frontend_->ShowProgress("Uploading...", -1.0f);
    SetWidgetEnabled("btn_verify", false);
    SetWidgetEnabled("btn_upload", false);
    
    bool success = backend_->Upload();
    
    frontend_->HideProgress();
    SetWidgetEnabled("btn_verify", true);
    SetWidgetEnabled("btn_upload", true);
    
    if (success) {
        frontend_->AddConsoleMessage("Upload successful!", "success");
    } else {
        frontend_->AddConsoleMessage("Upload failed!", "error");
    }
    frontend_->AddConsoleMessage("=== Upload Finished ===", "info");
}

void GuiWiredFramework::OnBuildStop(const GuiEvent& event) {
    if (!backend_) return;
    
    backend_->StopBuild();
    frontend_->AddConsoleMessage("Build stopped", "warning");
    frontend_->HideProgress();
    SetWidgetEnabled("btn_verify", true);
    SetWidgetEnabled("btn_upload", true);
}

void GuiWiredFramework::OnSerialMonitor(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    if (backend_->IsSerialOpen()) {
        backend_->CloseSerialMonitor();
        frontend_->AddConsoleMessage("Serial monitor closed", "info");
    } else {
        if (backend_->OpenSerialMonitor()) {
            frontend_->AddConsoleMessage("Serial monitor opened", "success");
        } else {
            frontend_->AddConsoleMessage("Failed to open serial monitor", "error");
        }
    }
}

void GuiWiredFramework::OnEmulatorStart(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    if (backend_->IsEmulatorRunning()) {
        frontend_->AddConsoleMessage("Emulator already running", "warning");
        return;
    }
    
    if (backend_->StartEmulator()) {
        frontend_->AddConsoleMessage("Emulator started", "success");
        SetWidgetEnabled("btn_emulator_start", false);
        SetWidgetEnabled("btn_emulator_stop", true);
    } else {
        frontend_->AddConsoleMessage("Failed to start emulator", "error");
    }
}

void GuiWiredFramework::OnEmulatorStop(const GuiEvent& event) {
    if (!backend_) return;
    
    backend_->StopEmulator();
    frontend_->AddConsoleMessage("Emulator stopped", "info");
    SetWidgetEnabled("btn_emulator_start", true);
    SetWidgetEnabled("btn_emulator_stop", false);
}

void GuiWiredFramework::OnDeviceAdd(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    auto params_it = event.params.find("device_id");
    auto instance_it = event.params.find("instance_id");
    
    std::string device_id = (params_it != event.params.end()) ? params_it->second : event.data;
    std::string instance_id = (instance_it != event.params.end()) ? instance_it->second : "";
    
    if (device_id.empty()) {
        frontend_->AddConsoleMessage("No device specified", "error");
        return;
    }
    
    if (backend_->AddDevice(device_id, instance_id)) {
        frontend_->AddConsoleMessage("Added device: " + device_id, "success");
        frontend_->UpdateDeviceList(backend_->GetDeviceList());
    } else {
        frontend_->AddConsoleMessage("Failed to add device: " + device_id, "error");
    }
}

void GuiWiredFramework::OnDeviceRemove(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    std::string instance_id = event.data;
    
    if (instance_id.empty()) {
        frontend_->AddConsoleMessage("No device instance specified", "error");
        return;
    }
    
    if (backend_->RemoveDevice(instance_id)) {
        frontend_->AddConsoleMessage("Removed device: " + instance_id, "success");
        frontend_->UpdateDeviceList(backend_->GetDeviceList());
    } else {
        frontend_->AddConsoleMessage("Failed to remove device: " + instance_id, "error");
    }
}

void GuiWiredFramework::OnDeviceConfigure(const GuiEvent& event) {
    if (!backend_ || !frontend_) return;
    
    std::string instance_id = event.data;
    
    if (instance_id.empty()) {
        frontend_->AddConsoleMessage("No device instance specified", "error");
        return;
    }
    
    if (backend_->ConfigureDevice(instance_id, event.params)) {
        frontend_->AddConsoleMessage("Configured device: " + instance_id, "success");
    } else {
        frontend_->AddConsoleMessage("Failed to configure device: " + instance_id, "error");
    }
}


// =============================================================================
// BackendAdapter Implementation
// =============================================================================

BackendAdapter::BackendAdapter() {
    // Initialize using the singleton BackendFramework
    // Note: Initialize() returns bool but we can't propagate this from constructor.
    // The BackendFramework is designed to be resilient and will log errors internally.
    if (!BackendFramework::GetInstance().Initialize()) {
        std::cerr << "BackendAdapter: Warning - BackendFramework initialization may have issues\n";
    }
}

BackendAdapter::~BackendAdapter() {
    // Don't shutdown the singleton - it may be used by others
}

bool BackendAdapter::NewFile(const std::string& filename) {
    return BackendFramework::GetInstance().NewFile(filename);
}

bool BackendAdapter::OpenFile(const std::string& filename) {
    bool result = BackendFramework::GetInstance().OpenFile(filename);
    if (result) {
        current_file_ = filename;
    }
    return result;
}

bool BackendAdapter::SaveFile() {
    return BackendFramework::GetInstance().SaveFile();
}

bool BackendAdapter::SaveFileAs(const std::string& filename) {
    bool result = BackendFramework::GetInstance().SaveFileAs(filename);
    if (result) {
        current_file_ = filename;
    }
    return result;
}

bool BackendAdapter::CloseFile() {
    bool result = BackendFramework::GetInstance().CloseFile();
    if (result) {
        current_file_.clear();
    }
    return result;
}

std::string BackendAdapter::GetCurrentFileName() const {
    return current_file_;
}

std::string BackendAdapter::GetEditorContent() const {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    return editor ? editor->GetText() : "";
}

void BackendAdapter::SetEditorContent(const std::string& content) {
    auto* editor = BackendFramework::GetInstance().GetTextEditor();
    if (editor) {
        editor->SetText(content);
    }
}

bool BackendAdapter::Verify() {
    return BackendFramework::GetInstance().Verify();
}

bool BackendAdapter::Upload() {
    return BackendFramework::GetInstance().Upload();
}

void BackendAdapter::StopBuild() {
    BackendFramework::GetInstance().StopCompile();
}

bool BackendAdapter::IsBuilding() const {
    return BackendFramework::GetInstance().IsCompiling();
}

bool BackendAdapter::OpenSerialMonitor() {
    return BackendFramework::GetInstance().OpenSerialMonitor();
}

void BackendAdapter::CloseSerialMonitor() {
    BackendFramework::GetInstance().CloseSerialMonitor();
}

bool BackendAdapter::IsSerialOpen() const {
    return BackendFramework::GetInstance().IsSerialOpen();
}

void BackendAdapter::SendSerialData(const std::string& data) {
    BackendFramework::GetInstance().SendSerialData(data);
}

bool BackendAdapter::StartEmulator() {
    return BackendFramework::GetInstance().StartEmulator();
}

void BackendAdapter::StopEmulator() {
    BackendFramework::GetInstance().StopEmulator();
}

bool BackendAdapter::IsEmulatorRunning() const {
    return BackendFramework::GetInstance().IsEmulatorRunning();
}

std::vector<std::string> BackendAdapter::GetDeviceList() const {
    std::vector<std::string> devices;
    auto* device_library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (device_library) {
        for (const auto* device : device_library->GetAllDevices()) {
            devices.push_back(device->GetName() + " (" + device->GetId() + ")");
        }
    }
    return devices;
}

bool BackendAdapter::AddDevice(const std::string& device_id, const std::string& instance_id) {
    auto* device_library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (!device_library) return false;
    
    std::string inst_id = instance_id.empty() ? 
        device_id + "_" + std::to_string(device_library->GetAllInstances().size() + 1) : instance_id;
    
    return device_library->CreateInstance(device_id, inst_id) != nullptr;
}

bool BackendAdapter::RemoveDevice(const std::string& instance_id) {
    auto* device_library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (!device_library) return false;
    
    device_library->RemoveInstance(instance_id);
    return true;
}

bool BackendAdapter::ConfigureDevice(const std::string& instance_id, const std::map<std::string, std::string>& params) {
    auto* device_library = BackendFramework::GetInstance().GetDeviceLibrary();
    if (!device_library) return false;
    
    auto* instance = device_library->GetInstance(instance_id);
    if (!instance) return false;
    
    for (const auto& param : params) {
        instance->SetParameterValue(param.first, param.second);
    }
    return true;
}

std::vector<std::string> BackendAdapter::GetFileList() const {
    auto* file_manager = BackendFramework::GetInstance().GetFileManager();
    return file_manager ? file_manager->GetFileList() : std::vector<std::string>();
}

std::string BackendAdapter::GetStatusMessage() const {
    return BackendFramework::GetInstance().GetStatusMessage();
}

void BackendAdapter::RegisterStateUpdateCallback(StateUpdateCallback callback) {
    state_callback_ = std::move(callback);
    
    // Register with BackendFramework event system
    auto& framework = BackendFramework::GetInstance();
    
    // Map BackendFramework events to GuiStateUpdate
    framework.AddEventHandler(BackendFramework::EventType::STATUS_MESSAGE, 
        [this](const BackendFramework::Event& e) {
            if (state_callback_) {
                state_callback_(GuiStateUpdate(GuiStateUpdate::Type::STATUS_MESSAGE, e.message));
            }
        });
    
    framework.AddEventHandler(BackendFramework::EventType::COMPILE_SUCCESS, 
        [this](const BackendFramework::Event& e) {
            if (state_callback_) {
                GuiStateUpdate update(GuiStateUpdate::Type::CONSOLE_MESSAGE, "Compilation successful!");
                update.params["type"] = "success";
                state_callback_(update);
            }
        });
    
    framework.AddEventHandler(BackendFramework::EventType::COMPILE_ERROR, 
        [this](const BackendFramework::Event& e) {
            if (state_callback_) {
                GuiStateUpdate update(GuiStateUpdate::Type::CONSOLE_MESSAGE, e.message);
                update.params["type"] = "error";
                state_callback_(update);
            }
        });
    
    framework.AddEventHandler(BackendFramework::EventType::SERIAL_DATA_RECEIVED, 
        [this](const BackendFramework::Event& e) {
            if (state_callback_) {
                state_callback_(GuiStateUpdate(GuiStateUpdate::Type::SERIAL_DATA, e.message));
            }
        });
}


// =============================================================================
// FrontendAdapter Implementation
// =============================================================================

FrontendAdapter::FrontendAdapter()
    : running_(false) {
}

FrontendAdapter::~FrontendAdapter() {
    Shutdown();
}

bool FrontendAdapter::Initialize(int width, int height) {
    window_ = std::make_unique<EnhancedGuiWindow>();
    
    if (!window_->Initialize(width, height)) {
        std::cerr << "FrontendAdapter: Failed to initialize window\n";
        return false;
    }
    
    running_ = true;
    return true;
}

void FrontendAdapter::Shutdown() {
    running_ = false;
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void FrontendAdapter::ProcessEvents() {
    // The EnhancedGuiWindow handles events internally
}

void FrontendAdapter::Render() {
    // The EnhancedGuiWindow handles rendering internally
}

bool FrontendAdapter::IsRunning() const {
    return running_;
}

void FrontendAdapter::SetEditorContent(const std::string& content) {
    // Would update the editor panel in the window
    // For now, we store it for later retrieval
}

void FrontendAdapter::AddConsoleMessage(const std::string& message, const std::string& type) {
    console_messages_.push_back("[" + type + "] " + message);
    std::cout << "[" << type << "] " << message << "\n";
    
    // Keep only last MAX_CONSOLE_MESSAGES messages
    if (console_messages_.size() > MAX_CONSOLE_MESSAGES) {
        console_messages_.erase(console_messages_.begin());
    }
}

void FrontendAdapter::SetStatusMessage(const std::string& message) {
    status_message_ = message;
    std::cout << "Status: " << message << "\n";
}

void FrontendAdapter::UpdateFileList(const std::vector<std::string>& files) {
    if (window_) {
        // Would update file browser panel
    }
}

void FrontendAdapter::UpdateDeviceList(const std::vector<std::string>& devices) {
    if (window_) {
        // Would update device library panel
    }
}

void FrontendAdapter::ShowPanel(const std::string& panel_id) {
    if (window_) {
        window_->ShowPanel(panel_id);
    }
}

void FrontendAdapter::HidePanel(const std::string& panel_id) {
    if (window_) {
        window_->HidePanel(panel_id);
    }
}

void FrontendAdapter::TogglePanel(const std::string& panel_id) {
    if (window_) {
        window_->TogglePanel(panel_id);
    }
}

void FrontendAdapter::SetWidgetEnabled(const std::string& widget_id, bool enabled) {
    // Would enable/disable specific widget in window
}

void FrontendAdapter::SetWidgetVisible(const std::string& widget_id, bool visible) {
    // Would show/hide specific widget in window
}

std::string FrontendAdapter::ShowOpenFileDialog(const std::string& title, const std::string& filter) {
    // STUB: In a real GUI implementation, this would show a native file dialog.
    // Current implementation logs to console and returns empty string (simulating user cancel).
    // Platform-specific implementations should override this method.
    std::cout << "Open File Dialog: " << title << " (filter: " << filter << ")\n";
    return "";
}

std::string FrontendAdapter::ShowSaveFileDialog(const std::string& title, const std::string& filter) {
    // STUB: In a real GUI implementation, this would show a native file dialog.
    // Current implementation logs to console and returns empty string (simulating user cancel).
    // Platform-specific implementations should override this method.
    std::cout << "Save File Dialog: " << title << " (filter: " << filter << ")\n";
    return "";
}

bool FrontendAdapter::ShowConfirmDialog(const std::string& title, const std::string& message) {
    // STUB: In a real GUI implementation, this would show a confirmation dialog.
    // Current implementation logs to console and returns true (auto-confirm).
    // Platform-specific implementations should override this method.
    std::cout << "Confirm: " << title << " - " << message << "\n";
    return true;
}

void FrontendAdapter::ShowMessageDialog(const std::string& title, const std::string& message) {
    std::cout << "Message: " << title << " - " << message << "\n";
}

void FrontendAdapter::ShowProgress(const std::string& message, float progress) {
    if (progress < 0) {
        std::cout << "Progress: " << message << " (indeterminate)\n";
    } else {
        std::cout << "Progress: " << message << " (" << (progress * 100) << "%)\n";
    }
}

void FrontendAdapter::HideProgress() {
    // Would hide progress indicator
}

void FrontendAdapter::RegisterActionCallback(ActionCallback callback) {
    action_callback_ = std::move(callback);
}

} // namespace gui
} // namespace esp32_ide
