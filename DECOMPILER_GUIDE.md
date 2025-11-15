# ESP32 Advanced Decompiler Guide

## Overview

The ESP32 Advanced Decompiler is a powerful tool for analyzing ESP32 firmware binaries and generating high-quality pseudo-code. It implements state-of-the-art decompilation techniques specifically optimized for the Xtensa architecture used in ESP32 microcontrollers.

## Features

### Core Decompilation Features

#### 1. Control Flow Graph (CFG) Analysis
- **Basic Block Construction**: Automatically identifies basic blocks based on control flow instructions
- **Edge Linking**: Builds accurate control flow edges between blocks
- **Dominator Tree Computation**: Computes dominators using iterative algorithm
- **Natural Loop Detection**: Identifies loops by finding back edges in the CFG
- **CFG Simplification**: Optimizes control flow graph structure

#### 2. Data Flow Analysis (DFA)
- **Reaching Definitions**: Tracks which definitions reach each program point
- **Live Variable Analysis**: Determines which variables are live at each point
- **Use-Def Chains**: Connects variable uses to their definitions
- **Type Inference**: Automatically infers variable types from operations

#### 3. Instruction Disassembly
- **Xtensa ISA Support**: Decodes Xtensa instruction set architecture
- **Multiple Instruction Formats**: Handles QRST, load/store, immediate, shift, call, and branch formats
- **Operand Extraction**: Accurately extracts register and immediate operands
- **Symbolic References**: Resolves function and data references

#### 4. Pattern Matching
- **ESP32 API Recognition**: Detects GPIO, WiFi, Bluetooth, UART, I2C, SPI, Timer, and NVS APIs
- **FreeRTOS Patterns**: Identifies task creation, delays, queues, semaphores, and mutexes
- **Common Code Patterns**: Recognizes delay loops, serial prints, GPIO operations

#### 5. Pseudo-Code Generation
- **Structured Control Flow**: Generates if/else, while, and for loops
- **Variable Declarations**: Creates properly typed variable declarations
- **Function Calls**: Reconstructs function call sequences
- **Expression Simplification**: Simplifies complex expressions

#### 6. ESP32-Specific Analysis
- **Interrupt Handler Detection**: Identifies ISRs by analyzing special register access
- **FreeRTOS Task Detection**: Recognizes task entry points and infinite loops
- **API Usage Analysis**: Tracks which ESP32 APIs are used and how often

### Output Formats

The decompiler supports multiple output formats:

1. **C-Style**: C-like pseudo-code with full syntax
2. **Pseudo-Code**: Simplified high-level representation
3. **Assembly Annotated**: Assembly with high-level comments
4. **HTML**: Formatted output with syntax highlighting
5. **Markdown**: Documentation-friendly format

## Usage

### Basic Usage

```cpp
#include "decompiler/advanced_decompiler.h"

using namespace esp32_ide::decompiler;

// Create decompiler instance
AdvancedDecompiler decompiler;

// Initialize
decompiler.Initialize();

// Load firmware
if (!decompiler.LoadFirmware("firmware.bin")) {
    std::cerr << "Failed to load firmware\n";
    return;
}

// Decompile all functions
decompiler.DecompileAll();

// Get results
std::string pseudo_code = decompiler.GetFullPseudoCode();
std::cout << pseudo_code << std::endl;
```

### Advanced Usage

#### Progress Monitoring

```cpp
decompiler.SetProgressCallback([](int percent, const std::string& status) {
    std::cout << "[" << percent << "%] " << status << "\n";
});
```

#### Verbose Output

```cpp
decompiler.SetVerboseOutput(true);
decompiler.SetOptimizationLevel(2); // 0-3, higher = more optimization
```

#### Function-Specific Decompilation

```cpp
// Get specific function
Function* func = decompiler.GetFunction(0x400C0000);
if (func) {
    std::string code = decompiler.GetPseudoCode(0x400C0000);
    std::cout << code << std::endl;
}
```

#### ESP32-Specific Analysis

```cpp
// Detect ESP32 APIs
decompiler.DetectESP32APIs();
decompiler.DetectFreeRTOSTasks();
decompiler.DetectInterruptHandlers();

// Get API usage statistics
auto api_usage = decompiler.GetESP32APIUsage();
for (const auto& [api, count] : api_usage) {
    std::cout << api << ": " << count << "\n";
}
```

#### String and Constant Extraction

```cpp
// Extract strings from firmware
auto strings = decompiler.ExtractStrings();
for (const auto& str : strings) {
    std::cout << "Found string: " << str << "\n";
}

// Extract constants
auto constants = decompiler.ExtractConstants();
for (auto constant : constants) {
    std::cout << "Constant: 0x" << std::hex << constant << std::dec << "\n";
}
```

