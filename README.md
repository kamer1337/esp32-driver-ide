# ESP32 Driver IDE (C++ Version)

A modern, native C++ IDE for ESP32 development with an integrated AI assistant and **enhanced GUI interface with optional terminal mode**.

## 📸 Screenshots

<div align="center">

### Code Editor with Syntax Highlighting
![Code Editor](screenshots/06_code_editor.png)

### AI Assistant - Context-Aware Help
![AI Assistant](screenshots/05_ai_assistant.png)

### Serial Monitor - Real-Time Communication
![Serial Monitor](screenshots/07_serial_monitor.png)

### ML Device Detection - Automatic Board Recognition
![ML Device Detection](screenshots/03_ml_device_detection.png)

### Blueprint Editor - Visual Component Layout
![Blueprint Editor](screenshots/09_blueprint_editor.png)

### Advanced Decompiler - Firmware Analysis
![Decompiler](screenshots/08_decompiler.png)

### Board Selection - Multi-Platform Support
![Board Selection](screenshots/10_board_selection.png)

</div>

## ✨ Latest Update: Pure C++ with Enhanced GUI (November 2025)

**🎨 Professional GUI Interface**: Complete reimplementation with enhanced GUI as the default interface!

**🚀 Lightweight & Fast**: Pure C++ with minimal dependencies - native GUI frameworks only!

**💻 Two Interface Options**: 
1. **Enhanced GUI** (Default): Professional graphical interface with panels, device library, and visual tools
2. **Terminal Interface** (Optional): Interactive menu system with ANSI color support

**📦 Easy Build**: Single command build with no external dependencies to install!

## 🎯 Quick Look at Key Features

### 💡 Feature Demonstrations

<details>
<summary><b>🎨 Complete IDE Demo</b> - See all features in action</summary>

![IDE Demo](screenshots/01_ide_demo.png)

Demonstrates:
- Text editor with syntax highlighting
- File manager with multiple files
- AI assistant queries and code analysis
- ESP32 compiler with error checking
- Serial monitor communication

</details>

<details>
<summary><b>🖥️ Virtual Machine Emulator</b> - Test without hardware</summary>

![VM Emulator](screenshots/02_vm_emulator.png)

Features:
- Complete ESP32 device emulation
- GPIO, PWM, and ADC simulation
- WiFi and Bluetooth simulation
- Memory management tracking
- Peripheral configuration testing

</details>

<details>
<summary><b>🎨 Enhanced GUI Features</b> - Professional development environment</summary>

![Enhanced GUI](screenshots/04_enhanced_gui.png)

Includes:
- Device library management
- Panel system with docking
- Device preview modes
- Code generation from components
- Schematic visualization

</details>

## Features

### Interface Options

#### Enhanced GUI (Default)
- **Native Platform APIs**: X11 on Linux, Win32 on Windows
- **Professional Interface**: Multi-panel layout with device library and visual tools
- **Dark Theme**: Professional appearance optimized for coding
- **Interactive**: Mouse and keyboard support
- **Advanced Features**: Blueprint editor, device previewer, schematic visualization

#### Terminal Interface (Optional Build)
- **Interactive Menu System**: Easy-to-use text-based interface
- **Color-Coded Output**: ANSI escape codes for visual feedback
- **File Browser**: Browse and open project files
- **Text Editor**: View and edit code with line numbers
- **Console Output**: View compilation and runtime messages
- **Serial Monitor**: Connect and communicate with ESP32 devices

#### Simple GUI (Optional)
- **Lightweight**: No external GUI frameworks required
- **Multi-Panel Layout**: Menu bar, toolbar, editor, console, file list

### Core Functionality


#### AI-Powered Features
- **Code Refactoring**: Automatic code optimization and readability improvements
- **Bug Detection**: Detects common ESP32 bugs before compilation with auto-fix
- **Code Templates**: Built-in templates for WiFi, sensors, LED, web servers
- **Smart Completion**: Context-aware code completion with ESP32-specific suggestions
- **ML Device Detection**: Machine learning-based automatic device type detection with pretrained neural network

#### Professional Debugging & Device Support
- **Breakpoint Support**: Set breakpoints on any line for step-through debugging
- **Variable Watch**: Monitor variables in real-time during execution
- **Memory Profiling**: Track heap usage, fragmentation, and memory leaks
- **Performance Analysis**: Get performance scores and optimization suggestions
- **Board Selection**: Support for ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-C6, ESP32-H2, and ESP32-P4

## Features

