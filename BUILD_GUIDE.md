# Building and Using ESP32 Driver IDE (C++)

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake g++
```

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake (via Homebrew)
brew install cmake
```

### Windows
- Install Visual Studio 2019 or later with C++ development tools
- Install CMake 3.15 or later from https://cmake.org/

## Building

### Standard Build

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

### Build with Tests

```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest
```

### Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Terminal Mode Build

```bash
cmake -DBUILD_TERMINAL_MODE=ON ..
cmake --build .
# Creates: esp32-driver-ide-terminal
```

## Usage

### Enhanced GUI Interface

The IDE provides a professional graphical interface with gradient styling and modular panels:

```bash
./esp32-driver-ide  # Linux/macOS
# or
esp32-driver-ide.exe  # Windows
```

Features:
- **Modular Panels**: Dockable panels with file browser, editor, console, terminal, and device library
- **Gradient Styling**: Beautiful gradient backgrounds and smooth transitions
- **Integrated Terminal**: Execute commands directly within the IDE
- **Device Library**: Visual device management with preview and configuration
- **Code Editor**: Syntax highlighting with multiple file tabs
- **AI Assistant**: Context-aware code help and generation
- **Compiler Integration**: Compile and upload code to ESP32 devices
- **Serial Monitor**: Real-time communication with devices

### Terminal Mode (CLI)

For scripting, automation, and headless operation:

```bash
# Interactive mode
./esp32-driver-ide-terminal -i

# Command-line usage
./esp32-driver-ide-terminal --help              # Show help
./esp32-driver-ide-terminal boards              # List available boards
./esp32-driver-ide-terminal board "ESP32-S3 Dev Module"  # Set board
./esp32-driver-ide-terminal verify              # Compile
./esp32-driver-ide-terminal upload              # Upload to board
./esp32-driver-ide-terminal monitor 115200      # Open serial monitor
./esp32-driver-ide-terminal ask "How do I use GPIO pins?"  # Ask AI

# Scripting example
./esp32-driver-ide-terminal open mysketch.ino
./esp32-driver-ide-terminal verify && ./esp32-driver-ide-terminal upload
```

### API Usage Examples

#### Text Editor
```cpp
#include "editor/text_editor.h"

auto editor = std::make_unique<esp32_ide::TextEditor>();
editor->SetText("void setup() {\n  // Your code\n}");
editor->InsertText("\nvoid loop() {\n}", editor->GetCursorPosition());

// Undo/Redo
editor->Undo();
editor->Redo();

// Search
auto positions = editor->FindText("setup");
```

#### Syntax Highlighter
```cpp
#include "editor/syntax_highlighter.h"

auto highlighter = std::make_unique<esp32_ide::SyntaxHighlighter>();
auto tokens = highlighter->Tokenize(code);

for (const auto& token : tokens) {
    std::cout << highlighter->GetColorCode(token.type) 
              << token.text << "\033[0m";
}
```

#### File Manager
```cpp
#include "file_manager/file_manager.h"

auto file_manager = std::make_unique<esp32_ide::FileManager>();
file_manager->CreateFile("main.ino");
file_manager->SetFileContent("main.ino", code);
file_manager->SaveFile("main.ino");

auto files = file_manager->GetFileList();
```

#### AI Assistant
```cpp
#include "ai_assistant/ai_assistant.h"

auto assistant = std::make_unique<esp32_ide::AIAssistant>();
std::string response = assistant->Query("How do I use GPIO pins?");
std::string analysis = assistant->AnalyzeCode(code);
```

#### ESP32 Compiler
```cpp
#include "compiler/esp32_compiler.h"

auto compiler = std::make_unique<esp32_ide::ESP32Compiler>();
compiler->SetBoard(esp32_ide::ESP32Compiler::BoardType::ESP32);

auto result = compiler->Compile(code, compiler->GetBoard());
if (result.status == esp32_ide::ESP32Compiler::CompileStatus::SUCCESS) {
    compiler->Upload(compiler->GetBoard());
}
```

#### Serial Monitor
```cpp
#include "serial/serial_monitor.h"

auto serial = std::make_unique<esp32_ide::SerialMonitor>();
serial->Connect("/dev/ttyUSB0", 115200);
serial->SendData("Hello ESP32!");

auto messages = serial->GetMessages();
for (const auto& msg : messages) {
    std::cout << msg.content << "\n";
}
```

## Extending the IDE

### Adding New Board Types
```cpp
// In esp32_compiler.h
enum class BoardType {
    ESP32,
    ESP32_S2,
    ESP32_S3,
    ESP32_C3,
    YOUR_NEW_BOARD  // Add here
};
```

### Adding AI Assistant Knowledge
```cpp
// In ai_assistant.cpp, add new response handler
std::string AIAssistant::HandleYourTopicQuery() const {
    return "Your expert response here...";
}
```

### Adding Code Snippets
Create a new SnippetManager class:
```cpp
class SnippetManager {
    std::map<std::string, std::string> snippets_;
    
    void AddSnippet(const std::string& name, const std::string& code);
    std::string GetSnippet(const std::string& name);
};
```

## Troubleshooting

### Build Issues

**CMake not found**
```bash
# Linux
sudo apt-get install cmake

# macOS
brew install cmake
```

**Compiler errors**
Ensure you have C++17 support:
```bash
g++ --version  # Should be 7.0 or higher
```

**GTK+3 not found (Linux)**
```bash
sudo apt-get install libgtk-3-dev pkg-config
```

### Runtime Issues

**Application crashes on startup**
- Check console output for error messages
- Ensure all dependencies are installed
- Try running with debugging enabled

**No output in console**
- Redirect stderr: `./esp32-driver-ide 2>&1`
- Check file permissions

## Performance Tips

1. **Release builds**: Always use Release build for production
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

2. **Optimize binary size**: Use link-time optimization
   ```bash
   cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
   ```

3. **Memory usage**: The IDE uses in-memory file storage. For large projects, consider implementing disk-based storage.

## Testing

Run the test suite:
```bash
cd build
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest --verbose
```

## Deployment

### Linux
```bash
cmake --install . --prefix /usr/local
```

### macOS
Create an app bundle or use Homebrew formula.

### Windows
Use NSIS or WiX to create an installer.

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/kamer1337/esp32-driver-ide/issues
- Documentation: See README_CPP.md
- Examples: See examples/ directory (if available)

## License

MIT License - See LICENSE file for details.
