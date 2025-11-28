#include <iostream>
#include <cassert>
#include <memory>
#include <set>
#include "gui/gui_wired_framework.h"

using namespace esp32_ide;
using namespace esp32_ide::gui;

/**
 * @brief Mock frontend for testing
 */
class MockFrontend : public IFrontendInterface {
public:
    MockFrontend() : running_(true), initialized_(false) {}
    
    bool Initialize(int width, int height) override {
        initialized_ = true;
        std::cout << "  MockFrontend initialized: " << width << "x" << height << "\n";
        return true;
    }
    
    void Shutdown() override {
        running_ = false;
        initialized_ = false;
    }
    
    void ProcessEvents() override {}
    void Render() override {}
    bool IsRunning() const override { return running_; }
    
    void SetEditorContent(const std::string& content) override {
        editor_content_ = content;
    }
    
    void AddConsoleMessage(const std::string& message, const std::string& type) override {
        console_messages_.push_back("[" + type + "] " + message);
    }
    
    void SetStatusMessage(const std::string& message) override {
        status_message_ = message;
    }
    
    void UpdateFileList(const std::vector<std::string>& files) override {
        file_list_ = files;
    }
    
    void UpdateDeviceList(const std::vector<std::string>& devices) override {
        device_list_ = devices;
    }
    
    void ShowPanel(const std::string& panel_id) override {
        visible_panels_.insert(panel_id);
    }
    
    void HidePanel(const std::string& panel_id) override {
        visible_panels_.erase(panel_id);
    }
    
    void TogglePanel(const std::string& panel_id) override {
        if (visible_panels_.count(panel_id)) {
            visible_panels_.erase(panel_id);
        } else {
            visible_panels_.insert(panel_id);
        }
    }
    
    void SetWidgetEnabled(const std::string& widget_id, bool enabled) override {
        widget_enabled_[widget_id] = enabled;
    }
    
    void SetWidgetVisible(const std::string& widget_id, bool visible) override {
        widget_visible_[widget_id] = visible;
    }
    
    std::string ShowOpenFileDialog(const std::string& title, const std::string& filter) override {
        return test_file_dialog_result_;
    }
    
    std::string ShowSaveFileDialog(const std::string& title, const std::string& filter) override {
        return test_file_dialog_result_;
    }
    
    bool ShowConfirmDialog(const std::string& title, const std::string& message) override {
        return test_confirm_result_;
    }
    
    void ShowMessageDialog(const std::string& title, const std::string& message) override {
        last_message_title_ = title;
        last_message_text_ = message;
    }
    
    void ShowProgress(const std::string& message, float progress) override {
        progress_visible_ = true;
        progress_message_ = message;
        progress_value_ = progress;
    }
    
    void HideProgress() override {
        progress_visible_ = false;
    }
    
    void RegisterActionCallback(ActionCallback callback) override {
        action_callback_ = callback;
    }
    
    // Test helpers
    void SimulateAction(const GuiEvent& event) {
        if (action_callback_) {
            action_callback_(event);
        }
    }
    
    void SetRunning(bool running) { running_ = running; }
    void SetFileDialogResult(const std::string& result) { test_file_dialog_result_ = result; }
    void SetConfirmResult(bool result) { test_confirm_result_ = result; }
    
    // State accessors for testing
    const std::string& GetEditorContent() const { return editor_content_; }
    const std::vector<std::string>& GetConsoleMessages() const { return console_messages_; }
    const std::string& GetStatusMessage() const { return status_message_; }
    const std::vector<std::string>& GetFileList() const { return file_list_; }
    const std::vector<std::string>& GetDeviceList() const { return device_list_; }
    bool IsPanelVisible(const std::string& id) const { return visible_panels_.count(id) > 0; }
    // Note: Returns true by default for widgets that haven't been explicitly disabled.
    // This matches the expected behavior where widgets are enabled until disabled.
    bool IsWidgetEnabled(const std::string& id) const { 
        auto it = widget_enabled_.find(id);
        return it != widget_enabled_.end() ? it->second : true;  // Default: enabled
    }
    bool IsProgressVisible() const { return progress_visible_; }
    
private:
    bool running_;
    bool initialized_;
    std::string editor_content_;
    std::vector<std::string> console_messages_;
    std::string status_message_;
    std::vector<std::string> file_list_;
    std::vector<std::string> device_list_;
    std::set<std::string> visible_panels_;
    std::map<std::string, bool> widget_enabled_;
    std::map<std::string, bool> widget_visible_;
    bool progress_visible_ = false;
    std::string progress_message_;
    float progress_value_ = 0;
    ActionCallback action_callback_;
    std::string test_file_dialog_result_;
    bool test_confirm_result_ = true;
    std::string last_message_title_;
    std::string last_message_text_;
};

/**
 * @brief Mock backend for testing
 */
class MockBackend : public IBackendInterface {
public:
    MockBackend() 
        : building_(false), serial_open_(false), emulator_running_(false) {}
    
    bool NewFile(const std::string& filename) override {
        current_file_ = filename.empty() ? "untitled.ino" : filename;
        editor_content_ = "// New file: " + current_file_;
        return true;
    }
    