#### Custom Output Formatting

```cpp
// Create formatter with specific settings
DecompilerOutput formatter(DecompilerOutput::Format::C_STYLE);
formatter.SetIndentSize(4);
formatter.SetShowAddresses(true);
formatter.SetShowComments(true);

// Format all functions
const auto& functions = decompiler.GetFunctions();
std::string output = formatter.FormatFullProgram(functions);
std::cout << output << std::endl;
```

## Architecture

### Class Hierarchy

```
AdvancedDecompiler
├── PatternMatcher
├── DataFlowAnalysis
├── DecompilerOutput
└── Functions (contains)
    ├── ControlFlowGraph
    │   └── BasicBlocks
    │       └── Instructions
    └── Metadata (ISR, Task, etc.)
```

### Key Data Structures

#### Instruction
```cpp
struct Instruction {
    uint32_t address;
    uint32_t opcode;
    std::string mnemonic;
    std::vector<std::string> operands;
    std::string comment;
    
    bool IsJump() const;
    bool IsCall() const;
    bool IsReturn() const;
    bool IsBranch() const;
};
```

#### BasicBlock
```cpp
struct BasicBlock {
    uint32_t start_address;
    uint32_t end_address;
    std::vector<Instruction> instructions;
    std::vector<BasicBlock*> successors;
    std::vector<BasicBlock*> predecessors;
};
```

#### Function
```cpp
struct Function {
    uint32_t start_address;
    uint32_t end_address;
    std::string name;
    std::string return_type;
    std::vector<std::string> parameters;
    std::vector<std::string> local_variables;
    std::unique_ptr<ControlFlowGraph> cfg;
    std::string pseudo_code;
    
    // ESP32-specific
    bool is_isr;
    bool is_task;
    std::string task_priority;
};
```

## Algorithms

### Control Flow Graph Construction

1. **Identify Leaders**: First instruction, targets of jumps/branches, instructions after branches
2. **Create Basic Blocks**: Group instructions between leaders
3. **Build Edges**: Connect blocks based on control flow instructions
4. **Identify Entry/Exit**: Mark entry block and exit blocks

### Dominator Computation

Uses iterative algorithm:
1. Initialize entry block dominates only itself
2. All other blocks dominated by all blocks initially
3. Iteratively compute: `Dom(n) = {n} ∪ (∩ Dom(p) for all predecessors p)`
4. Continue until fixed point

### Loop Detection

1. Find back edges (edges to dominators)
2. For each back edge (tail → header):
   - Header and tail are in loop
   - Find all nodes that can reach tail without going through header
3. Mark loop headers for structured code generation

### Data Flow Analysis

#### Reaching Definitions
- Forward analysis
- `OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])`
- `IN[B] = ∪ OUT[P] for all predecessors P`

#### Live Variables
- Backward analysis
- `IN[B] = USE[B] ∪ (OUT[B] - DEF[B])`
- `OUT[B] = ∪ IN[S] for all successors S`

### Type Inference

Infers types based on:
- Integer operations (add, sub, mul) → `int`
- Floating point operations (fadd, fsub) → `float`
- Load/store operations → appropriate size type
- Pointer operations → pointer types
- GPIO operations → `gpio_num_t`

## ESP32 Detection Patterns

### GPIO Operations

```
Pattern: movi + s32i to GPIO register
High-level: gpio_set_level(pin, level);
```

### FreeRTOS Delays

```
Pattern: movi + call vTaskDelay
High-level: vTaskDelay(pdMS_TO_TICKS(ms));
```

### Serial/UART

```
Pattern: movi + movi + call uart_write
High-level: uart_write_bytes(uart, data, len);
```

### WiFi Connection

```
Pattern: movi + movi + call WiFi.begin
High-level: WiFi.begin(ssid, password);
```

### Task Detection

Identified by:
- Calls to `xTaskCreate`
- Infinite loops (back edges in CFG)
- No return paths

### ISR Detection

Identified by:
- Special register access (rsr, wsr, xsr)
- Interrupt register manipulation
- GPIO/Timer interrupt patterns
- Short function length

## Performance Considerations

### Optimization Levels

- **Level 0**: No optimization, show all details
- **Level 1**: Basic optimization, remove obvious redundancy
- **Level 2**: Standard optimization (default)
- **Level 3**: Aggressive optimization, may sacrifice accuracy

### Memory Usage

- CFG construction: O(n) where n = number of instructions
- Dominator computation: O(n²) worst case
- Data flow analysis: O(n × d) where d = number of definitions

### Processing Time

For a typical ESP32 firmware:
- Disassembly: ~100,000 instructions/second
- CFG construction: ~50,000 instructions/second
- Data flow analysis: ~25,000 instructions/second
- Pseudo-code generation: ~10,000 instructions/second

