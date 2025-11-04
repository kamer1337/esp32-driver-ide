# ESP32 Driver IDE - New Features Guide

## Overview

This document describes the newly implemented advanced features for the ESP32 Driver IDE, including integrated terminal, board management, device schematics, and enhanced syntax highlighting.

## New Features

### 1. Integrated Terminal

A built-in terminal panel for executing commands directly within the IDE.

**Location:** Bottom-right panel of the IDE

**Features:**
- Command history with color-coded output
- Error highlighting (red)
- Warning highlighting (yellow)
- Command prompts (green)

**Available Commands:**
- `help` - Display available commands
- `clear` - Clear terminal output
- `compile` - Compile current code
- `upload` - Upload code to device
- `ports` - List available serial ports
- `boards` - List detected boards
- `ls` - List files in project

**Usage:**
Type a command and press Enter to execute it.

---

### 2. Board List Panel

A dedicated panel for managing and viewing connected ESP32 boards.

**Location:** Left panel, "Boards" tab (alongside "Files" tab)

**Features:**
- Scan for connected ESP32 devices
- Display board information:
  - Board name (e.g., ESP32-DevKit, ESP32-S3-DevKit)
  - Chip type (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
  - Serial port
  - Flash size (MB)
  - RAM size (KB)
  - Connection status
- Select boards for operations
- Quick access to device schematic viewer

**Usage:**
1. Click "Scan for Boards" to detect connected devices
2. Select a board from the list to view details
3. Click "Show Schematic" to view the device pinout

---

### 3. Device Schematic Viewer

A floating window displaying detailed schematics for ESP32 boards.

**Access:** Click "Show Schematic" button in the Board List panel

**Views:**

#### Pinout View
Complete GPIO pinout diagram showing:
- All GPIO pins with their alternate functions
- Power pins (3.3V, GND, VIN)
- Special function pins (ADC, DAC, Touch, I2C, SPI, UART)
- Pin capabilities and restrictions

#### Block Diagram View
System architecture overview showing:
- CPU specifications (Dual-core Xtensa LX6 @ 240 MHz)
- Memory configuration (ROM, SRAM)
- Wireless capabilities (WiFi, Bluetooth)
- Peripheral interfaces (UART, SPI, I2C, PWM, etc.)
- Security features

**Usage:**
- Switch between tabs to view different schematic information
- Useful for understanding pin assignments and hardware capabilities
- Reference for hardware design and debugging

---

### 4. Advanced Syntax Highlighting

Real-time syntax highlighting with color-coded code display.

**Location:** Editor panel with toggle option

**Features:**
- Split-view editor:
  - Top: Editable text area
  - Bottom: Live syntax-highlighted preview
- Color scheme:
  - **Purple** - Keywords (if, for, while, return, etc.)
  - **Cyan** - Types (int, void, bool, etc.)
  - **Yellow** - Functions
  - **Orange** - Strings
  - **Green** - Comments
  - **Light Blue** - Numbers
  - **Red** - Preprocessor directives (#include, #define)

**Usage:**
1. Check "Syntax Highlighting" checkbox in the editor tab
2. Edit code in the top pane
3. View colored preview in the bottom pane
4. Uncheck to return to full-screen plain text editor

**Benefits:**
- Easier code reading and debugging
- Quick identification of syntax elements
- Professional IDE experience

---

### 5. Enhanced Properties Panel

The properties panel now includes three tabs for different functionality.

**Tabs:**

#### Editor Tab (Default)
- File information
- Editor settings
- Board configuration
- Apply settings button

#### Templates Tab
- Browse available code templates:
  - Basic Arduino sketch
  - LED blink
  - WiFi connection
  - Web server
  - Sensor reading
- One-click template insertion
- Template descriptions

#### Completion Tab
Code completion snippets organized by category:

**GPIO Functions:**
- pinMode()
- digitalWrite()
- digitalRead()
- analogRead()

**Serial Functions:**
- Serial.begin()
- Serial.println()
- Serial.print()

**WiFi Functions:**
- WiFi.begin()
- WiFi.status()
- WiFi.localIP()

**Timing Functions:**
- delay()
- millis()
- micros()

**Usage:**
- Click any function button to insert it at the cursor position
- Modify parameters as needed

---

## Layout Overview

```
┌─────────────────────────────────────────────────────────────┐
│  Menu Bar                                                     │
├─────────────────────────────────────────────────────────────┤
│  Toolbar (USB Port, Upload, Download, Debug, RE)             │
├──────────┬─────────────────────────┬──────────┬─────────────┤
│          │                         │          │             │
│  Files/  │   Editor                │  Props   │  AI         │
│  Boards  │   Debugger              │  Editor  │  Assistant  │
│          │   Reverse Engineering   │  Temps   │             │
│          │                         │  Compl   │             │
├──────────┴────────────────┬────────┴──────────┴─────────────┤
│                           │                                  │
│  Console                  │  Terminal                        │
│                           │                                  │
└───────────────────────────┴──────────────────────────────────┘
```

**Panel Descriptions:**
- **Left:** File explorer and Board list (tabs)
- **Center:** Editor with syntax highlighting, Debugger, Reverse Engineering (tabs)
- **Right (inner):** Editor properties, Templates, Completion (tabs)
- **Right (outer):** AI Assistant
- **Bottom Left:** Console output
- **Bottom Right:** Integrated terminal
- **Floating:** Device schematic window (when activated)

---

## Integration with Existing Features

### Templates
- Pre-existing template system (from FileManager)
- Now accessible through UI in Properties Panel
- 5+ built-in templates ready to use

### Code Completion
- Pre-existing completion system (from TextEditor/AIAssistant)
- Now accessible through UI in Properties Panel
- Organized by function category

### Multiple Editor Windows
- Already implemented as tabs
- Enhanced with syntax highlighting preview
- Reorderable tabs
- Modified indicator (*)

---

## Technical Details

**Technology Stack:**
- ImGui for UI rendering
- SyntaxHighlighter class for token-based parsing
- Zero external dependencies beyond existing libraries

**Performance:**
- Syntax highlighting uses efficient token-based parsing
- Preview updates only when code changes
- Minimal performance impact

**Compatibility:**
- Works on all platforms (Linux, macOS, Windows)
- Requires OpenGL 3.3+ for ImGui rendering

---

## Future Enhancements

Potential improvements for future versions:
- Real-time syntax highlighting in edit mode (requires custom ImGui text editor)
- More sophisticated board detection with chip ID reading
- Actual firmware download and analysis from devices
- Vector graphics for schematic diagrams
- Custom template creation UI
- Configurable syntax highlighting colors
- Auto-completion with IntelliSense-style popup

---

## Troubleshooting

**Syntax highlighting not working:**
- Ensure "Syntax Highlighting" checkbox is enabled
- Verify that SyntaxHighlighter is properly initialized

**Board list is empty:**
- Click "Scan for Boards" button
- Check that devices are properly connected
- Verify USB permissions on Linux

**Terminal commands not executing:**
- Ensure command is spelled correctly
- Type "help" to see available commands
- Check console output for error messages

**Schematic window not showing:**
- Click "Show Schematic" button in Board List panel
- Check that window is not minimized or hidden
- Reopen from Board List panel if needed

---

## Keyboard Shortcuts

Currently supported shortcuts in editor:
- **Tab** - Insert tab character
- **Ctrl+Tab** - Switch between editor tabs
- **Ctrl+W** - Close current tab

Future shortcuts (planned):
- **Ctrl+S** - Save file
- **Ctrl+Space** - Trigger code completion
- **F5** - Compile and upload
- **F9** - Toggle breakpoint

---

## Documentation

For more information, see:
- [README.md](README.md) - Main project documentation
- [ADVANCED_FEATURES.md](ADVANCED_FEATURES.md) - Advanced AI features
- [AI_CODE_GENERATION.md](AI_CODE_GENERATION.md) - AI code generation guide
- [IMGUI_GUIDE.md](IMGUI_GUIDE.md) - ImGui interface guide

---

## Feedback and Contributions

We welcome feedback and contributions! If you have ideas for new features or improvements:
1. Open an issue on GitHub
2. Submit a pull request
3. Join our community discussions

---

**Last Updated:** November 2025
**Version:** 1.1.0
