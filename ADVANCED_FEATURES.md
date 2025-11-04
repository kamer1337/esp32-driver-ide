# Advanced Features Guide - ESP32 Driver IDE

This document provides detailed information about the advanced features implemented in the ESP32 Driver IDE.

## Table of Contents
1. [AI-Powered Code Refactoring](#ai-powered-code-refactoring)
2. [Automatic Bug Detection](#automatic-bug-detection)
3. [Custom Code Templates](#custom-code-templates)
4. [Breakpoint Debugging](#breakpoint-debugging)
5. [Variable Watch Expressions](#variable-watch-expressions)
6. [Memory Profiling](#memory-profiling)
7. [Performance Analysis](#performance-analysis)
8. [Code Completion](#code-completion)

---

## AI-Powered Code Refactoring

The AI Assistant now includes intelligent code refactoring capabilities to help you write better, more efficient code.

### Features

#### RefactorCode(code, refactor_type)
Performs comprehensive code refactoring based on the specified type:
- **"optimize"** or **"performance"**: Focus on performance improvements
- **"readability"** or **"clean"**: Focus on code clarity
- **default**: General refactoring with multiple improvements

#### OptimizeCode(code)
Analyzes code and suggests performance optimizations:
- Detects repeated analogRead() calls
- Identifies String usage (memory intensive)
- Warns about Serial.print() in loops
- Provides specific optimization suggestions

#### ImproveReadability(code)
Enhances code readability:
- Detects magic numbers in pinMode() calls
- Suggests using named constants
- Recommends adding comments for complex logic

### Usage Example

```cpp
#include "ai_assistant/ai_assistant.h"

AIAssistant ai;

// Original code
std::string code = R"(
void loop() {
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
)";

// Refactor for better performance
std::string optimized = ai.OptimizeCode(code);
// Returns code with suggestions to use millis() instead of delay()

// Improve readability
std::string readable = ai.ImproveReadability(code);
// Suggests: const int LED_PIN = 13;
```

### Refactoring Suggestions

The refactoring system detects and suggests fixes for:
- Blocking delays in loop() → Use millis() pattern
- String objects → Use char arrays
- Repeated calculations → Cache results
- Serial.print() in loops → Minimize output calls
- Magic numbers → Define as constants

---

## Automatic Bug Detection

Advanced bug detection system that identifies common ESP32 programming errors before compilation.

### Features

#### DetectBugs(code)
Scans code and returns a list of bugs with:
- **Severity**: "critical", "warning", or "suggestion"
- **Line number**: Where the bug occurs
- **Description**: What the bug is
- **Suggested fix**: How to fix it

#### AutoFixBugs(code)
Automatically fixes critical bugs:
- Adds missing Serial.begin()
- Adds missing WiFi.h includes
- (More auto-fixes can be added)

### Detected Issues

| Category | Severity | Description |
|----------|----------|-------------|
| Serial without begin() | Critical | Serial used without initialization |
| GPIO without pinMode | Critical | digitalWrite/Read without pinMode |
| WiFi without include | Critical | WiFi.* used without #include <WiFi.h> |
| delay() in ISR | Warning | Blocking delay in interrupt code |
| Float loop counter | Suggestion | Floating-point used as loop counter |

### Usage Example

```cpp
// Detect all bugs
auto bugs = ai.DetectBugs(code);

for (const auto& bug : bugs) {
    std::cout << "[" << bug.severity << "] Line " << bug.line_number << std::endl;
    std::cout << "Problem: " << bug.description << std::endl;
    std::cout << "Fix: " << bug.suggested_fix << std::endl;
}

// Auto-fix critical bugs
std::string fixed_code = ai.AutoFixBugs(code);
```

### Example Output

```
[critical] Line 5
Problem: Serial used without initialization
Fix: Add Serial.begin(115200); in setup() function

[warning] Line 12
Problem: Delay used in interrupt-related code
Fix: Use millis() or hardware timers instead of delay()
```

---

## Custom Code Templates

Built-in template system with variable substitution for rapid prototyping.

### Built-in Templates

| Template Name | Description | Variables |
|---------------|-------------|-----------|
| `basic` | Basic Arduino sketch | None |
| `led_blink` | LED blink program | {{pin}} |
| `wifi_connect` | WiFi connection setup | {{ssid}}, {{password}} |
| `web_server` | Basic web server | {{ssid}}, {{password}} |
| `sensor_read` | Analog sensor reading | {{pin}} |

### Features

#### AddTemplate(name, code, description, tags)
Add your own custom templates:
```cpp
file_manager.AddTemplate("my_template", 
    "const int PIN = {{pin}};\nvoid setup() { pinMode(PIN, OUTPUT); }",
    "Custom LED template",
    {"led", "custom"});
```

#### ApplyTemplate(name, variables)
Apply template with variable substitution:
```cpp
std::map<std::string, std::string> vars = {
    {"pin", "13"},
    {"ssid", "MyNetwork"},
    {"password", "MyPassword123"}
};

std::string code = file_manager.ApplyTemplate("wifi_connect", vars);
```

#### GetTemplatesByTag(tag)
Filter templates by tag:
```cpp
auto wifi_templates = file_manager.GetTemplatesByTag("wifi");
```

### Template Format

Templates use `{{variable}}` syntax for placeholders:

```cpp
#include <WiFi.h>

const char* ssid = "{{ssid}}";
const char* password = "{{password}}";

void setup() {
  WiFi.begin(ssid, password);
}
```

After applying with vars `{"ssid": "MyWiFi", "password": "Pass123"}`:

```cpp
#include <WiFi.h>

const char* ssid = "MyWiFi";
const char* password = "Pass123";

void setup() {
  WiFi.begin(ssid, password);
}
```

---

## Breakpoint Debugging

Full breakpoint support for step-through debugging.

### Features

#### ToggleBreakpoint(line_number)
Toggle breakpoint on/off for specified line

#### AddBreakpoint(line_number)
Add breakpoint to line

#### RemoveBreakpoint(line_number)
Remove breakpoint from line

#### ClearAllBreakpoints()
Remove all breakpoints

#### GetBreakpoints()
Returns list of all breakpoint line numbers

#### HasBreakpoint(line_number)
Check if line has a breakpoint

### Usage Example

```cpp
TextEditor editor;

// Set breakpoint on line 42
editor.AddBreakpoint(42);

// Toggle breakpoint (turns on/off)
editor.ToggleBreakpoint(15);

// Check for breakpoint
if (editor.HasBreakpoint(42)) {
    std::cout << "Breakpoint at line 42" << std::endl;
}

// Get all breakpoints
auto breakpoints = editor.GetBreakpoints();
for (auto line : breakpoints) {
    std::cout << "Breakpoint at line " << line << std::endl;
}

// Clear all
editor.ClearAllBreakpoints();
```

### Visual Indicators

Breakpoints are displayed in the editor with visual markers:
- 🔴 Red dot on the left margin
- Highlighted line background
- Line number in bold

---

## Variable Watch Expressions

Monitor variables in real-time during debugging sessions.

### Features

#### AddWatchVariable(name, type)
Add a variable to the watch list:
```cpp
serial_monitor.AddWatchVariable("temperature", "float");
serial_monitor.AddWatchVariable("wifi_status", "int");
serial_monitor.AddWatchVariable("heap_free", "size_t");
```

#### RemoveWatchVariable(name)
Remove variable from watch list

#### GetWatchVariables()
Returns list of all watched variables with current values

#### UpdateWatchVariable(name, value)
Update variable value (typically called when receiving data from device)

### Watch Variable Structure

```cpp
struct WatchVariable {
    std::string name;        // Variable name
    std::string value;       // Current value
    std::string type;        // Data type
    long long last_update;   // Timestamp of last update
};
```

### Usage Example

```cpp
SerialMonitor monitor;

// Add variables to watch
monitor.AddWatchVariable("temperature", "float");
monitor.AddWatchVariable("led_state", "bool");

// During debugging, update values
monitor.UpdateWatchVariable("temperature", "25.3");
monitor.UpdateWatchVariable("led_state", "true");

// Display watch list
auto watches = monitor.GetWatchVariables();
for (const auto& var : watches) {
    std::cout << var.name << " (" << var.type << ") = " << var.value << std::endl;
}
```

### Example Output

```
Variable Watch List:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
temperature (float)    = 25.3
led_state (bool)       = true
heap_free (size_t)     = 280000
wifi_status (int)      = 3
loop_count (unsigned)  = 1542
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Last Update: 2025-11-04 02:46:41
```

---

## Memory Profiling

Real-time memory monitoring and leak detection for ESP32.

### Memory Profile Structure

```cpp
struct MemoryProfile {
    size_t free_heap;              // Free heap in bytes
    size_t total_heap;             // Total heap size
    size_t free_psram;             // Free PSRAM (if available)
    size_t largest_free_block;     // Largest contiguous free block
    float fragmentation_percent;   // Memory fragmentation %
    std::vector<std::string> warnings;  // Memory warnings
};
```

### Features

#### StartMemoryProfiling()
Start continuous memory profiling

#### GetMemoryProfile()
Get current memory snapshot

#### GetMemoryHistory()
Returns historical memory snapshots

#### StopMemoryProfiling()
Stop profiling

### Memory Warnings

| Condition | Warning Level | Threshold |
|-----------|---------------|-----------|
| Free heap < 20KB | CRITICAL | < 20,000 bytes |
| Free heap < 50KB | WARNING | < 50,000 bytes |
| Fragmentation > 30% | WARNING | > 30% |

### Usage Example

```cpp
SerialMonitor monitor;

// Start profiling
monitor.StartMemoryProfiling();

// Get current state
auto profile = monitor.GetMemoryProfile();

std::cout << "Memory Status:" << std::endl;
std::cout << "Free Heap: " << profile.free_heap << " bytes" << std::endl;
std::cout << "Total Heap: " << profile.total_heap << " bytes" << std::endl;
std::cout << "Largest Block: " << profile.largest_free_block << " bytes" << std::endl;
std::cout << "Fragmentation: " << profile.fragmentation_percent << "%" << std::endl;

// Check warnings
for (const auto& warning : profile.warnings) {
    std::cout << "⚠ " << warning << std::endl;
}

// Get historical data
auto history = monitor.GetMemoryHistory();
for (size_t i = 0; i < history.size(); i++) {
    std::cout << "Sample " << i << ": " << history[i].free_heap << " bytes free" << std::endl;
}
```

### Visualization

Memory profiling data can be visualized as:
- Line graph showing heap usage over time
- Bar chart of fragmentation
- Alert panel for warnings

---

## Performance Analysis

Comprehensive code performance analysis with scoring system.

### Performance Metrics Structure

```cpp
struct PerformanceMetrics {
    size_t code_lines;                           // Total lines of code
    size_t estimated_ram_usage;                  // Estimated RAM (bytes)
    size_t estimated_flash_usage;                // Estimated Flash (bytes)
    std::vector<std::string> warnings;           // Performance warnings
    std::vector<std::string> optimization_suggestions;  // Suggestions
    int performance_score;                       // Score 0-100
};
```

### Performance Scoring

Base score: 100 points

Deductions:
- Blocking delay() in loop(): -10 points
- Each String object: -5 points
- Serial.print() in loops: -15 points
- Multiple analogRead(): -5 points
- delay() in ISR: -30 points (CRITICAL)
- WiFi without status check: -10 points
- High RAM usage (>200KB): -20 points

### Usage Example

```cpp
ESP32Compiler compiler;

// Analyze code
auto metrics = compiler.AnalyzePerformance(code);

// Display results
std::cout << "Performance Analysis Results:" << std::endl;
std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
std::cout << "Performance Score: " << metrics.performance_score << "/100" << std::endl;
std::cout << "Lines of Code: " << metrics.code_lines << std::endl;
std::cout << "Estimated RAM: " << metrics.estimated_ram_usage << " bytes" << std::endl;
std::cout << "Estimated Flash: " << metrics.estimated_flash_usage << " bytes" << std::endl;

std::cout << "\nWarnings:" << std::endl;
for (const auto& warning : metrics.warnings) {
    std::cout << "⚠ " << warning << std::endl;
}

std::cout << "\nOptimization Suggestions:" << std::endl;
for (const auto& suggestion : metrics.optimization_suggestions) {
    std::cout << "💡 " << suggestion << std::endl;
}
```

### Example Output

```
Performance Analysis Results:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Performance Score: 75/100
Lines of Code: 45
Estimated RAM: 8192 bytes
Estimated Flash: 15360 bytes

Warnings:
⚠ Blocking delay() calls detected in loop()
⚠ String objects detected - may cause memory fragmentation

Optimization Suggestions:
💡 Consider using millis() for non-blocking timing
💡 Consider using char arrays for better memory management
```

---

## Code Completion

Intelligent, context-aware code completion system.

### Completion Item Structure

```cpp
struct CompletionItem {
    std::string text;           // Completion text
    std::string description;    // What it does
    std::string insert_text;    // Text to insert
    int priority;               // Priority (0-100)
};
```

### Features

#### GetCompletionsAtCursor()
Returns list of relevant completions based on cursor position and current line

#### GetCompletionSuggestions(code, position, line)
Advanced completions with full code context

### Completion Categories

| Category | Trigger Keywords | Completions |
|----------|------------------|-------------|
| GPIO | pin, gpio | pinMode, digitalWrite, digitalRead, analogRead |
| Serial | serial | Serial.begin, Serial.println, Serial.print |
| WiFi | wifi | WiFi.begin, WiFi.status, WiFi.localIP |
| Timing | delay, time | delay, millis, micros |
| Setup | empty file | void setup(), void loop() |

### Usage Example

```cpp
TextEditor editor;

// Get completions at current cursor position
auto completions = editor.GetCompletionsAtCursor();

// Display as dropdown menu
for (const auto& item : completions) {
    std::cout << item.text << " - " << item.description << std::endl;
}

// User selects completion, insert the text
if (!completions.empty()) {
    editor.InsertText(completions[0].insert_text, cursor_position);
}
```

### Example Completions

When typing `"Serial"`:
```
Serial.begin(115200);     - Initialize serial           [Priority: 95]
Serial.println();         - Print line to serial        [Priority: 90]
Serial.print();           - Print to serial             [Priority: 88]
Serial.available()        - Check if data available     [Priority: 85]
Serial.read()             - Read one byte               [Priority: 84]
```

When typing `"pin"`:
```
pinMode(pin, OUTPUT);     - Set pin as output           [Priority: 90]
pinMode(pin, INPUT);      - Set pin as input            [Priority: 90]
digitalWrite(pin, HIGH);  - Set pin HIGH                [Priority: 88]
digitalRead(pin)          - Read digital pin            [Priority: 87]
analogRead(pin)           - Read analog pin             [Priority: 86]
```

### AI-Powered Completions

For advanced completions, the AI Assistant provides:
- Multi-line code snippets
- Complete function implementations
- Error-free, tested code patterns
- ESP32-specific best practices

---

## Integration Guide

### Using Multiple Features Together

```cpp
// Complete workflow example
AIAssistant ai;
FileManager fm;
TextEditor editor;
ESP32Compiler compiler;
SerialMonitor monitor;

// 1. Start with template
auto templates = fm.GetTemplates();
std::map<std::string, std::string> vars = {{"pin", "13"}};
std::string code = fm.ApplyTemplate("led_blink", vars);

// 2. Load into editor
editor.SetText(code);

// 3. Add breakpoint
editor.AddBreakpoint(5);

// 4. Detect bugs
auto bugs = ai.DetectBugs(code);
if (!bugs.empty()) {
    code = ai.AutoFixBugs(code);
    editor.SetText(code);
}

// 5. Optimize code
code = ai.OptimizeCode(code);
editor.SetText(code);

// 6. Analyze performance
auto metrics = compiler.AnalyzePerformance(code);
std::cout << "Performance Score: " << metrics.performance_score << std::endl;

// 7. Start debugging with watch variables
monitor.Connect("/dev/ttyUSB0", 115200);
monitor.AddWatchVariable("led_state", "bool");
monitor.StartMemoryProfiling();

// 8. Monitor during execution
auto profile = monitor.GetMemoryProfile();
auto watches = monitor.GetWatchVariables();
```

---

## Best Practices

### Code Refactoring
1. Run bug detection before refactoring
2. Apply specific refactoring types based on needs
3. Review AI suggestions before applying
4. Test code after refactoring

### Bug Detection
1. Run bug detection regularly during development
2. Fix critical bugs immediately
3. Address warnings when possible
4. Use auto-fix for common issues

### Templates
1. Create templates for repeated patterns
2. Use descriptive variable names
3. Tag templates for easy filtering
4. Share templates with team

### Debugging
1. Set breakpoints at key decision points
2. Watch critical variables
3. Monitor memory during loops
4. Check performance scores regularly

### Performance
1. Aim for scores > 80
2. Address critical warnings first
3. Profile memory usage for long-running code
4. Use optimization suggestions

---

## API Reference

### AIAssistant

```cpp
// Refactoring
std::string RefactorCode(const std::string& code, const std::string& type);
std::string OptimizeCode(const std::string& code);
std::string ImproveReadability(const std::string& code);

// Bug Detection
std::vector<BugReport> DetectBugs(const std::string& code);
std::string AutoFixBugs(const std::string& code);

// Code Completion
std::vector<CompletionSuggestion> GetCompletionSuggestions(
    const std::string& code, int cursor_position, const std::string& line);
```

### FileManager

```cpp
// Templates
void AddTemplate(const std::string& name, const std::string& code,
                const std::string& description, const std::vector<std::string>& tags);
bool DeleteTemplate(const std::string& name);
std::vector<CodeTemplate> GetTemplates() const;
std::vector<CodeTemplate> GetTemplatesByTag(const std::string& tag) const;
std::string ApplyTemplate(const std::string& name,
                         const std::map<std::string, std::string>& vars);
```

### TextEditor

```cpp
// Breakpoints
void ToggleBreakpoint(size_t line_number);
void AddBreakpoint(size_t line_number);
void RemoveBreakpoint(size_t line_number);
void ClearAllBreakpoints();
std::vector<size_t> GetBreakpoints() const;
bool HasBreakpoint(size_t line_number) const;

// Completion
std::vector<CompletionItem> GetCompletionsAtCursor() const;
```

### SerialMonitor

```cpp
// Memory Profiling
MemoryProfile GetMemoryProfile() const;
void StartMemoryProfiling();
void StopMemoryProfiling();
std::vector<MemoryProfile> GetMemoryHistory() const;

// Variable Watch
void AddWatchVariable(const std::string& name, const std::string& type);
void RemoveWatchVariable(const std::string& name);
std::vector<WatchVariable> GetWatchVariables() const;
void UpdateWatchVariable(const std::string& name, const std::string& value);
```

### ESP32Compiler

```cpp
// Performance Analysis
PerformanceMetrics AnalyzePerformance(const std::string& code);
```

---

## Troubleshooting

### Common Issues

**Issue**: Code completion not showing
- **Solution**: Check that cursor is on a line with recognized keywords

**Issue**: Bug detection false positives
- **Solution**: Review suggested fixes, not all warnings require action

**Issue**: Performance score seems low
- **Solution**: Review optimization suggestions, some deductions are minor

**Issue**: Breakpoints not hit
- **Solution**: Ensure code is compiled and uploaded after setting breakpoints

**Issue**: Memory profiling shows N/A
- **Solution**: Connect to device and start profiling first

---

## Version Information

- **Feature Set Version**: 2.0
- **Implementation Date**: November 2025
- **Compatibility**: ESP32 Driver IDE v1.0.0+

---

## See Also

- [AI Code Generation Guide](AI_CODE_GENERATION.md)
- [ImGui Interface Guide](IMGUI_GUIDE.md)
- [Build Guide](BUILD_GUIDE.md)
- [README](README.md)