## Limitations

1. **Indirect Jumps**: May not accurately resolve computed jump targets
2. **Dynamic Code**: Cannot handle self-modifying code
3. **Obfuscation**: Limited support for deliberately obfuscated code
4. **Optimization Recovery**: May not perfectly recover optimized code
5. **Type Inference**: Conservative typing may result in generic types

## Best Practices

### For Accurate Decompilation

1. **Use Symbols**: Provide symbol table when available
2. **Set Entry Point**: Explicitly set correct entry point
3. **Enable Verbose**: Use verbose mode for debugging
4. **Validate Output**: Cross-reference with known code sections

### For Better Performance

1. **Limit Scope**: Decompile specific functions rather than entire firmware
2. **Adjust Optimization**: Lower optimization level for faster processing
3. **Disable Features**: Turn off unused analysis passes

### For Integration

1. **Use Callbacks**: Implement progress callbacks for long operations
2. **Error Handling**: Always check return values
3. **Memory Management**: Be mindful of large firmware files

## Examples

### Complete Example: Analyzing ESP32 Firmware

```cpp
#include "decompiler/advanced_decompiler.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <firmware.bin>\n";
        return 1;
    }
    
    // Create decompiler
    esp32_ide::decompiler::AdvancedDecompiler decompiler;
    
    // Set options
    decompiler.SetVerboseOutput(true);
    decompiler.SetOptimizationLevel(2);
    
    // Progress callback
    decompiler.SetProgressCallback([](int percent, const std::string& status) {
        std::cout << "\r[" << percent << "%] " << status << std::flush;
    });
    
    // Load firmware
    if (!decompiler.LoadFirmware(argv[1])) {
        std::cerr << "Failed to load firmware\n";
        return 1;
    }
    
    // Decompile
    if (!decompiler.DecompileAll()) {
        std::cerr << "Decompilation failed\n";
        return 1;
    }
    
    std::cout << "\n\nDecompilation complete!\n\n";
    
    // Perform ESP32-specific analysis
    decompiler.DetectESP32APIs();
    decompiler.DetectFreeRTOSTasks();
    decompiler.DetectInterruptHandlers();
    
    // Display results
    const auto& functions = decompiler.GetFunctions();
    std::cout << "Found " << functions.size() << " functions\n\n";
    
    // Show ESP32 API usage
    auto api_usage = decompiler.GetESP32APIUsage();
    std::cout << "ESP32 API Usage:\n";
    for (const auto& [api, count] : api_usage) {
        std::cout << "  " << api << ": " << count << "\n";
    }
    std::cout << "\n";
    
    // Generate pseudo-code
    std::string code = decompiler.GetFullPseudoCode();
    
    // Write to file
    std::ofstream out("decompiled.c");
    out << code;
    out.close();
    
    std::cout << "Pseudo-code written to decompiled.c\n";
    
    return 0;
}
```

### Example: Custom Pattern Matching

```cpp
// Add custom pattern
esp32_ide::decompiler::PatternMatcher matcher;

esp32_ide::decompiler::PatternMatcher::Pattern custom_pattern;
custom_pattern.name = "custom_delay";
custom_pattern.instruction_pattern = {"movi", "loop", "subi", "bnez"};
custom_pattern.high_level_code = "delay_microseconds(us);";

matcher.AddPattern(custom_pattern);

// Use matcher
std::vector<esp32_ide::decompiler::Instruction> instructions = /* ... */;
std::string result = matcher.MatchPattern(instructions);
```

## Testing

Run the comprehensive test suite:

```bash
cd build
./esp32-decompiler-test
```

This will test:
- CFG construction
- Data flow analysis
- Pattern matching
- Full decompilation
- ESP32 API detection
- Pseudo-code generation
- Output formatting

## Contributing

When adding new features:

1. **Add Tests**: Include test cases for new functionality
2. **Document**: Update this guide with new features
3. **Patterns**: Add ESP32-specific patterns to PatternMatcher
4. **Optimize**: Profile and optimize hot paths

## Version History

### Version 1.0 (November 2025)
- Initial release with full decompilation features
- Complete CFG and DFA implementation
- ESP32-specific pattern recognition
- Multiple output formats
- Comprehensive test suite

## See Also

- [Advanced Features Guide](ADVANCED_FEATURES.md)
- [Build Guide](BUILD_GUIDE.md)
- [README](README.md)

## References

1. "Compilers: Principles, Techniques, and Tools" (Dragon Book)
2. "Advanced Compiler Design and Implementation" by Steven Muchnick
3. Xtensa Instruction Set Architecture Reference Manual
4. ESP-IDF Programming Guide
5. FreeRTOS API Reference
