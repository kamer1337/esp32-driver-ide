# ESP32 Driver IDE - Complete Features Guide

This document provides comprehensive information about all features in the ESP32 Driver IDE.

## Table of Contents

1. [AI-Powered Features](#ai-powered-features)
2. [Code Editor](#code-editor)
3. [Debugging Tools](#debugging-tools)
4. [Device Support](#device-support)
5. [Advanced Tools](#advanced-tools)
6. [GUI Features](#gui-features)

---

## AI-Powered Features

### AI Code Generation

The AI Assistant can generate complete, working code snippets that can be directly inserted into your editor.

#### Usage
```cpp
AIAssistant ai;
std::string code = ai.GenerateCode("LED blink on pin 13");
```

#### Supported Code Generation
- LED blink patterns
- WiFi connection setup
- Bluetooth serial communication
- Sensor reading and data processing
- Web server setup
- Button input handling

### AI-Powered Code Refactoring

Intelligent code refactoring capabilities to improve code quality:

```cpp
std::string RefactorCode(const std::string& code, const std::string& type);
std::string OptimizeCode(const std::string& code);
std::string ImproveReadability(const std::string& code);
```

**Refactoring Types:**
- **"optimize"** or **"performance"**: Focus on performance improvements
- **"readability"** or **"clean"**: Focus on code clarity
- **default**: General refactoring with multiple improvements

**Detected Issues:**
- Blocking delays → millis() pattern
- String objects → char arrays
- Repeated calculations → cached results
- Serial.print() in loops → minimized output
- Magic numbers → named constants

### Automatic Bug Detection

Advanced bug detection system that identifies common ESP32 programming errors before compilation.

```cpp
std::vector<BugReport> DetectBugs(const std::string& code);
std::string AutoFixBugs(const std::string& code);
```

**Detected Issues:**

| Category | Severity | Description |
|----------|----------|-------------|
| Serial without begin() | Critical | Serial used without initialization |
| GPIO without pinMode | Critical | digitalWrite/Read without pinMode |
| WiFi without include | Critical | WiFi.* used without #include <WiFi.h> |
| delay() in ISR | Warning | Blocking delay in interrupt code |
| Float loop counter | Suggestion | Floating-point used as loop counter |

### Code Templates

Built-in template system with variable substitution for rapid prototyping:

```cpp
std::string ApplyTemplate(const std::string& name,
                         const std::map<std::string, std::string>& vars);
```

**Available Templates:**
- `basic` - Basic Arduino sketch
- `led_blink` - LED blink program (variable: {{pin}})
- `wifi_connect` - WiFi connection setup (variables: {{ssid}}, {{password}})
- `web_server` - Basic web server
- `sensor_read` - Analog sensor reading (variable: {{pin}})

### Smart Code Completion

Context-aware code completion with ESP32-specific suggestions:

```cpp
std::vector<CompletionItem> GetCompletionsAtCursor() const;
```

**Completion Categories:**
- GPIO operations (pinMode, digitalWrite, analogRead)
- Serial communication
- WiFi networking
- Timing functions (delay, millis, micros)
- Auto-setup scaffolding

---

## Code Editor

### Core Features

- **Syntax Highlighting**: Full C/C++ syntax highlighting optimized for ESP32/Arduino
- **Undo/Redo**: Complete undo/redo functionality
- **Multiple Files**: Support for managing multiple source files
- **Code Search**: Find and replace functionality
- **Line Management**: Line-based operations and navigation
- **Tab Management**: Multiple tabs with split views

### Advanced Editor Features

#### Tab Manager
```cpp
TabManager tabs;
tabs.CreateTab("sketch.ino");
tabs.SwitchTab(tab_id);
tabs.CloseTab(tab_id);
```

#### File Tree
- Hierarchical folder/subfolder/file structure
- Drag-and-drop support
- File operations (create, rename, delete)

#### Autocomplete Engine
- Context-aware suggestions
- ESP32 API completion
- Arduino function completion

---

## Debugging Tools

### Breakpoint Debugging

Full breakpoint support for step-through debugging:

```cpp
void AddBreakpoint(size_t line_number);
void RemoveBreakpoint(size_t line_number);
void ClearAllBreakpoints();
std::vector<size_t> GetBreakpoints() const;
bool HasBreakpoint(size_t line_number) const;
```

**Visual Indicators:**
- 🔴 Red dot on left margin
- Highlighted line background
- Bold line numbers

### Variable Watch Expressions

Monitor variables in real-time during debugging:

```cpp
void AddWatchVariable(const std::string& name, const std::string& type);
std::vector<WatchVariable> GetWatchVariables() const;
void UpdateWatchVariable(const std::string& name, const std::string& value);
```

**Watch Variable Structure:**
```cpp
struct WatchVariable {
    std::string name;        // Variable name
    std::string value;       // Current value
    std::string type;        // Data type
    long long last_update;   // Timestamp
};
```

### Memory Profiling

Real-time memory monitoring and leak detection:

```cpp
MemoryProfile GetMemoryProfile() const;
void StartMemoryProfiling();
std::vector<MemoryProfile> GetMemoryHistory() const;
```

**Memory Profile Structure:**
```cpp
struct MemoryProfile {
    size_t free_heap;
    size_t total_heap;
    size_t free_psram;
    size_t largest_free_block;
    float fragmentation_percent;
    std::vector<std::string> warnings;
};
```

**Memory Warnings:**
- Free heap < 20KB: CRITICAL
- Free heap < 50KB: WARNING
- Fragmentation > 30%: WARNING

### Performance Analysis

Comprehensive code performance analysis with scoring:

```cpp
PerformanceMetrics AnalyzePerformance(const std::string& code);
```

**Performance Scoring (Base: 100 points):**
- Blocking delay() in loop(): -10
- Each String object: -5
- Serial.print() in loops: -15
- Multiple analogRead(): -5
- delay() in ISR: -30 (CRITICAL)
- WiFi without status check: -10
- High RAM usage (>200KB): -20

---

## Device Support

### Supported ESP32 Variants

The IDE supports 8 ESP32 device variants:

1. **ESP32** - Original dual-core module
2. **ESP32-S2** - Single-core with USB OTG
3. **ESP32-S3** - Dual-core with AI acceleration
4. **ESP32-C3** - RISC-V single-core
5. **ESP32-C2** - Cost-optimized RISC-V
6. **ESP32-C6** - WiFi 6 + Matter/Thread
7. **ESP32-H2** - Zigbee/Thread for smart home
8. **ESP32-P4** - High-performance MCU

See [DEVICE_SUPPORT.md](DEVICE_SUPPORT.md) for detailed specifications.

### ML Device Detection

Machine learning-based automatic device type detection:

```cpp
MLDeviceDetector detector;
detector.LoadPretrainedModel();
auto result = detector.DetectDevice(features);
```

**Features:**
- Pretrained neural network model
- Automatic variant detection
- Confidence scores
- Async callbacks
- Non-blocking detection

See [ML_DEVICE_DETECTION.md](ML_DEVICE_DETECTION.md) for implementation details.

### Board Selection

Manual board selection with compiler configuration:

```cpp
compiler.SetBoard(ESP32Compiler::BoardType::ESP32_S3);
auto result = compiler.Compile(code, board);
```

---

## Advanced Tools

### Virtual Machine Emulator

Test code without physical ESP32 hardware:

```cpp
VMEmulator emulator;
emulator.Initialize();
emulator.SetGPIOMode(pin, mode);
emulator.WriteGPIO(pin, value);
```

**Simulation Features:**
- GPIO, PWM, ADC simulation
- WiFi and Bluetooth simulation
- Memory management tracking
- Peripheral configuration testing
- FreeRTOS task simulation

See [VM_EMULATOR.md](VM_EMULATOR.md) for detailed usage.

### Blueprint Editor

Visual component editor for device layouts:

```cpp
Blueprint bp;
auto led = std::make_unique<Component>("LED1", ComponentType::LED, "LED");
bp.AddComponent(std::move(led));
std::string code = bp.GenerateCode();
```

**Features:**
- Drag-and-drop component placement
- Visual connection between components
- Automatic code generation
- Component library
- 2D/3D rendering

### Advanced Decompiler

High-quality pseudo-code generation from firmware:

```cpp
AdvancedDecompiler decompiler;
std::vector<uint8_t> binary = LoadBinary("firmware.bin");
std::string pseudocode = decompiler.Decompile(binary);
```

**Analysis Features:**
- Control Flow Graph (CFG) analysis
- Data Flow Analysis (DFA)
- Variable type inference
- Function detection
- ESP32 API recognition
- FreeRTOS pattern detection

See [DECOMPILER_GUIDE.md](DECOMPILER_GUIDE.md) for examples.

### Scripting Engine

Extensible scripting for automation:

```cpp
ScriptingEngine engine;
engine.LoadScript("automation.txt");
engine.Execute();
```

**Supported Commands:**
- File operations
- Build automation
- Device communication
- Custom extensions

---

## GUI Features

### Enhanced GUI (Default)

Professional graphical interface with modular panels:

**Interface Options:**
- **Enhanced GUI** (Default): Full graphical interface
- **Simple GUI**: Lightweight native GUI
- **Terminal UI**: Text-based interface

### Modular Panel System

Flexible panel layout with docking support:

```cpp
panel_layout->AddPanel(std::move(editor_panel));
panel->SetDock(PanelDock::CENTER);
panel->SetMinSize(200, 100);
```

**Panel Features:**
- Dockable (LEFT, RIGHT, TOP, BOTTOM, CENTER)
- Floating panels
- Resizable with constraints
- Moveable and repositionable
- Auto-layout management

**Default Panels:**
- Editor (center)
- File Browser (left)
- Device Library (left)
- Preview (right)
- Console (bottom)
- Terminal (bottom)

### Device Library

Component library browser with preview:

```cpp
DeviceLibrary library;
library.AddDevice(device);
auto devices = library.GetDevicesByCategory(category);
library.AddDeviceInstance(device_id);
```

**Device Categories:**
- Microcontrollers
- Sensors
- Actuators
- Communication modules
- Display devices
- Power management

### Integrated Terminal

Built-in terminal within GUI:

```cpp
terminal->Initialize();
terminal->ExecuteCommand("ls");
terminal->SetCommandCallback(handler);
```

**Terminal Features:**
- Command execution
- Command history
- Custom command handlers
- Color-coded output
- Auto-completion

### Dark Theme with Gradient Styling

Professional dark theme with gradient backgrounds and smooth transitions:

**Color Scheme:**
- Background: `#1E1E1E` (base)
- Background Gradient: `#2D2D30` → `#1E1E1E` (top to bottom)
- Panel Background: `#252526` (base)
- Panel Gradient: `#2D2D30` → `#1E1E1E` (top to bottom)
- Panel Border: `#3E3E42`
- Panel Active Border: `#007ACC` (highlighted)
- Text: `#CCCCCC`
- Text Dim: `#6A6A6A`
- Accent: `#007ACC`
- Accent Light: `#0098FF`
- Button Gradient: `#3E3E42` → `#2D2D30`
- Error: `#F14C4C`
- Success: `#89D185`
- Warning: `#CCA700`

**Gradient Features:**
- Smooth color transitions for backgrounds
- Vertical and horizontal gradient support
- Active panel highlighting with accent colors
- Button depth with gradient styling
- Professional polish with subtle gradients

---

## Integration Examples

### Complete Workflow

```cpp
// 1. Start with template
FileManager fm;
std::map<std::string, std::string> vars = {{"pin", "13"}};
std::string code = fm.ApplyTemplate("led_blink", vars);

// 2. Load into editor
TextEditor editor;
editor.SetText(code);

// 3. Add breakpoint
editor.AddBreakpoint(5);

// 4. Detect and fix bugs
AIAssistant ai;
auto bugs = ai.DetectBugs(code);
if (!bugs.empty()) {
    code = ai.AutoFixBugs(code);
    editor.SetText(code);
}

// 5. Optimize code
code = ai.OptimizeCode(code);
editor.SetText(code);

// 6. Analyze performance
ESP32Compiler compiler;
auto metrics = compiler.AnalyzePerformance(code);
std::cout << "Performance Score: " << metrics.performance_score << std::endl;

// 7. Compile and upload
compiler.SetBoard(ESP32Compiler::BoardType::ESP32);
auto result = compiler.Compile(code, board);
if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
    compiler.Upload(board);
}

// 8. Monitor with serial
SerialMonitor monitor;
monitor.Connect("/dev/ttyUSB0", 115200);
monitor.AddWatchVariable("led_state", "bool");
monitor.StartMemoryProfiling();
```

---

## Building and Testing

See [BUILD_GUIDE.md](BUILD_GUIDE.md) for build instructions.

### Quick Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
./esp32-driver-ide
```

### Build Options

```bash
# Terminal UI
cmake -DUSE_TERMINAL_UI=ON ..

# With tests
cmake -DBUILD_TESTS=ON ..

# Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
```

---

## API Reference

### AIAssistant
```cpp
std::string GenerateCode(const std::string& request);
std::string RefactorCode(const std::string& code, const std::string& type);
std::string OptimizeCode(const std::string& code);
std::string ImproveReadability(const std::string& code);
std::vector<BugReport> DetectBugs(const std::string& code);
std::string AutoFixBugs(const std::string& code);
```

### FileManager
```cpp
void CreateFile(const std::string& name, const std::string& content);
void SaveFile(const std::string& name);
std::string LoadFile(const std::string& name);
void AddTemplate(const std::string& name, const std::string& code,
                const std::string& description, const std::vector<std::string>& tags);
std::string ApplyTemplate(const std::string& name,
                         const std::map<std::string, std::string>& vars);
```

### TextEditor
```cpp
void SetText(const std::string& text);
std::string GetText() const;
void InsertText(const std::string& text, size_t position);
void DeleteText(size_t start, size_t end);
void AddBreakpoint(size_t line_number);
void RemoveBreakpoint(size_t line_number);
std::vector<CompletionItem> GetCompletionsAtCursor() const;
```

### ESP32Compiler
```cpp
void SetBoard(BoardType board);
CompileResult Compile(const std::string& code, const std::string& board);
bool Upload(const std::string& board);
PerformanceMetrics AnalyzePerformance(const std::string& code);
```

### SerialMonitor
```cpp
bool Connect(const std::string& port, int baud_rate);
void Disconnect();
void SendMessage(const std::string& message);
void AddWatchVariable(const std::string& name, const std::string& type);
MemoryProfile GetMemoryProfile() const;
void StartMemoryProfiling();
```

---

## Best Practices

### Development Workflow
1. Use templates for common patterns
2. Enable syntax highlighting
3. Set breakpoints at key points
4. Run bug detection regularly
5. Monitor performance scores
6. Profile memory for long-running code

### Code Quality
1. Fix critical bugs immediately
2. Address warnings when possible
3. Aim for performance scores > 80
4. Use optimization suggestions
5. Test with VM emulator before hardware

### Debugging
1. Watch critical variables
2. Monitor memory during loops
3. Check performance regularly
4. Use breakpoints wisely
5. Profile before optimization

---

## Version Information

- **IDE Version**: 2.0.0
- **Feature Set**: Complete
- **Last Updated**: November 2025
- **Build System**: CMake 3.15+
- **C++ Standard**: C++17

---

## See Also

- [README](README.md) - Getting started guide
- [BUILD_GUIDE](BUILD_GUIDE.md) - Building from source
- [DEVICE_SUPPORT](DEVICE_SUPPORT.md) - Supported devices
- [ML_DEVICE_DETECTION](ML_DEVICE_DETECTION.md) - ML detection details
- [VM_EMULATOR](VM_EMULATOR.md) - Virtual machine emulator
- [DECOMPILER_GUIDE](DECOMPILER_GUIDE.md) - Decompiler usage
