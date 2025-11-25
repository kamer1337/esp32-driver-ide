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
- **Terminal Mode**: Complete CLI alternative

### Complete GUI Widget System

The IDE provides a comprehensive set of GUI widgets for building professional interfaces:

#### Button Widget
```cpp
auto button = std::make_unique<Button>("compile_btn", "Compile");
button->SetIcon("▶");
button->SetOnClick([]() { BackendFramework::GetInstance().Verify(); });
```

#### Dropdown Widget
```cpp
auto dropdown = std::make_unique<Dropdown>("board_select");
dropdown->AddItem("ESP32 Dev Module");
dropdown->AddItem("ESP32-S3 Dev Module");
dropdown->SetOnSelect([](int index, const std::string& value) {
    std::cout << "Selected: " << value << std::endl;
});
```

#### Slider Widget
```cpp
auto slider = std::make_unique<Slider>("baud_rate", 9600, 921600);
slider->SetValue(115200);
slider->SetStep(9600);
slider->SetOnValueChange([](float value) {
    BackendFramework::GetInstance().SetSerialBaudRate(static_cast<int>(value));
});
```

#### Checkbox Widget
```cpp
auto checkbox = std::make_unique<Checkbox>("auto_upload", "Auto Upload", false);
checkbox->SetOnToggle([](bool checked) {
    std::cout << "Auto upload: " << (checked ? "enabled" : "disabled") << std::endl;
});
```

#### TextInput Widget
```cpp
auto input = std::make_unique<TextInput>("ssid_input", "Enter SSID");
input->SetMaxLength(32);
input->SetOnSubmit([](const std::string& text) {
    std::cout << "SSID set to: " << text << std::endl;
});
```

#### TabBar Widget
```cpp
auto tabs = std::make_unique<TabBar>("file_tabs");
tabs->AddTab("main", "main.ino", "", true);
tabs->AddTab("config", "config.h", "", true);
tabs->SetOnTabChange([](int index, const std::string& tab_id) {
    std::cout << "Switched to: " << tab_id << std::endl;
});
```

#### MenuBar Widget (Arduino IDE Style)
```cpp
auto menubar = std::make_unique<MenuBar>("main_menu");

// File menu
menubar->AddMenu("file", "File");
menubar->AddMenuItem("file", "new", "New", "Ctrl+N", ArduinoActions::New);
menubar->AddMenuItem("file", "open", "Open...", "Ctrl+O", ArduinoActions::Open);
menubar->AddMenuItem("file", "save", "Save", "Ctrl+S", ArduinoActions::Save);
menubar->AddSeparator("file");
menubar->AddMenuItem("file", "quit", "Quit", "Ctrl+Q", ArduinoActions::Quit);

// Sketch menu
menubar->AddMenu("sketch", "Sketch");
menubar->AddMenuItem("sketch", "verify", "Verify/Compile", "Ctrl+R", ArduinoActions::Verify);
menubar->AddMenuItem("sketch", "upload", "Upload", "Ctrl+U", ArduinoActions::Upload);
```

#### Toolbar Widget (Arduino IDE Style)
```cpp
auto toolbar = std::make_unique<Toolbar>("main_toolbar");
toolbar->AddButton("verify", "✓", "Verify", "Compile", ArduinoActions::Verify);
toolbar->AddButton("upload", "→", "Upload", "Upload to board", ArduinoActions::Upload);
toolbar->AddSeparator();
toolbar->AddButton("new", "+", "New", "New sketch", ArduinoActions::New);
toolbar->AddButton("open", "📁", "Open", "Open file", ArduinoActions::Open);
toolbar->AddButton("save", "💾", "Save", "Save file", ArduinoActions::Save);
```