    bool OpenFile(const std::string& filename) override {
        if (filename == "nonexistent.ino") return false;
        current_file_ = filename;
        editor_content_ = "// Content of: " + filename;
        return true;
    }
    
    bool SaveFile() override {
        return !current_file_.empty();
    }
    
    bool SaveFileAs(const std::string& filename) override {
        current_file_ = filename;
        return true;
    }
    
    bool CloseFile() override {
        current_file_.clear();
        editor_content_.clear();
        return true;
    }
    
    std::string GetCurrentFileName() const override { return current_file_; }
    std::string GetEditorContent() const override { return editor_content_; }
    void SetEditorContent(const std::string& content) override { editor_content_ = content; }
    
    bool Verify() override { return verify_result_; }
    bool Upload() override { return upload_result_; }
    void StopBuild() override { building_ = false; }
    bool IsBuilding() const override { return building_; }
    
    bool OpenSerialMonitor() override { serial_open_ = true; return true; }
    void CloseSerialMonitor() override { serial_open_ = false; }
    bool IsSerialOpen() const override { return serial_open_; }
    void SendSerialData(const std::string& data) override {}
    
    bool StartEmulator() override { emulator_running_ = true; return true; }
    void StopEmulator() override { emulator_running_ = false; }
    bool IsEmulatorRunning() const override { return emulator_running_; }
    
    std::vector<std::string> GetDeviceList() const override {
        return {"DHT22 (dht22)", "SSD1306 (ssd1306)"};
    }
    
    bool AddDevice(const std::string& device_id, const std::string& instance_id) override {
        return !device_id.empty();
    }
    
    bool RemoveDevice(const std::string& instance_id) override {
        return !instance_id.empty();
    }
    
    bool ConfigureDevice(const std::string& instance_id, const std::map<std::string, std::string>& params) override {
        return !instance_id.empty();
    }
    
    std::vector<std::string> GetFileList() const override {
        return {"sketch.ino", "helper.h", "config.h"};
    }
    
    std::string GetStatusMessage() const override { return "Ready"; }
    
    void RegisterStateUpdateCallback(StateUpdateCallback callback) override {
        state_callback_ = callback;
    }
    
    // Test helpers
    void SetVerifyResult(bool result) { verify_result_ = result; }
    void SetUploadResult(bool result) { upload_result_ = result; }
    void SetBuilding(bool building) { building_ = building; }
    
    void SimulateStateUpdate(const GuiStateUpdate& update) {
        if (state_callback_) {
            state_callback_(update);
        }
    }
    
private:
    std::string current_file_;
    std::string editor_content_;
    bool building_;
    bool serial_open_;
    bool emulator_running_;
    bool verify_result_ = true;
    bool upload_result_ = true;
    StateUpdateCallback state_callback_;
};


void test_framework_initialization() {
    std::cout << "Testing GuiWiredFramework initialization...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    
    bool init_result = framework.Initialize(1280, 800);
    assert(init_result);
    assert(framework.IsRunning());
    
    framework.Shutdown();
    assert(!framework.IsRunning());
    
    std::cout << "  ✓ Framework initialization tests passed\n";
}

void test_widget_bindings() {
    std::cout << "Testing widget bindings...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Test custom binding with callback
    bool callback_triggered = false;
    framework.BindWidget("custom_btn", [&callback_triggered]() {
        callback_triggered = true;
    });
    
    // Simulate click on bound widget
    GuiEvent event;
    event.source = "custom_btn";
    event.action = GuiAction::CUSTOM;
    framework.TriggerAction(event);
    
    assert(callback_triggered);
    
    // Test unbinding
    framework.UnbindWidget("custom_btn");
    callback_triggered = false;
    framework.TriggerAction(event);
    assert(!callback_triggered);
    
    framework.Shutdown();
    std::cout << "  ✓ Widget binding tests passed\n";
}

void test_file_actions() {
    std::cout << "Testing file actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Test new file
    framework.TriggerAction(GuiAction::FILE_NEW, "test.ino");
    assert(backend->GetCurrentFileName() == "test.ino");
    
    // Test save
    framework.TriggerAction(GuiAction::FILE_SAVE);
    
    // Test close
    framework.TriggerAction(GuiAction::FILE_CLOSE);
    assert(backend->GetCurrentFileName().empty());
    
    // Test open with data
    framework.TriggerAction(GuiAction::FILE_OPEN, "myfile.ino");
    assert(backend->GetCurrentFileName() == "myfile.ino");
    
    framework.Shutdown();
    std::cout << "  ✓ File action tests passed\n";
}

