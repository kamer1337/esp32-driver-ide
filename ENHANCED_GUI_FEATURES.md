# Enhanced GUI Features

## Overview

The ESP32 Driver IDE now features an **Enhanced GUI** with modular panels, integrated terminal, and comprehensive device library management. The GUI is the default interface, providing a professional development environment.

## New Features

### 1. Modular Panel System

The GUI uses a flexible panel system that supports:

- **Dockable Panels**: Panels can be docked to LEFT, RIGHT, TOP, BOTTOM, or CENTER
- **Floating Panels**: Panels can float independently
- **Resizable**: All panels support resizing with min/max constraints
- **Moveable**: Panels can be moved and repositioned
- **Auto-layout**: Automatic layout computation based on window size

#### Default Panel Layout

```
┌──────────────┬─────────────────────┬──────────────┐
│              │                     │              │
│   Files      │       Editor        │   Preview    │
│              │                     │              │
│──────────────│                     │              │
│              │                     │              │
│   Devices    │                     │              │
│              │                     │              │
└──────────────┴──────┬──────────────┴──────────────┘
                      │   Console   │   Terminal    │
                      └─────────────┴───────────────┘
```

### 2. Device Library Manager

A comprehensive device library system with:

- **Pre-loaded Devices**:
  - ESP32 boards (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
  - DHT22 Temperature/Humidity Sensor
  - Servo Motor
  - OLED SSD1306 Display
  - HC-05 Bluetooth Module

- **Device Features**:
  - Device definitions with parameters
  - Pin configurations
  - Code templates (init and loop)
  - Multiple device instances
  - Parameter validation

- **Device Operations**:
  - Add device instances
  - Edit device parameters
  - Remove devices
  - Upload configuration to ESP32
  - Download configuration from ESP32
  - Preview devices

### 3. Device Preview System

Multiple preview modes for devices:

- **Schematic**: 2D schematic diagram
- **Physical**: Physical appearance view
- **Pinout**: Pin diagram with descriptions
- **Code**: Generated code preview

### 4. Integrated Terminal

A full-featured terminal integrated directly into the GUI:

- **Terminal Commands**:
  - `devices` - List all available devices
  - `instances` - List device instances
  - `compile` - Compile current code
  - `upload` - Upload to ESP32
  - `clear` - Clear terminal
  - `help` - Show help

- **Terminal Features**:
  - Command history (up/down arrows)
  - Auto-scroll
  - Color-coded output
  - Custom commands support

### 5. Panel Types

The system includes specialized panel types:

1. **EditorPanel**: Code editor with syntax highlighting
2. **ConsolePanel**: Build output and messages
3. **FileBrowserPanel**: Project file navigation
4. **DeviceLibraryPanel**: Device browser
5. **TerminalPanel**: Integrated terminal
6. **PreviewPanel**: Device preview and visualization

## Usage

### Building with Enhanced GUI (Default)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./esp32-driver-ide
```

### Building with Terminal UI

```bash
mkdir build && cd build
cmake -DUSE_TERMINAL_UI=ON ..
cmake --build .
./esp32-driver-ide
```

### Using Device Library

```cpp
// Create device library
DeviceLibrary library;
library.Initialize();

// Get available devices
auto devices = library.GetAllDevices();

// Create device instance
DeviceInstance* sensor = library.CreateInstance("dht22", "temp_sensor");
sensor->SetParameterValue("pin", "4");

// Generate code
std::string init = sensor->GenerateInitCode();
std::string loop = sensor->GenerateLoopCode();
```

### Using Panel System

```cpp
// Create panel layout
PanelLayout layout;
layout.Initialize(1280, 800);

// Add panels
auto editor = std::make_unique<EditorPanel>("editor");
editor->SetDock(PanelDock::CENTER);
layout.AddPanel(std::move(editor));

// Show/hide panels
layout.ShowPanel("editor");
layout.HidePanel("console");

// Dock panels
layout.DockPanel("files", PanelDock::LEFT);
```

### Device Workflow Example

```cpp
// 1. Create device instances
DeviceLibrary library;
library.Initialize();

DeviceInstance* dht22 = library.CreateInstance("dht22", "sensor1");
dht22->SetParameterValue("pin", "4");

DeviceInstance* oled = library.CreateInstance("oled_ssd1306", "display1");
oled->SetParameterValue("i2c_address", "0x3C");

// 2. Validate configuration
std::string error;
if (!dht22->Validate(error)) {
    std::cerr << "Error: " << error << "\n";
}

// 3. Generate complete code
std::string setup_code;
std::string loop_code;

for (auto* instance : library.GetAllInstances()) {
    setup_code += instance->GenerateInitCode() + "\n";
    loop_code += instance->GenerateLoopCode() + "\n";
}

// 4. Preview device
DeviceLibraryPreview preview;
preview.SetInstance(dht22);
preview.SetPreviewMode(DeviceLibraryPreview::PreviewMode::PINOUT);
std::string pinout = preview.RenderPreview();
```

## Testing

Run the enhanced GUI test suite:

```bash
cd build
./esp32-driver-ide-enhanced-test
```

This tests:
- Device library management
- Device instances with parameters
- Panel system with docking
- Device preview modes
- Enhanced GUI integration
- Complete device workflow
- Code generation

## Architecture

```
src/gui/
├── enhanced_gui_window.h/cpp    # Main enhanced GUI window
├── panel_system.h/cpp           # Modular panel system
├── device_library.h/cpp         # Device library manager
├── integrated_terminal.h/cpp    # Terminal widget
├── simple_gui_window.h/cpp      # Lightweight GUI (optional)
└── terminal_window.h/cpp        # Terminal UI (optional)
```

## Customization

### Adding New Devices

```cpp
void DeviceLibrary::AddCustomDevice() {
    auto device = std::make_unique<DeviceDefinition>(
        "my_device",           // ID
        "My Custom Device",    // Name
        DeviceType::SENSOR     // Type
    );
    
    device->SetDescription("My custom device description");
    
    // Add parameters
    DeviceParameter pin_param;
    pin_param.name = "pin";
    pin_param.type = "int";
    pin_param.default_value = "5";
    pin_param.required = true;
    device->AddParameter(pin_param);
    
    // Add pins
    device->AddPin("VCC", "Power");
    device->AddPin("GND", "Ground");
    
    // Add code templates
    device->SetInitCode("// Init code with ${pin}");
    device->SetLoopCode("// Loop code");
    
    AddDevice(std::move(device));
}
```

### Creating Custom Panels

```cpp
class MyCustomPanel : public Panel {
public:
    MyCustomPanel(const std::string& id) 
        : Panel(id, "My Panel") {}
    
    std::string GetContent() const override {
        return "My custom content";
    }
    
    void OnResize(int width, int height) override {
        // Handle resize
    }
};
```

## CMake Options

- `BUILD_WITH_SIMPLE_GUI=ON` (default): Build with GUI
- `USE_TERMINAL_UI=ON`: Use terminal UI instead
- `BUILD_TESTS=ON`: Build test programs

## Future Enhancements

- Drag-and-drop device placement
- Visual connection editor
- Real-time device simulation
- Device firmware upload/download
- Custom device library import/export
- Multi-monitor support
- Saved panel layouts
- Theme customization