#### StatusBar Widget
```cpp
auto statusbar = std::make_unique<StatusBar>("main_status");
statusbar->AddSection("status", "Ready", -1);
statusbar->AddSection("cursor", "Ln 1, Col 1", 100);
statusbar->AddSection("board", "ESP32 on /dev/ttyUSB0", 200);

// Update sections
statusbar->SetStatusText("Compiling...");
statusbar->SetCursorPosition(42, 15);
statusbar->SetBoardInfo("ESP32-S3", "/dev/ttyACM0");
```

#### Additional Widgets
- **Label**: Static text display with alignment options
- **ProgressBar**: Progress indication with percentage display
- **TreeView**: Hierarchical file/data display
- **ListView**: Scrollable list with selection
- **SplitView**: Resizable panel dividers

### Backend Framework

Centralized component management with event-based communication:

```cpp
// Get the singleton instance
auto& framework = BackendFramework::GetInstance();
framework.Initialize();

// Access components
auto* editor = framework.GetTextEditor();
auto* compiler = framework.GetCompiler();
auto* serial = framework.GetSerialMonitor();

// Event handling
framework.AddEventHandler(BackendFramework::EventType::COMPILE_SUCCESS, 
    [](const BackendFramework::Event& event) {
        std::cout << "Compilation successful!" << std::endl;
    });

// Arduino-style operations
framework.NewFile("blink.ino");
framework.Verify();    // Compile
framework.Upload();    // Upload to board
framework.OpenSerialMonitor();
```

### Arduino Actions

Pre-built menu actions matching Arduino IDE functionality:

```cpp
// File actions
ArduinoActions::New();
ArduinoActions::Open();
ArduinoActions::Save();

// Sketch actions
ArduinoActions::Verify();
ArduinoActions::Upload();

// Tools actions
ArduinoActions::SerialMonitor();
ArduinoActions::BoardManager();
ArduinoActions::AutoFormat();
```

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

### Terminal Mode (Alternative CLI)

Complete command-line interface for headless/scriptable operation:

```bash
# Start interactive mode
./esp32-driver-ide-terminal -i

# Single command execution
./esp32-driver-ide-terminal verify
./esp32-driver-ide-terminal upload
./esp32-driver-ide-terminal boards
```

**Available Commands:**

| Category | Commands |
|----------|----------|
| File Operations | `new`, `open`, `save`, `close`, `list`, `cat`, `edit` |
| Board & Port | `board`, `port`, `boards`, `ports` |
| Compile & Upload | `verify`, `upload` |
| Serial | `monitor`, `send` |
| Emulator | `emulator [start|stop|status]` |
| AI Assistant | `ask`, `generate`, `analyze`, `fix` |
| Device Library | `devices`, `add-device` |
| Settings | `config`, `set`, `get` |

**Interactive Mode Example:**
```
esp32> board ESP32-S3 Dev Module
✓ Board set to: ESP32-S3 Dev Module

esp32> verify
ℹ Compiling sketch...
[██████████████████████████████] 100%
✓ Compilation successful

esp32> upload
ℹ Uploading to ESP32-S3 Dev Module on /dev/ttyUSB0...
✓ Upload complete

esp32> monitor 115200
✓ Serial monitor opened at 115200 baud
ℹ Press Ctrl+C to close
```

**Scripting Support:**
```bash
# Automated build script
./esp32-driver-ide-terminal open project.ino
./esp32-driver-ide-terminal verify && ./esp32-driver-ide-terminal upload
```

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

## Version 1.3.0 Features (NEW)

### Natural Language Commands

The AI Assistant now supports natural language command interpretation for intuitive interaction:

```cpp
AIAssistant ai;

// Interpret natural language
auto interpretation = ai.InterpretNaturalLanguage("Create a LED blink program");
// Returns: action="generate_code", target="led_blink", confidence=0.9

// Execute commands directly
std::string result = ai.ExecuteNaturalLanguageCommand("Generate WiFi connection code");
```

**Supported Commands:**
- "Create/Generate/Make [feature]" - Code generation
- "Optimize/Improve/Refactor code" - Code optimization
- "Analyze/Check/Scan code" - Code analysis
- "Fix/Repair bug/error" - Bug fixing