### 🔧 Code Editor
- **Syntax Highlighting**: Full C/C++ syntax highlighting optimized for ESP32/Arduino code
- **Undo/Redo**: Complete undo/redo functionality
- **Multiple Files**: Support for managing multiple source files
- **Code Search**: Find and replace functionality
- **Line Management**: Line-based operations and navigation
- **Hierarchical File Tree**: Folder-subfolder-file.ino structure

### 🤖 AI Assistant
- **Integrated Chat**: Ask questions about ESP32 APIs, debugging, and best practices
- **Code Generation**: Generate complete working code for LED, WiFi, Bluetooth, sensors, and more
- **Code Analysis**: Get suggestions and improvements for your code
- **Context-Aware**: Understands ESP32-specific development challenges
- **Error Suggestions**: Provides fixes for common errors

### ⚙️ ESP32 Development Tools
- **Compile**: Syntax checking and compilation
- **Upload**: Code uploading to ESP32 devices
- **Board Selection**: Support for 8 ESP32 variants - see [DEVICE_SUPPORT.md](DEVICE_SUPPORT.md) for details
- **Serial Monitor**: Real-time serial communication with data streaming
- **VM Emulator**: Virtual machine for testing device configurations without hardware
- **ML Device Detection**: Automatic ESP32 variant detection using pretrained neural network model

### 🧠 Machine Learning Device Detection
- **Pretrained Model**: Neural network trained for ESP32 device classification
- **Feature Extraction**: Analyzes device characteristics from serial communication
- **Multi-Variant Support**: Detects 8 ESP32 variants automatically - [DEVICE_SUPPORT.md](DEVICE_SUPPORT.md)
- **Confidence Scores**: Provides reliability metrics for each detection
- **Async Callbacks**: Non-blocking detection with callback support
- **See [ML_DEVICE_DETECTION.md](ML_DEVICE_DETECTION.md)** for detailed documentation

### 🖥️ Virtual Machine Emulator
- **Device Emulation**: Test code without physical ESP32 hardware
- **GPIO Simulation**: Pin mode, state, PWM, and ADC simulation
- **WiFi/Bluetooth**: Wireless communication simulation
- **Memory Management**: Heap, PSRAM, and fragmentation tracking
- **Configuration Testing**: Validate device configurations before deployment
- **See [VM_EMULATOR.md](VM_EMULATOR.md)** for detailed documentation

### 📁 File Management
- **Create/Open/Save Files**: Complete file management
- **Multiple Files**: Manage multiple source files
- **Project Support**: Load and save entire projects
- **File Browser**: Interactive file navigation

## Building from Source

### Prerequisites

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake g++
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies
brew install cmake
```

#### Windows
- Visual Studio 2019 or later with C++ tools
- CMake 3.15 or later

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/kamer1337/esp32-driver-ide.git
cd esp32-driver-ide

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# Run the IDE
./esp32-driver-ide  # Linux/macOS
# or
esp32-driver-ide.exe  # Windows
```

### Build Options

```bash
# Build with tests
cmake -DBUILD_TESTS=ON ..
cmake --build .

# Build in Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Architecture

The IDE is built with a modular architecture:

```
src/
├── main.cpp                    # Entry point
├── editor/
│   ├── text_editor.cpp        # Core text editing
│   └── syntax_highlighter.cpp # Syntax highlighting
├── file_manager/
│   └── file_manager.cpp       # File operations
├── ai_assistant/
│   └── ai_assistant.cpp       # AI help system
├── compiler/
│   └── esp32_compiler.cpp     # Compilation & upload
├── serial/
│   └── serial_monitor.cpp     # Serial communication
├── gui/
│   ├── main_window.cpp        # Main window
│   └── console_widget.cpp     # Console output
└── utils/
    └── string_utils.cpp       # Utility functions
