// ESP32 Driver IDE - Main Application Logic

// Initialize CodeMirror Editor
let editor;
let currentFileName = 'sketch.ino';
let files = {
    'sketch.ino': `// ESP32 Blink Example
#include <Arduino.h>

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32 Started!");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(1000);
  
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(1000);
}`
};

// Initialize the editor when DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    initializeEditor();
    setupEventListeners();
    addConsoleMessage('IDE initialized successfully', 'success');
});

function initializeEditor() {
    const textarea = document.getElementById('editor');
    editor = CodeMirror.fromTextArea(textarea, {
        mode: 'text/x-c++src',
        theme: 'monokai',
        lineNumbers: true,
        autoCloseBrackets: true,
        matchBrackets: true,
        indentUnit: 2,
        tabSize: 2,
        indentWithTabs: false,
        lineWrapping: true,
    });
    
    // Load initial file
    editor.setValue(files[currentFileName]);
    
    // Auto-save on change
    editor.on('change', function() {
        files[currentFileName] = editor.getValue();
    });
}

function setupEventListeners() {
    // File operations
    document.getElementById('newFileBtn').addEventListener('click', createNewFile);
    document.getElementById('saveBtn').addEventListener('click', saveFile);
    
    // Compile and upload
    document.getElementById('compileBtn').addEventListener('click', compileCode);
    document.getElementById('uploadBtn').addEventListener('click', uploadToESP32);
    
    // Editor controls
    document.getElementById('formatBtn').addEventListener('click', formatCode);
    
    // Console
    document.getElementById('clearConsoleBtn').addEventListener('click', clearConsole);
    
    // Chat AI
    document.getElementById('sendChatBtn').addEventListener('click', sendChatMessage);
    document.getElementById('chatInput').addEventListener('keypress', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendChatMessage();
        }
    });
    document.getElementById('toggleChatBtn').addEventListener('click', toggleChatPanel);
    
    // File list
    document.getElementById('fileList').addEventListener('click', function(e) {
        const fileItem = e.target.closest('.file-item');
        if (fileItem) {
            const fileName = fileItem.getAttribute('data-file');
            openFile(fileName);
        }
    });
}

// File Operations
function createNewFile() {
    const fileName = prompt('Enter new file name:', 'new_file.ino');
    if (fileName && !files[fileName]) {
        files[fileName] = '// New ESP32 file\n#include <Arduino.h>\n\nvoid setup() {\n  \n}\n\nvoid loop() {\n  \n}';
        addFileToList(fileName);
        openFile(fileName);
        addConsoleMessage(`Created new file: ${fileName}`, 'success');
    } else if (files[fileName]) {
        addConsoleMessage(`File ${fileName} already exists`, 'error');
    }
}

function addFileToList(fileName) {
    const fileList = document.getElementById('fileList');
    const fileItem = document.createElement('div');
    fileItem.className = 'file-item';
    fileItem.setAttribute('data-file', fileName);
    fileItem.innerHTML = `
        <span class="file-icon">📝</span>
        <span class="file-name">${fileName}</span>
    `;
    fileList.appendChild(fileItem);
}

function openFile(fileName) {
    if (files[fileName]) {
        // Save current file
        files[currentFileName] = editor.getValue();
        
        // Load new file
        currentFileName = fileName;
        editor.setValue(files[fileName]);
        document.getElementById('currentFile').textContent = fileName;
        
        // Update active state
        document.querySelectorAll('.file-item').forEach(item => {
            item.classList.remove('active');
            if (item.getAttribute('data-file') === fileName) {
                item.classList.add('active');
            }
        });
        
        addConsoleMessage(`Opened file: ${fileName}`);
    }
}

function saveFile() {
    files[currentFileName] = editor.getValue();
    
    // Simulate saving to local storage
    try {
        localStorage.setItem('esp32_ide_files', JSON.stringify(files));
        addConsoleMessage(`Saved ${currentFileName}`, 'success');
    } catch (e) {
        addConsoleMessage('Error saving file: ' + e.message, 'error');
    }
}