### Advanced Code Analysis

Comprehensive security and quality analysis capabilities:

#### Security Vulnerability Scanning
```cpp
AIAssistant ai;
auto issues = ai.ScanSecurityVulnerabilities(code);

for (const auto& issue : issues) {
    std::cout << "[" << issue.severity << "] Line " << issue.line_number 
              << ": " << issue.type << std::endl;
    std::cout << "Recommendation: " << issue.recommendation << std::endl;
}

// Generate full report
std::string report = ai.GenerateSecurityReport(code);
```

**Detected Security Issues:**
- Hardcoded credentials
- Buffer overflow risks
- Unbounded input
- Insecure HTTP connections

#### Performance Optimization Suggestions
```cpp
auto perf_issues = ai.SuggestPerformanceOptimizations(code);

for (const auto& issue : perf_issues) {
    std::cout << "Impact: " << issue.impact_score << "/10" << std::endl;
    std::cout << "Optimization: " << issue.optimization << std::endl;
}

std::string perf_report = ai.GeneratePerformanceReport(code);
```

**Detected Performance Issues:**
- Blocking delays in loop()
- String concatenation in loops
- Excessive analogRead() calls
- Inefficient loop conditions

#### Code Smell Detection
```cpp
auto smells = ai.DetectCodeSmells(code);

for (const auto& smell : smells) {
    std::cout << "Type: " << smell.type << std::endl;
    std::cout << "Suggestion: " << smell.refactoring_suggestion << std::endl;
}
```

**Detected Code Smells:**
- Magic numbers
- Long lines (>120 characters)
- Commented-out code
- Duplicate code patterns

### Learning Mode

Personalized AI assistance based on your coding patterns:

```cpp
AIAssistant ai;

// Enable learning mode
ai.EnableLearningMode(true);

// Record usage patterns
std::map<std::string, std::string> params = {{"type", "wifi"}};
ai.RecordUsagePattern("wifi_connection", params);

// Get personalized suggestions
auto suggestions = ai.GetPersonalizedSuggestions("working with wifi");

for (const auto& suggestion : suggestions) {
    std::cout << suggestion.suggestion << std::endl;
    std::cout << "Reason: " << suggestion.reasoning << std::endl;
    std::cout << "Relevance: " << suggestion.relevance_score << std::endl;
}

// View usage history
auto patterns = ai.GetUsagePatterns();
```

### Git Integration

Full version control capabilities within the IDE:

```cpp
GitIntegration git;

// Initialize/Open repository
git.InitRepository("/path/to/repo");
git.OpenRepository("/path/to/existing/repo");

// Stage and commit
git.StageFile("src/main.cpp");
git.Commit("Add LED support", "John Doe");

// Branch management
auto branches = git.GetBranches();
git.CreateBranch("feature/new-sensor");
git.SwitchBranch("feature/new-sensor");

// View history and diffs
auto commits = git.GetCommitHistory(10);
std::string diff = git.GetDiff("src/main.cpp");

// Remote operations
git.AddRemote("origin", "https://github.com/user/repo.git");
git.Push("origin", "main");
git.Pull("origin", "main");
```

### Code Review System

Comprehensive code review tools with comment tracking:

```cpp
CodeReviewSystem review;

// Create review
std::string review_id = review.CreateReview(
    "Add LED Support",
    "This PR adds LED blinking functionality",
    "developer1"
);

// Add reviewers
review.AddReviewer(review_id, "reviewer1");

// Add comments
std::string comment_id = review.AddComment(
    review_id, "reviewer1", "led.cpp", 42,
    CodeReviewSystem::CommentType::SUGGESTION,
    "Consider using a constant for the delay value"
);

// Track changes
review.TrackChange("led.cpp", 42, "modified", 
                   "delay(1000);", "delay(LED_DELAY);", "developer1");

// Approve or request changes
review.ApproveReview(review_id, "reviewer1");
review.RequestChanges(review_id, "reviewer2", "Need better error handling");

// Generate report
std::string report = review.GenerateReviewReport(review_id);
```

