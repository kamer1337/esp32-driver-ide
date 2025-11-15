#include "gui/enhanced_gui_window.h"
#include "gui/device_library.h"
#include "gui/panel_system.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include <iostream>
#include <cassert>

using namespace esp32_ide;
using namespace esp32_ide::gui;

void TestDeviceLibrary() {
    std::cout << "=== Testing Device Library ===\n";
    
    DeviceLibrary library;
    library.Initialize();
    
    // Test device loading
    auto devices = library.GetAllDevices();
    std::cout << "Loaded " << devices.size() << " devices\n";
    assert(devices.size() > 0);
    
    // Test device by type
    auto sensors = library.GetDevicesByType(DeviceType::SENSOR);
    std::cout << "Found " << sensors.size() << " sensors\n";
    
    // Test device creation
    const DeviceDefinition* dht22 = library.GetDevice("dht22");
    assert(dht22 != nullptr);
    std::cout << "DHT22 device: " << dht22->GetName() << "\n";
    std::cout << "  Description: " << dht22->GetDescription() << "\n";
    std::cout << "  Parameters: " << dht22->GetParameters().size() << "\n";
    
    // Test instance creation
    DeviceInstance* instance = library.CreateInstance("dht22", "dht22_1");
    assert(instance != nullptr);
    std::cout << "Created instance: " << instance->GetInstanceId() << "\n";
    
    // Test parameter setting
    instance->SetParameterValue("pin", "15");
    std::cout << "Set pin to: " << instance->GetParameterValue("pin") << "\n";
    
    // Test code generation
    std::string init_code = instance->GenerateInitCode();
    std::cout << "Generated init code:\n" << init_code << "\n";
    
    // Test validation
    std::string error;
    bool valid = instance->Validate(error);
    std::cout << "Validation: " << (valid ? "PASS" : "FAIL") << "\n";
    if (!valid) std::cout << "  Error: " << error << "\n";
    
    std::cout << "✓ Device Library tests passed\n\n";
}

void TestPanelSystem() {
    std::cout << "=== Testing Panel System ===\n";
    
    PanelLayout layout;
    layout.Initialize(1280, 800);
    
    // Test panel creation
    auto editor_panel = std::make_unique<EditorPanel>("editor");
    editor_panel->SetDock(PanelDock::CENTER);
    layout.AddPanel(std::move(editor_panel));
    
    auto console_panel = std::make_unique<ConsolePanel>("console");
    console_panel->SetDock(PanelDock::BOTTOM);
    layout.AddPanel(std::move(console_panel));
    
    auto file_panel = std::make_unique<FileBrowserPanel>("files");
    file_panel->SetDock(PanelDock::LEFT);
    layout.AddPanel(std::move(file_panel));
    
    std::cout << "Created " << layout.GetAllPanels().size() << " panels\n";
    assert(layout.GetAllPanels().size() == 3);
    
    // Test layout computation
    layout.ComputeLayout();
    
    // Check panel bounds
    Panel* editor = layout.GetPanel("editor");
    assert(editor != nullptr);
    Rectangle bounds = editor->GetBounds();
    std::cout << "Editor panel bounds: " << bounds.x << "," << bounds.y 
              << " " << bounds.width << "x" << bounds.height << "\n";
    assert(bounds.width > 0 && bounds.height > 0);
    
    // Test visibility
    layout.ShowPanel("console");
    assert(editor->IsVisible());
    
    layout.HidePanel("console");
    Panel* console = layout.GetPanel("console");
    assert(!console->IsVisible());
    
    // Test panel content
    auto* console_panel_ptr = dynamic_cast<ConsolePanel*>(console);
    console_panel_ptr->AddLine("Test message");
    console_panel_ptr->AddLine("Another message");
    std::string content = console_panel_ptr->GetContent();
    assert(!content.empty());
    std::cout << "Console content:\n" << content << "\n";
    
    std::cout << "✓ Panel System tests passed\n\n";
}

void TestDevicePreview() {
    std::cout << "=== Testing Device Preview ===\n";
    
    DeviceLibrary library;
    library.Initialize();
    
    DeviceLibraryPreview preview;
    
    // Test device preview
    const DeviceDefinition* esp32 = library.GetDevice("esp32_dev");
    assert(esp32 != nullptr);
    
    preview.SetDevice(esp32);
    
    // Test different preview modes
    preview.SetPreviewMode(DeviceLibraryPreview::PreviewMode::SCHEMATIC);
    std::string schematic = preview.RenderPreview();
    std::cout << "Schematic preview:\n" << schematic << "\n";
    assert(!schematic.empty());
    
    preview.SetPreviewMode(DeviceLibraryPreview::PreviewMode::PINOUT);
    std::string pinout = preview.RenderPreview();
    std::cout << "Pinout preview:\n" << pinout << "\n";
    
    // Test device info
    std::string info = preview.GetDeviceInfo();
    std::cout << "Device info:\n" << info << "\n";
    assert(info.find("ESP32") != std::string::npos);
    
    std::cout << "✓ Device Preview tests passed\n\n";
}