// Compile and Upload Functions
function compileCode() {
    const code = editor.getValue();
    const board = document.getElementById('boardSelect').value;
    
    addConsoleMessage('='.repeat(50));
    addConsoleMessage(`Compiling for ${board}...`, 'warning');
    addConsoleMessage('Checking syntax...');
    
    // Simulate compilation
    setTimeout(() => {
        // Basic syntax check
        const openBraces = (code.match(/{/g) || []).length;
        const closeBraces = (code.match(/}/g) || []).length;
        
        if (openBraces !== closeBraces) {
            addConsoleMessage('Compilation failed: Mismatched braces', 'error');
            return;
        }
        
        if (!code.includes('void setup()') || !code.includes('void loop()')) {
            addConsoleMessage('Warning: Missing setup() or loop() function', 'warning');
        }
        
        addConsoleMessage('Sketch uses 234532 bytes (17%) of program storage space.', 'success');
        addConsoleMessage('Global variables use 28784 bytes (8%) of dynamic memory.', 'success');
        addConsoleMessage('Compilation complete!', 'success');
        addConsoleMessage('='.repeat(50));
    }, 1500);
}

function uploadToESP32() {
    const board = document.getElementById('boardSelect').value;
    
    addConsoleMessage('='.repeat(50));
    addConsoleMessage(`Uploading to ${board}...`, 'warning');
    
    // Simulate upload process
    setTimeout(() => {
        addConsoleMessage('Connecting to ESP32...');
    }, 500);
    
    setTimeout(() => {
        addConsoleMessage('Writing at 0x00010000... (10%)');
    }, 1000);
    
    setTimeout(() => {
        addConsoleMessage('Writing at 0x00020000... (50%)');
    }, 2000);
    
    setTimeout(() => {
        addConsoleMessage('Writing at 0x00030000... (100%)');
    }, 3000);
    
    setTimeout(() => {
        addConsoleMessage('Upload successful!', 'success');
        addConsoleMessage('Hard resetting via RTS pin...', 'success');
        addConsoleMessage('='.repeat(50));
    }, 3500);
}

// Editor Functions
function formatCode() {
    const code = editor.getValue();
    // Simple formatting (in a real IDE, you'd use a proper formatter)
    const formatted = code
        .split('\n')
        .map(line => line.trim())
        .join('\n');
    
    editor.setValue(formatted);
    addConsoleMessage('Code formatted', 'success');
}

// Console Functions
function addConsoleMessage(message, type = '') {
    const consoleOutput = document.getElementById('consoleOutput');
    const line = document.createElement('div');
    line.className = `console-line ${type}`;
    line.textContent = message;
    consoleOutput.appendChild(line);
    consoleOutput.scrollTop = consoleOutput.scrollHeight;
}

function clearConsole() {
    const consoleOutput = document.getElementById('consoleOutput');
    consoleOutput.innerHTML = '<div class="console-line">Console cleared</div>';
}

// Chat AI Functions
function sendChatMessage() {
    const input = document.getElementById('chatInput');
    const message = input.value.trim();
    
    if (!message) return;
    
    // Add user message
    addChatMessage(message, 'user');
    input.value = '';
    
    // Simulate AI response
    setTimeout(() => {
        const response = generateAIResponse(message);
        addChatMessage(response, 'assistant');
    }, 1000);
}

function addChatMessage(message, sender) {
    const chatMessages = document.getElementById('chatMessages');
    const messageDiv = document.createElement('div');
    messageDiv.className = `chat-message ${sender}`;
    
    const contentDiv = document.createElement('div');
    contentDiv.className = 'message-content';
    
    if (sender === 'assistant') {
        contentDiv.innerHTML = `<strong>AI Assistant:</strong> ${message}`;
    } else {
        contentDiv.textContent = message;
    }
    
    messageDiv.appendChild(contentDiv);
    chatMessages.appendChild(messageDiv);
    chatMessages.scrollTop = chatMessages.scrollHeight;
}

