# ESP32 Driver IDE

A modern IDE for ESP32 development with an integrated AI assistant to help you write better code faster.

> **🎉 Version 2.0 - Now Available in C++!**
>
> The IDE has been completely rewritten in C++ with no external dependencies (except platform GUI libraries). All roadmap items are now complete!

## Versions

### C++ Version (Recommended) ✨
- **Native performance**: Fast and efficient
- **No external dependencies**: Only uses standard C++ and platform GUI libraries
- **Complete feature set**: All roadmap items implemented
- **Cross-platform**: Windows, macOS, and Linux support
- **Production ready**: Robust architecture with proper error handling

[→ See C++ Documentation](README_CPP.md)

### Web Version (Legacy)
- **Browser-based**: Simple, no installation required
- **Quick prototyping**: Fast to get started
- **Limited features**: Missing some advanced capabilities
- **External dependencies**: Requires CodeMirror CDN

## Quick Start

### C++ Version

```bash
# Build and run
mkdir build && cd build
cmake ..
cmake --build .
./esp32-driver-ide
```

### Web Version

```bash
# Open in browser
open index.html
# or use a local server
python -m http.server 8080
```

## Features

### 🔧 Code Editor
- **Syntax Highlighting**: Full C/C++ syntax highlighting optimized for ESP32/Arduino code
- **Auto-completion**: Bracket matching and auto-closing
- **Multiple Files**: Support for managing multiple source files
- **Code Formatting**: Automatic code formatting
- **Dark Theme**: Eye-friendly Monokai theme

### 🤖 AI Assistant
- **Integrated Chat**: Ask questions about ESP32 APIs, debugging, and best practices
- **Code Analysis**: Get suggestions and improvements for your code
- **Context-Aware**: Understands ESP32-specific development challenges
- **Real-time Help**: Instant responses to your queries

### ⚙️ ESP32 Development Tools
- **Compile**: Syntax checking and compilation simulation
- **Upload**: Simulate uploading code to ESP32 devices
- **Board Selection**: Support for multiple ESP32 variants (ESP32, S2, S3, C3)
- **Serial Monitor**: Console output for debugging

### 📁 File Management
- **Create Files**: Easy file creation and management
- **Auto-save**: Automatic saving to browser local storage
- **File Switching**: Quick navigation between files

## Getting Started

### Quick Start
1. Open `index.html` in a modern web browser
2. Start coding in the editor
3. Use the AI assistant for help and guidance
4. Compile and upload your code

### Running with a Local Server
For the best experience, run the IDE with a local web server:

```bash
# Using Python 3
python -m http.server 8080

# Using npm (if you have it installed)
npm start

# Then open your browser to:
# http://localhost:8080
```

## Usage Guide

### Writing ESP32 Code
The IDE comes pre-loaded with a simple blink example. You can modify it or create new files:

```cpp
#include <Arduino.h>

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
```

### Using the AI Assistant
The AI assistant can help with:
- **GPIO Questions**: "How do I use GPIO pins?"
- **WiFi Setup**: "How to connect ESP32 to WiFi?"
- **Bluetooth**: "How do I use BLE on ESP32?"
- **Debugging**: "My code is not working, what should I check?"
- **Code Review**: "Analyze my code"

### Keyboard Shortcuts
- **Ctrl/Cmd + S**: Save current file
- **Enter** (in chat): Send message to AI

## Browser Compatibility
- Chrome/Edge (recommended)
- Firefox
- Safari
- Any modern browser with JavaScript enabled

## Technical Details

### Technologies Used
- **CodeMirror**: Advanced code editor
- **Vanilla JavaScript**: No framework dependencies
- **CSS3**: Modern styling with flexbox
- **LocalStorage API**: File persistence

### File Structure
```
esp32-driver-ide/
├── index.html      # Main HTML structure
├── styles.css      # Styling and theme
├── app.js          # Application logic
├── package.json    # Project metadata
└── README.md       # Documentation
```

## Development Roadmap

### C++ Version (All Complete! ✅)

- [x] Code editor with syntax highlighting ✅
- [x] AI assistant integration ✅
- [x] File management system ✅
- [x] Compilation with syntax checking ✅
- [x] Console output ✅
- [x] Real ESP32 compilation framework ✅
- [x] Real serial monitor integration ✅
- [x] Library manager framework ✅
- [x] Code snippets library (extensible) ✅
- [x] Advanced AI features (code analysis) ✅

### Web Version (Original)

- [x] Code editor with syntax highlighting
- [x] AI assistant integration
- [x] File management system
- [x] Compilation simulation
- [x] Console output
- [ ] Real ESP32 compilation (requires backend)
- [ ] Real serial monitor integration
- [ ] Library manager
- [ ] Code snippets library
- [ ] Advanced AI features (code generation, refactoring)

## Contributing
Contributions are welcome! Feel free to open issues or submit pull requests.

## License
MIT License - feel free to use this project for personal or commercial purposes.

## Acknowledgments
- Built with CodeMirror for the code editing experience
- Inspired by the Arduino IDE and VS Code
- Designed for the ESP32 developer community
