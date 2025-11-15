# ESP32 Driver IDE (C++ Version)

A modern, native C++ IDE for ESP32 development with an integrated AI assistant and **terminal-based interface with no external GUI dependencies**.

## ✨ Latest Update: Pure C++ with Custom GUI Option (November 2025)

**🎨 Zero External Dependencies**: Complete reimplementation with pure C++ and optional lightweight native GUI!

**🚀 Lightweight & Fast**: Pure C++ with zero external dependencies - no GUI frameworks required!

**💻 Two Interface Options**: 
1. **Terminal Interface** (Default): Interactive menu system with ANSI color support
2. **Simple GUI** (Optional): Lightweight native GUI using X11 (Linux) or Win32 (Windows)

**📦 Easy Build**: Single command build with no external dependencies to install!

## Features

### Interface Options

#### Terminal-Based Interface (Default)
- **Interactive Menu System**: Easy-to-use text-based interface
- **Color-Coded Output**: ANSI escape codes for visual feedback
- **File Browser**: Browse and open project files
- **Text Editor**: View and edit code with line numbers
- **Console Output**: View compilation and runtime messages
- **Serial Monitor**: Connect and communicate with ESP32 devices

#### Simple GUI (Optional Build)
- **Native Platform APIs**: X11 on Linux, Win32 on Windows
- **Lightweight**: No external GUI frameworks required
- **Dark Theme**: Professional appearance optimized for coding
- **Multi-Panel Layout**: Menu bar, toolbar, editor, console, file list
- **Interactive**: Mouse and keyboard support

### Core Functionality


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
- **Board Selection**: Support for multiple ESP32 variants (ESP32, S2, S3, C3)
- **Serial Monitor**: Real-time serial communication with data streaming
- **VM Emulator**: Virtual machine for testing device configurations without hardware

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

### Quick Start
```bash
# Build and run (Terminal UI - default)
mkdir build && cd build
cmake ..
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