function generateAIResponse(userMessage) {
    const lowerMessage = userMessage.toLowerCase();
    
    // ESP32 specific responses
    if (lowerMessage.includes('gpio') || lowerMessage.includes('pin')) {
        return 'ESP32 has multiple GPIO pins. For digital output, use pinMode(pin, OUTPUT) and digitalWrite(pin, HIGH/LOW). For analog input, use analogRead(pin). Remember that some pins are input-only and some have special functions.';
    }
    
    if (lowerMessage.includes('wifi') || lowerMessage.includes('network')) {
        return 'To use WiFi on ESP32, include WiFi.h library. Use WiFi.begin(ssid, password) to connect. Check connection with WiFi.status() == WL_CONNECTED. Example: WiFi.begin("SSID", "PASSWORD");';
    }
    
    if (lowerMessage.includes('bluetooth') || lowerMessage.includes('ble')) {
        return 'ESP32 supports both Classic Bluetooth and BLE. For BLE, use the BLE library. For Classic Bluetooth, use BluetoothSerial.h. BLE is more power-efficient for IoT applications.';
    }
    
    if (lowerMessage.includes('serial') || lowerMessage.includes('debug')) {
        return 'Use Serial.begin(115200) in setup() to initialize serial communication. Use Serial.println() to print debug messages. You can monitor serial output in the console below.';
    }
    
    if (lowerMessage.includes('error') || lowerMessage.includes('not working')) {
        return 'Common ESP32 issues: 1) Check your board and port selection. 2) Ensure correct pin numbers. 3) Verify power supply (3.3V for GPIO). 4) Check if pins have special functions. 5) Review serial output for error messages.';
    }
    
    if (lowerMessage.includes('sensor') || lowerMessage.includes('read')) {
        return 'ESP32 has built-in ADC for reading analog sensors. Use analogRead(pin) for values 0-4095. For digital sensors, use appropriate libraries (DHT, BMP, etc.). Make sure to install required libraries first.';
    }
    
    if (lowerMessage.includes('delay') || lowerMessage.includes('timing')) {
        return 'Use delay(milliseconds) for simple delays. For non-blocking timing, use millis() to track elapsed time. Example: unsigned long previousMillis = 0; if (millis() - previousMillis >= interval) { ... }';
    }
    
    // Code analysis
    if (lowerMessage.includes('analyze') || lowerMessage.includes('review')) {
        const code = editor.getValue();
        if (code.includes('delay(') && code.length > 100) {
            return 'I notice you\'re using delay() functions. Consider using millis() for non-blocking delays in more complex applications. This allows your ESP32 to handle multiple tasks simultaneously.';
        }
        return 'Your code structure looks good! Make sure you have proper error handling and consider adding comments for complex logic.';
    }
    
    // Default response
    return 'I can help you with ESP32 development! Ask me about GPIO pins, WiFi, Bluetooth, sensors, debugging, or any ESP32-specific questions. I can also analyze your code and provide suggestions.';
}

function toggleChatPanel() {
    const chatPanel = document.getElementById('chatPanel');
    chatPanel.classList.toggle('minimized');
}

// Load saved files on startup
window.addEventListener('load', function() {
    try {
        const savedFiles = localStorage.getItem('esp32_ide_files');
        if (savedFiles) {
            const parsed = JSON.parse(savedFiles);
            files = { ...files, ...parsed };
            
            // Add files to list
            Object.keys(parsed).forEach(fileName => {
                if (fileName !== 'sketch.ino') {
                    addFileToList(fileName);
                }
            });
            
            addConsoleMessage('Loaded saved files from previous session', 'success');
        }
    } catch (e) {
        console.error('Error loading saved files:', e);
    }
});