### Testing Framework

Professional testing infrastructure with full support for unit tests, coverage, mocks, and hardware-in-loop testing:

#### Unit Testing
```cpp
using namespace esp32_ide::testing;

// Create test suite
TestSuite suite("My Tests");

// Add tests
suite.AddTest("test_led_on", []() {
    Assert::IsTrue(led.IsOn());
});

suite.AddTest("test_led_off", []() {
    Assert::IsFalse(led.IsOn());
});

// Add setup/teardown
suite.AddSetup([]() {
    led.Initialize();
});

suite.AddTeardown([]() {
    led.Cleanup();
});

// Run tests
auto results = suite.Run();
```

#### Test Runner
```cpp
TestRunner runner;
runner.AddSuite(&suite1);
runner.AddSuite(&suite2);

// Configure
runner.SetVerbose(true);
runner.SetStopOnFailure(false);

// Run all tests
auto results = runner.RunAll();

// Generate report
std::string report = runner.GenerateReport();
std::cout << "Passed: " << runner.GetPassedCount() << std::endl;
std::cout << "Failed: " << runner.GetFailedCount() << std::endl;
```

#### Coverage Analysis
```cpp
CoverageAnalyzer analyzer;

// Register source files
analyzer.AddSourceFile("main.cpp", 100);
analyzer.AddFunction("main.cpp", "setup");
analyzer.AddFunction("main.cpp", "loop");

// Track coverage
analyzer.StartTracking();
analyzer.RecordLineCoverage("main.cpp", 10);
analyzer.RecordFunctionCoverage("main.cpp", "setup");
analyzer.StopTracking();

// Get results
auto info = analyzer.GetCoverageInfo();
std::cout << "Line Coverage: " << info.GetLineCoverage() << "%" << std::endl;
std::cout << "Function Coverage: " << info.GetFunctionCoverage() << "%" << std::endl;

std::string report = analyzer.GenerateCoverageReport();
```

#### Mock Framework
```cpp
MockFramework mock;

// Register mock functions
mock.RegisterMock("digitalWrite");
mock.SetReturnValue("digitalWrite", "void");
mock.SetExpectedCalls("digitalWrite", 2);

// Record calls in your code
mock.RecordCall("digitalWrite", {"13", "HIGH"});
mock.RecordCall("digitalWrite", {"13", "LOW"});

// Verify
Assert::AreEqual(2, mock.GetCallCount("digitalWrite"));
Assert::IsTrue(mock.VerifyExpectedCalls());
```

#### Hardware-in-Loop Testing
```cpp
HardwareInLoopSimulator simulator;

// Create test scenario
HardwareInLoopSimulator::TestScenario scenario;
scenario.name = "LED Blink Test";
scenario.description = "Verify LED blinks correctly";
scenario.steps = {"Initialize GPIO", "Turn LED on", "Wait", "Turn LED off"};
scenario.expected_outputs["led_state"] = "off";

simulator.AddScenario(scenario);

// Simulate hardware inputs
simulator.SimulateGPIOInput(2, true);
simulator.SimulateSensorData(34, 2048);

// Run tests
auto result = simulator.RunScenario("LED Blink Test");
Assert::AreEqual(TestStatus::PASSED, result.status);
```

#### Assertion Helpers
```cpp
// Boolean assertions
Assert::IsTrue(condition);
Assert::IsFalse(condition);

// Equality assertions
Assert::AreEqual(expected, actual);
Assert::AreNotEqual(a, b);

// Null checks
Assert::IsNull(ptr);
Assert::IsNotNull(ptr);

// Exception assertions
Assert::Throws([]() { throw std::runtime_error("error"); });
Assert::DoesNotThrow([]() { /* safe code */ });
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
