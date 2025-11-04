# ESP32 Driver IDE (C++ Version)

A modern, native C++ IDE for ESP32 development with an integrated AI assistant and **professional ImGui-based graphical interface**.

![ESP32 Driver IDE](https://github.com/user-attachments/assets/18511046-9c87-48ff-a377-960e97afd9f3)

## ✨ Latest Update: Advanced IDE Features (November 2025)

**🎨 Professional IDE Experience**: Complete IDE features including integrated terminal, board management, device schematics, advanced syntax highlighting, and **blueprint node editor**!

**🔷 Blueprint Node Editor**: Visual node-based editor for designing ESP32 hardware connections and component interactions!

**📋 Board Management**: Scan and manage multiple ESP32 boards with detailed information and device pinout diagrams!

**🎨 Syntax Highlighting**: Beautiful color-coded syntax highlighting with split-view editor!

📖 **[Read the New Features Guide](NEW_FEATURES.md)**

### Latest Advanced Capabilities (November 2025)

#### Blueprint Node Editor (NEW!)
- **Visual Design**: Node-based interface for hardware component design
- **ESP32 Components**: Pre-built nodes for GPIO, I2C, SPI, UART, ADC, DAC, PWM, and more
- **Pin Visualization**: Color-coded pins showing input/output/bidirectional connections
- **Interactive Connections**: Click and drag to create connections between component pins
- **Code Generation**: Automatically generate Arduino/ESP32 code from visual design
- **Component Library**: LED, Button, Sensor, Motor, Display nodes and more
- **Context Menu**: Right-click to add new components to your design

#### Integrated Terminal
- **Command-Line Interface**: Built-in terminal for executing commands
- **Color-Coded Output**: Error, warning, and success message highlighting
- **Built-in Commands**: compile, upload, ports, boards, ls, help, and more
- **Command History**: Track all executed commands

#### Board Management
- **Device Detection**: Automatically scan for connected ESP32 boards
- **Board Information**: View chip type, flash size, RAM, and port details
- **Multi-Board Support**: Manage multiple boards simultaneously
- **Connection Status**: Visual indicators for connected devices

#### Device Schematic Viewer
- **Pinout Diagram**: Complete ESP32 GPIO pinout with alternate functions
- **Block Diagram**: System architecture overview
- **Floating Window**: Convenient reference while coding
- **Multiple Views**: Switch between pinout and architecture views

#### Advanced Syntax Highlighting
- **Split-View Editor**: Edit mode with live preview
- **Color-Coded Syntax**: Keywords, types, functions, strings, comments
- **Real-Time Preview**: See highlighted code as you type
- **Toggle On/Off**: Switch between plain and highlighted modes

#### Enhanced UI Panels
- **Templates Panel**: Browse and insert code templates
- **Completion Panel**: One-click function insertion
- **Board List Panel**: Manage connected devices
- **Dual Bottom Panels**: Console and Terminal side-by-side

### Previous Advanced Capabilities

#### AI-Powered Features
- **Code Refactoring**: Automatic code optimization and readability improvements
- **Bug Detection**: Detects common ESP32 bugs before compilation with auto-fix
- **Code Templates**: Built-in templates for WiFi, sensors, LED, web servers
- **Smart Completion**: Context-aware code completion with ESP32-specific suggestions

#### Professional Debugging
- **Breakpoint Support**: Set breakpoints on any line for step-through debugging
- **Variable Watch**: Monitor variables in real-time during execution
- **Memory Profiling**: Track heap usage, fragmentation, and memory leaks
- **Performance Analysis**: Get performance scores and optimization suggestions

#### Previous Features
- **Generate Code**: Ask AI to "Generate code for LED blink" and get working code instantly
- **Insert Directly**: One-click code insertion into your active editor tab
- **Live Debugging**: Start debugging to see real-time data from your connected device
- **Variable Inspection**: Monitor heap memory, WiFi status, GPIO states, and more

## ✨ ImGui Interface

The IDE features a complete ImGui-based GUI with:
- **Five-panel layout**: File/Board Explorer (left), Editor/Debugger/RE/Blueprint (center), Properties/Templates/Completion (right), AI Assistant (far right), Terminal (bottom)
- **Professional toolbar**: USB port selector, Upload, Download, Debug, and Reverse Engineering buttons
- **Multi-tab panels**: Switch between Files/Boards, Editor/Debugger/RE/Blueprint, Editor/Templates/Completion views
- **Dual bottom panels**: Console output and integrated terminal
- **Blueprint editor**: Visual node-based hardware design with component library
- **Device schematic viewer**: Floating window with pinout and block diagrams
- **Dark theme**: Professional appearance optimized for long coding sessions

📖 **[Read the complete ImGui Interface Guide](IMGUI_GUIDE.md)**

## Features

### 🔧 Code Editor
- **Syntax Highlighting**: Full C/C++ syntax highlighting optimized for ESP32/Arduino code
- **Undo/Redo**: Complete undo/redo functionality
- **Multiple Files**: Support for managing multiple source files in tabs
- **Code Search**: Find and replace functionality
- **Line Management**: Line-based operations and navigation
- **Hierarchical File Tree**: Folder-subfolder-file.ino structure
- **Breakpoint Support**: Visual breakpoint indicators on code lines
- **Code Completion**: Smart suggestions as you type

### 🤖 AI Assistant (Enhanced!)
- **Integrated Chat**: Ask questions about ESP32 APIs, debugging, and best practices
- **Code Generation**: Generate complete working code for LED, WiFi, Bluetooth, sensors, and more
- **Direct Code Insertion**: Insert generated code into editor with one click
- **Code Analysis**: Get suggestions and improvements for your code
- **AI Refactoring**: Automatic code optimization and readability improvements
- **Bug Detection**: Detects and auto-fixes common ESP32 bugs
- **Smart Completion**: Context-aware code completion suggestions
- **Code Templates**: 5+ built-in templates (LED, WiFi, sensors, web server)
- **Context-Aware**: Understands ESP32-specific development challenges
- **Error Suggestions**: Provides fixes for common errors

### 🔍 Professional Debugging
- **Live Data Monitoring**: Stream real-time data from connected ESP32 devices
- **Breakpoint Support**: Set/remove breakpoints on any line
- **Variable Watch**: Add variables to watch list for real-time monitoring
- **Memory Profiling**: Track heap usage, fragmentation, and memory warnings
- **Performance Analysis**: Get code performance scores (0-100) with suggestions
- **Color-Coded Output**: Red for errors, yellow for warnings, green for success
- **Device Validation**: Ensures USB connection before debugging operations

### ⚙️ ESP32 Development Tools
- **Compile**: Syntax checking and compilation
- **Upload**: Code uploading to ESP32 devices
- **Download**: Firmware extraction from connected devices
- **Board Selection**: Support for multiple ESP32 variants (ESP32, S2, S3, C3)
- **Serial Monitor**: Real-time serial communication with data streaming

### 📁 File Management
- **Create/Open/Save Files**: Complete file management
- **Folder Structure**: Hierarchical file tree with folders and subfolders
- **Multiple Tabs**: Open multiple files simultaneously in separate tabs
- **Project Support**: Load and save entire projects
- **Modified Tracking**: Visual indicators for unsaved changes
- **File Switching**: Quick navigation between files

## Building from Source

### Prerequisites

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
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

# Initialize and update submodules (includes glfw3)
git submodule update --init --recursive

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

All roadmap items are now complete or have framework ready for implementation!

## GUI Implementation

The IDE now includes a complete **ImGui-based graphical interface** with:

### Interface Layout
- **Left Panel**: File Explorer for project navigation
- **Center Panel**: 
  - Editor tab for code editing
  - Debugger tab for debugging sessions
  - Reverse Engineering tab for firmware analysis
- **Right Panel**: Editor properties and board configuration
- **Top Toolbar**: USB port selector, Upload, Download, Debug, RE buttons
- **Bottom Console**: Real-time output and status messages

### Technology Stack
- **GUI Framework**: Dear ImGui v1.90.1
- **Windowing**: GLFW3
- **Rendering**: OpenGL 3.3+
- **Backend Integration**: All existing components (TextEditor, FileManager, Compiler, SerialMonitor)

### Quick Start with GUI
```bash
# Build and run
mkdir build && cd build
cmake ..
cmake --build .
./esp32-driver-ide
```

For detailed usage instructions, see **[IMGUI_GUIDE.md](IMGUI_GUIDE.md)**.

## Previous GUI Information

The original implementation framework supports multiple GUI options:

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
- **GUI**: Platform-specific (GTK+3 for Linux, Win32 for Windows, Cocoa for macOS)

### Design Patterns
- **MVC Architecture**: Separation of model, view, and controller
- **Observer Pattern**: Callbacks for event handling
- **Factory Pattern**: Object creation and management
- **Strategy Pattern**: Different compilation strategies per board

### No External Dependencies
The core IDE logic has **zero external dependencies** beyond the standard C++ library. Platform GUI libraries are system-provided:
- Linux: GTK+3 (system package)
- Windows: Win32 (built into Windows)
- macOS: Cocoa (built into macOS)

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
