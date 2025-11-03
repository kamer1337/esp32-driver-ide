# ESP32 Driver IDE - ImGui Interface Guide

## Overview

The ESP32 Driver IDE now features a modern ImGui-based graphical user interface with a professional three-panel layout optimized for ESP32 development, debugging, and reverse engineering.

![ESP32 Driver IDE Interface](https://github.com/user-attachments/assets/18511046-9c87-48ff-a377-960e97afd9f3)

## Interface Layout

### Three-Panel Design

The IDE features a professional three-panel layout:

#### Left Panel: File Explorer
- Browse and manage project files
- Quick file switching
- New file creation
- Refresh file list
- Shows all files in the project

#### Center Panel: Multi-Tab Workspace
Three main tabs for different workflows:

1. **Editor Tab**
   - Full-featured code editor
   - 1MB buffer for large files
   - Syntax-aware text editing
   - Real-time code modification
   - Tab support
   - Line-based editing

2. **Debugger Tab**
   - ESP32 debugging controls
   - Breakpoint management
   - Variable inspection
   - Start/Stop debugging
   - Debug session management

3. **Reverse Engineering Tab**
   - Firmware analysis tools
   - Binary disassembly
   - Architecture detection (Xtensa LX6)
   - Function detection
   - String extraction
   - Entry point identification

#### Right Panel: Editor Properties
- File information (name, size, line count)
- Editor settings
  - Show line numbers
  - Auto indent
  - Syntax highlighting
- Board configuration
  - Board type selection (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
  - CPU frequency adjustment (80-240 MHz)
- Quick settings apply

### Top Toolbar

The toolbar provides quick access to essential functions:

- **USB Port Selector**: Choose the serial port for your ESP32 device
- **Refresh Ports**: Scan for available serial ports
- **Baud Rate**: Configure serial communication speed (9600 - 921600)
- **Upload**: Flash code to ESP32
- **Download**: Download firmware from ESP32
- **Debug**: Start debugging session
- **RE**: Launch reverse engineering analysis

### Bottom Panel: Console

- Real-time output display
- Compilation messages
- Upload status
- Debug information
- Error messages
- Clear console button
- Auto-scroll to latest messages

### Menu Bar

Standard menu system with:
- **File**: New, Save, Exit
- **View**: Toggle File Explorer, Toggle Properties Panel
- **Tools**: Compile, Upload
- **Help**: About

## Building the ImGui Version

### Prerequisites

Install required dependencies:

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake g++
sudo apt-get install -y libglfw3-dev libgl1-mesa-dev
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake glfw
```

#### Windows
- Visual Studio 2019 or later with C++ tools
- CMake 3.15 or later
- GLFW3 (will be linked automatically)

### Build Instructions

```bash
# Clone repository
git clone https://github.com/kamer1337/esp32-driver-ide.git
cd esp32-driver-ide

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./esp32-driver-ide  # Linux/macOS
# or
esp32-driver-ide.exe  # Windows
```

## Using the IDE

### Basic Workflow

1. **Start the IDE**
   ```bash
   ./esp32-driver-ide
   ```

2. **Create or Open a File**
   - Click "New File" in the File Explorer
   - Or select an existing file from the list

3. **Write Your Code**
   - Use the Editor tab in the center panel
   - Code is auto-saved to the file manager

4. **Configure Board Settings**
   - Use the Properties panel on the right
   - Select your ESP32 board type
   - Adjust CPU frequency if needed

5. **Select USB Port**
   - Use the toolbar port selector
   - Click "Refresh Ports" if your device isn't listed
   - Choose appropriate baud rate (typically 115200)

6. **Compile and Upload**
   - Click "Compile" in the Tools menu or use Ctrl+R
   - Check the console for compilation results
   - Click "Upload" to flash to your ESP32

7. **Debug (Optional)**
   - Click "Debug" button in toolbar
   - Switch to Debugger tab
   - Set breakpoints and inspect variables

8. **Reverse Engineering (Advanced)**
   - Click "RE" button in toolbar
   - Switch to Reverse Engineering tab
   - Analyze firmware structure and functions

### Keyboard Shortcuts

- **Ctrl+N**: New file
- **Ctrl+S**: Save file
- **Ctrl+R**: Compile
- **Ctrl+U**: Upload

## Features

### Code Editor
- Multi-line text editing
- Tab support
- Large file handling (1MB buffer)
- Real-time updates
- Integration with backend TextEditor

### File Management
- Create new files
- Open existing files
- Save files
- Switch between multiple files
- File explorer navigation

### Compilation & Upload
- Syntax checking
- Board-specific compilation
- Serial port selection
- Baud rate configuration
- Upload progress tracking

### Debugging
- Start/stop debugging
- Breakpoint support (framework)
- Variable inspection (framework)
- Debug output in console

### Reverse Engineering
- Binary analysis
- Disassembly
- Function detection
- String extraction
- Architecture identification

### Console Output
- Color-coded messages
- Success/error/warning indicators
- Compilation output
- Upload status
- Debug messages
- Scrollable history
- Clear console functionality

## Architecture

The ImGui implementation is built on top of the existing backend components:

```
ImGuiWindow (src/gui/imgui_window.cpp)
├── TextEditor (src/editor/text_editor.cpp)
├── FileManager (src/file_manager/file_manager.cpp)
├── ESP32Compiler (src/compiler/esp32_compiler.cpp)
└── SerialMonitor (src/serial/serial_monitor.cpp)
```

### Technology Stack

- **GUI Framework**: Dear ImGui v1.90.1
- **Windowing**: GLFW3
- **Rendering**: OpenGL 3.3+
- **Language**: C++17
- **Build System**: CMake

### ImGui Integration

The IDE uses:
- ImGui core library for UI widgets
- GLFW backend for window management
- OpenGL3 backend for rendering
- Custom dark theme for professional appearance

## Customization

### Changing the Theme

Edit `ImGuiWindow::SetupImGuiStyle()` in `src/gui/imgui_window.cpp` to customize colors and appearance.

### Adding New Features

The modular design allows easy feature additions:

1. Add UI elements in `RenderXXX()` methods
2. Connect to backend components
3. Update console with status messages

### Panel Visibility

Panels can be toggled:
- View → File Explorer
- View → Properties Panel

## Troubleshooting

### Window doesn't appear
- Ensure GLFW3 is installed
- Check that OpenGL 3.3+ is available
- Verify display environment (DISPLAY variable on Linux)

### Build errors
- Install all dependencies (libglfw3-dev, libgl1-mesa-dev)
- Ensure CMake 3.15 or later
- Use C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)

### Serial port not detected
- Click "Refresh Ports" button
- Check device permissions (Linux: add user to dialout group)
- Verify ESP32 is connected via USB

## Performance

The ImGui interface is lightweight and efficient:
- ~60 FPS rendering on modern hardware
- Low memory footprint
- Minimal CPU usage when idle
- Fast startup time

## Future Enhancements

Potential improvements:
- Docking support (requires ImGui docking branch)
- Multiple editor windows
- Split view editing
- Advanced syntax highlighting in ImGui
- Integrated terminal
- Project templates
- Code completion
- Git integration

## License

MIT License - Same as the main project

## Contributing

Contributions to the ImGui interface are welcome! Focus areas:
- UI/UX improvements
- Additional features
- Performance optimizations
- Cross-platform compatibility
- Documentation

## Credits

- **Dear ImGui**: Omar Cornut and contributors
- **GLFW**: Marcus Geelnard and Camilla Löwy
- **ESP32 Driver IDE**: kamer1337 and contributors