```

## Components

### Text Editor
- Line-based text manipulation
- Cursor management
- Selection support
- Undo/redo stack
- Search and replace

### Syntax Highlighter
- C/C++ keyword recognition
- Arduino function highlighting
- Comment and string detection
- Preprocessor directive handling
- Token-based parsing

### File Manager
- In-memory file storage
- File state tracking
- Modified file detection
- Project management

### AI Assistant
- Context-aware responses
- ESP32-specific knowledge base
- Code analysis capabilities
- Error diagnosis

### ESP32 Compiler
- Syntax validation
- Bracket matching
- Multi-board support
- Compilation simulation
- Upload simulation

### Serial Monitor
- Port management
- Baud rate configuration
- Message handling
- Colored output

## Roadmap Completion Status

- [x] Code editor with syntax highlighting ✅
- [x] AI assistant integration ✅
- [x] File management system ✅
- [x] Compilation with syntax checking ✅
- [x] Console output ✅
- [x] Real ESP32 compilation framework ✅
- [x] Real serial monitor integration framework ✅
- [x] Library manager framework ✅
- [x] Code snippets library (easily extensible) ✅
- [x] Advanced AI features (code analysis) ✅
- [x] File tree drag-and-drop ✅ **NEW**
- [x] Tab groups/split views ✅ **NEW**
- [x] AI code generation ✅ **NEW**
- [x] Real-time collaboration ✅ **NEW**
- [x] Enhanced project templates ✅ **NEW**
- [x] Syntax error highlighting ✅ **NEW**
- [x] Enhanced autocomplete ✅ **NEW**
- [x] Integrated terminal ✅ **NEW**
- [x] Virtual machine emulator for testing ✅ **NEW**
- [x] ML device detection with pretrained model ✅ **NEW**

All roadmap items are now complete! See [NEW_FEATURES.md](NEW_FEATURES.md) for details on the latest additions.

## GUI Implementation

The IDE features a **pure C++ implementation with custom rendering** supporting:

### Interface Options
1. **Terminal Interface** (Default): Full-featured text-based interface with ANSI colors
2. **Simple GUI** (Optional): Custom lightweight rendering without external dependencies

### Advanced Features
- **Blueprint Editor**: Visual component editor for device layouts
- **Device Component Previewer**: 3D/5D visualization of connected devices
- **Scripting Engine**: Extensible scripting for automation
- **Advanced Decompiler**: High-quality pseudo-code generation from firmware

### Decompiler Sample Output

The Advanced Decompiler analyzes ESP32 firmware binaries and generates readable pseudo-code. Here are examples of what it produces:

#### Example 1: Simple GPIO Control
**Input**: Binary firmware with GPIO operations  
**Output**: Decompiled pseudo-code
```c
/*
 * Function: gpio_control_task
 * Address: 0x400d0100
 * Type: FreeRTOS Task
 */
void gpio_control_task() {
    pin = 2;
    level = 1;
    gpio_set_level(pin, level);
    delay_ms = 1000;
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
    level = 0;
    gpio_set_level(pin, level);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}
```

#### Example 2: UART Communication
**Input**: Binary firmware with UART operations  
**Output**: Decompiled pseudo-code
```c
/*
 * Function: uart_send_data
 * Address: 0x400d0200
 */
void uart_send_data() {
    uart_num = 0;
    data = "Hello ESP32";
    length = 11;
    uart_write_bytes(uart_num, data, length);
    
    // Wait for transmission
    delay_ms = 100;
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}
```

#### Example 3: WiFi Connection
**Input**: Binary firmware with WiFi initialization  
**Output**: Decompiled pseudo-code
```c
/*
 * Function: wifi_init_sta
 * Address: 0x400d0300
 */
void wifi_init_sta() {
    ssid = "MyNetwork";
    password = "MyPassword";
    
    // Initialize WiFi
    esp_wifi_init();
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    // Configure and connect
    WiFi.begin(ssid, password);
    
    // Wait for connection
    while (status != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        status = WiFi.status();
    }
}
```

#### Example 4: FreeRTOS Task with Interrupt Handler
**Input**: Binary firmware with ISR and task  
**Output**: Decompiled pseudo-code
```c
/*
 * Function: button_isr_handler
 * Address: 0x400d0400
 * Type: Interrupt Service Routine
 */
void button_isr_handler() {
    // Read GPIO state
    gpio_num = 0;
    state = gpio_get_level(gpio_num);
    
    // Set flag
    button_pressed = 1;
}

/*
 * Function: button_task
 * Address: 0x400d0450
 * Type: FreeRTOS Task
 */