void test_build_actions() {
    std::cout << "Testing build actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Test verify success
    backend->SetVerifyResult(true);
    framework.TriggerAction(GuiAction::BUILD_VERIFY);
    
    // Check console messages contain success
    bool found_success = false;
    for (const auto& msg : frontend->GetConsoleMessages()) {
        if (msg.find("successful") != std::string::npos) {
            found_success = true;
            break;
        }
    }
    assert(found_success);
    
    // Test verify failure
    backend->SetVerifyResult(false);
    framework.TriggerAction(GuiAction::BUILD_VERIFY);
    
    bool found_failed = false;
    for (const auto& msg : frontend->GetConsoleMessages()) {
        if (msg.find("failed") != std::string::npos) {
            found_failed = true;
            break;
        }
    }
    assert(found_failed);
    
    framework.Shutdown();
    std::cout << "  ✓ Build action tests passed\n";
}

void test_panel_actions() {
    std::cout << "Testing panel actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Toggle console panel
    framework.ToggleConsole();
    assert(frontend->IsPanelVisible("console"));
    
    framework.ToggleConsole();
    assert(!frontend->IsPanelVisible("console"));
    
    // Toggle terminal panel
    framework.ToggleTerminal();
    assert(frontend->IsPanelVisible("terminal"));
    
    framework.Shutdown();
    std::cout << "  ✓ Panel action tests passed\n";
}

void test_emulator_actions() {
    std::cout << "Testing emulator actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Start emulator
    framework.TriggerAction(GuiAction::EMULATOR_START);
    assert(backend->IsEmulatorRunning());
    
    // Stop emulator
    framework.TriggerAction(GuiAction::EMULATOR_STOP);
    assert(!backend->IsEmulatorRunning());
    
    framework.Shutdown();
    std::cout << "  ✓ Emulator action tests passed\n";
}

void test_serial_actions() {
    std::cout << "Testing serial actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Open serial monitor
    framework.TriggerAction(GuiAction::TOOLS_SERIAL_MONITOR);
    assert(backend->IsSerialOpen());
    
    // Toggle closes it
    framework.TriggerAction(GuiAction::TOOLS_SERIAL_MONITOR);
    assert(!backend->IsSerialOpen());
    
    framework.Shutdown();
    std::cout << "  ✓ Serial action tests passed\n";
}

void test_state_updates() {
    std::cout << "Testing state updates...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Test console message update
    GuiStateUpdate update(GuiStateUpdate::Type::CONSOLE_MESSAGE, "Test message");
    update.params["type"] = "info";
    framework.NotifyStateUpdate(update);
    
    bool found_message = false;
    for (const auto& msg : frontend->GetConsoleMessages()) {
        if (msg.find("Test message") != std::string::npos) {
            found_message = true;
            break;
        }
    }
    assert(found_message);
    
    // Test status update
    framework.NotifyStateUpdate(GuiStateUpdate(GuiStateUpdate::Type::STATUS_MESSAGE, "Building..."));
    assert(frontend->GetStatusMessage().find("Building") != std::string::npos);
    
    framework.Shutdown();
    std::cout << "  ✓ State update tests passed\n";
}

void test_device_actions() {
    std::cout << "Testing device actions...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Add device
    framework.TriggerAction(GuiAction::DEVICE_ADD, "dht22");
    
    // Check success message
    bool found_success = false;
    for (const auto& msg : frontend->GetConsoleMessages()) {
        if (msg.find("Added device") != std::string::npos) {
            found_success = true;
            break;
        }
    }
    assert(found_success);
    
    // Remove device
    framework.TriggerAction(GuiAction::DEVICE_REMOVE, "dht22_1");
    
    bool found_removed = false;
    for (const auto& msg : frontend->GetConsoleMessages()) {
        if (msg.find("Removed device") != std::string::npos) {
            found_removed = true;
            break;
        }
    }
    assert(found_removed);
    
    framework.Shutdown();
    std::cout << "  ✓ Device action tests passed\n";
}

void test_action_handlers() {
    std::cout << "Testing custom action handlers...\n";
    
    auto frontend = std::make_shared<MockFrontend>();
    auto backend = std::make_shared<MockBackend>();
    
    GuiWiredFramework framework;
    framework.SetFrontend(frontend);
    framework.SetBackend(backend);
    framework.Initialize();
    
    // Register custom handler
    bool custom_handler_called = false;
    std::string received_data;
    
    framework.RegisterActionHandler(GuiAction::HELP_ABOUT, [&](const GuiEvent& e) {
        custom_handler_called = true;
        received_data = e.data;
    });
    
    // Trigger the action
    framework.TriggerAction(GuiAction::HELP_ABOUT, "test_data");
    
    assert(custom_handler_called);
    assert(received_data == "test_data");
    
    framework.Shutdown();
    std::cout << "  ✓ Custom action handler tests passed\n";
}


int main() {
    std::cout << "========================================\n";
    std::cout << "ESP32 Driver IDE - GUI Wired Framework Tests\n";
    std::cout << "========================================\n\n";
    
    try {
        test_framework_initialization();
        test_widget_bindings();
        test_file_actions();
        test_build_actions();
        test_panel_actions();
        test_emulator_actions();
        test_serial_actions();
        test_state_updates();
        test_device_actions();
        test_action_handlers();
        
        std::cout << "\n========================================\n";
        std::cout << "✓ ALL GUI WIRED FRAMEWORK TESTS PASSED!\n";
        std::cout << "========================================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