void TestEnhancedGuiIntegration() {
    std::cout << "=== Testing Enhanced GUI Integration ===\n";
    
    // Create backend components
    auto text_editor = std::make_unique<TextEditor>();
    auto file_manager = std::make_unique<FileManager>();
    auto compiler = std::make_unique<ESP32Compiler>();
    auto serial_monitor = std::make_unique<SerialMonitor>();
    
    // Create enhanced GUI window
    EnhancedGuiWindow window;
    
    // Initialize with components
    bool init = window.Initialize(1280, 800);
    assert(init);
    std::cout << "GUI window initialized\n";
    
    window.SetTextEditor(text_editor.get());
    window.SetFileManager(file_manager.get());
    window.SetCompiler(compiler.get());
    window.SetSerialMonitor(serial_monitor.get());
    
    // Test device library operations
    window.ShowDeviceLibrary();
    window.AddDeviceInstance("dht22");
    window.AddDeviceInstance("servo");
    
    std::cout << "Added 2 device instances\n";
    
    // Test terminal
    window.ShowTerminal();
    window.ExecuteTerminalCommand("devices");
    window.ExecuteTerminalCommand("instances");
    
    std::cout << "Executed terminal commands\n";
    
    // Test panel operations
    window.ShowPanel("preview");
    window.ShowPanel("terminal");
    window.TogglePanel("console");
    
    std::cout << "Panel operations completed\n";
    
    std::cout << "✓ Enhanced GUI Integration tests passed\n\n";
}

void TestCompleteWorkflow() {
    std::cout << "=== Testing Complete Workflow ===\n";
    
    // 1. Setup
    DeviceLibrary library;
    library.Initialize();
    
    // 2. Create device instances
    DeviceInstance* dht22 = library.CreateInstance("dht22", "temp_sensor");
    assert(dht22 != nullptr);
    dht22->SetParameterValue("pin", "4");
    
    DeviceInstance* oled = library.CreateInstance("oled_ssd1306", "display");
    assert(oled != nullptr);
    oled->SetParameterValue("i2c_address", "0x3C");
    
    std::cout << "Created device instances:\n";
    std::cout << "  - " << dht22->GetInstanceId() << "\n";
    std::cout << "  - " << oled->GetInstanceId() << "\n";
    
    // 3. Validate all instances
    std::string error;
    for (auto* instance : library.GetAllInstances()) {
        bool valid = instance->Validate(error);
        if (!valid) {
            std::cout << "Validation error for " << instance->GetInstanceId() << ": " << error << "\n";
            assert(false);
        }
    }
    std::cout << "All instances validated\n";
    
    // 4. Generate code
    std::string full_code = "// Generated ESP32 Code\n\n";
    
    // Add all init code
    full_code += "// Setup function\nvoid setup() {\n";
    for (auto* instance : library.GetAllInstances()) {
        full_code += "  " + instance->GenerateInitCode() + "\n";
    }
    full_code += "}\n\n";
    
    // Add all loop code
    full_code += "// Loop function\nvoid loop() {\n";
    for (auto* instance : library.GetAllInstances()) {
        full_code += "  " + instance->GenerateLoopCode() + "\n";
    }
    full_code += "}\n";
    
    std::cout << "Generated code:\n" << full_code << "\n";
    
    // 5. Preview devices
    DeviceLibraryPreview preview;
    preview.SetInstance(dht22);
    std::string preview_text = preview.RenderPreview();
    std::cout << "Device preview:\n" << preview_text << "\n";
    
    std::cout << "✓ Complete Workflow test passed\n\n";
}

int main() {
    std::cout << "ESP32 Driver IDE - Enhanced GUI Feature Tests\n";
    std::cout << "==============================================\n\n";
    
    try {
        TestDeviceLibrary();
        TestPanelSystem();
        TestDevicePreview();
        TestEnhancedGuiIntegration();
        TestCompleteWorkflow();
        
        std::cout << "\n==============================================\n";
        std::cout << "✓ All tests passed successfully!\n";
        std::cout << "\nFeatures tested:\n";
        std::cout << "  • Device library management\n";
        std::cout << "  • Device instances with parameters\n";
        std::cout << "  • Panel system with docking\n";
        std::cout << "  • Device preview modes\n";
        std::cout << "  • Enhanced GUI integration\n";
        std::cout << "  • Complete device workflow\n";
        std::cout << "  • Code generation\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