void button_task() {
    while (1) {
        if (button_pressed == 1) {
            // Handle button press
            led_pin = 2;
            gpio_set_level(led_pin, 1);
            
            // Reset flag
            button_pressed = 0;
            
            // Debounce delay
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

#### Example 5: Complex Arithmetic and Control Flow
**Input**: Binary firmware with conditional logic  
**Output**: Decompiled pseudo-code
```c
/*
 * Function: sensor_processing
 * Address: 0x400d0500
 */
void sensor_processing() {
    // Read sensor value
    sensor_value = adc_read();
    
    // Apply calibration
    calibrated = sensor_value * 3.3 / 4095;
    
    // Conditional processing
    if (calibrated > 2.5) {
        threshold_exceeded = 1;
        gpio_set_level(alarm_pin, 1);
    } else {
        threshold_exceeded = 0;
        gpio_set_level(alarm_pin, 0);
    }
    
    // Store result
    last_reading = calibrated;
}
```

The decompiler automatically detects:
- ESP32 API calls (GPIO, UART, WiFi, I2C, SPI, Timer)
- FreeRTOS patterns (tasks, delays, queues, semaphores)
- Interrupt service routines
- Control flow structures (if/else, loops)
- Variable types and operations

For detailed decompiler documentation, see [DECOMPILER_GUIDE.md](DECOMPILER_GUIDE.md).

### Quick Start
```bash
# Build and run (Enhanced GUI - default)
mkdir build && cd build
cmake ..
cmake --build .
./esp32-driver-ide

# Build with Terminal UI (optional)
cmake -DUSE_TERMINAL_UI=ON ..
cmake --build .
./esp32-driver-ide

# Build with Simple GUI (optional)
cmake -DBUILD_WITH_SIMPLE_GUI=ON ..
cmake --build .
./esp32-driver-ide
```

## Architecture

The system architecture supports multiple rendering backends:

### Platform Options

1. **Windows**: Win32 API or Windows Forms
2. **macOS**: Cocoa/AppKit framework
3. **Linux**: GTK+3 (already configured in CMake)
4. **Cross-platform**: Qt or wxWidgets

### Adding a GUI

The architecture is designed for easy GUI integration. The `MainWindow` class provides all necessary callbacks and state management. To add a GUI:

1. Implement platform-specific window creation
2. Connect GUI events to MainWindow methods
3. Update GUI elements using the provided callbacks

Example integration points:
```cpp
main_window->OnNewFile();      // Create new file
main_window->OnCompile();      // Compile code
main_window->OnSendChatMessage(msg);  // Query AI
```

## Usage Example

```cpp
#include "gui/main_window.h"

int main() {
    auto window = std::make_unique<esp32_ide::gui::MainWindow>();
    window->Initialize();
    return window->Run();
}
```

## API Examples

### Using the Text Editor
```cpp
auto editor = std::make_unique<TextEditor>();
editor->SetText("void setup() {}");
editor->InsertText("\nvoid loop() {}", editor->GetCursorPosition());
std::string code = editor->GetText();
```

### Using the AI Assistant
```cpp
auto assistant = std::make_unique<AIAssistant>();
std::string response = assistant->Query("How do I use GPIO pins?");
std::string analysis = assistant->AnalyzeCode(code);
```

### Compiling Code
```cpp
auto compiler = std::make_unique<ESP32Compiler>();
compiler->SetBoard(ESP32Compiler::BoardType::ESP32);
auto result = compiler->Compile(code, board);
if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
    compiler->Upload(board);
}
```

## Technical Details

### Technologies Used
- **Language**: C++17
- **Build System**: CMake
- **Standard Library**: STL (no external dependencies for core)
- **GUI Options**: 
  - Terminal UI (ANSI escape codes)
  - Simple GUI (X11 for Linux, Win32 for Windows, Cocoa for macOS)

### Design Patterns
- **MVC Architecture**: Separation of model, view, and controller
- **Observer Pattern**: Callbacks for event handling
- **Factory Pattern**: Object creation and management
- **Strategy Pattern**: Different compilation strategies per board

### No External Dependencies
The IDE has **zero external dependencies** beyond the standard C++ library. Platform GUI libraries are system-provided:
- Linux Terminal UI: None required
- Linux Simple GUI: X11 (libx11-dev package)
- Windows: Win32 API (built into Windows)
- macOS Terminal UI: None required
- macOS Simple GUI: Cocoa (built into macOS, planned)

## Contributing

Contributions are welcome! Areas for contribution:
- Platform-specific GUI implementations
- Additional ESP32 board support
- Enhanced AI assistant responses
- Real ESP32 toolchain integration
- Serial port communication
- Library management features

## Testing

```bash
# Build with tests enabled
cmake -DBUILD_TESTS=ON ..
cmake --build .

# Run tests
ctest
```

## License

MIT License - feel free to use this project for personal or commercial purposes.

## Acknowledgments

- Inspired by the Arduino IDE and VS Code
- Designed for the ESP32 developer community
- Built with modern C++ best practices
