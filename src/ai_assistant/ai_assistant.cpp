#include "ai_assistant/ai_assistant.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <regex>

namespace esp32_ide {

AIAssistant::AIAssistant() : learning_mode_enabled_(false) {
    AddMessage(Message::Sender::ASSISTANT, 
               "Hello! I'm here to help you with ESP32 development. "
               "Ask me anything about your code, ESP32 APIs, or debugging issues!");
}

AIAssistant::~AIAssistant() = default;

std::string AIAssistant::Query(const std::string& user_message) {
    AddMessage(Message::Sender::USER, user_message);
    std::string response = GenerateResponse(user_message);
    AddMessage(Message::Sender::ASSISTANT, response);
    return response;
}

std::vector<AIAssistant::Message> AIAssistant::GetHistory() const {
    return history_;
}

void AIAssistant::ClearHistory() {
    history_.clear();
}

std::string AIAssistant::AnalyzeCode(const std::string& code) {
    std::string analysis = "Code Analysis:\n";
    
    // Check for delay() usage
    if (code.find("delay(") != std::string::npos) {
        analysis += "- Consider using millis() for non-blocking delays in complex applications\n";
    }
    
    // Check for Serial initialization
    if (code.find("Serial.") != std::string::npos && code.find("Serial.begin") == std::string::npos) {
        analysis += "- Remember to initialize Serial with Serial.begin() in setup()\n";
    }
    
    // Check for pinMode
    if (code.find("digitalWrite") != std::string::npos || code.find("digitalRead") != std::string::npos) {
        if (code.find("pinMode") == std::string::npos) {
            analysis += "- Remember to set pin modes with pinMode() in setup()\n";
        }
    }
    
    // Check for WiFi usage
    if (code.find("WiFi.") != std::string::npos) {
        if (code.find("#include") == std::string::npos || code.find("WiFi.h") == std::string::npos) {
            analysis += "- Include WiFi.h library for WiFi functionality\n";
        }
    }
    
    if (analysis == "Code Analysis:\n") {
        analysis += "Your code structure looks good!";
    }
    
    return analysis;
}

std::string AIAssistant::SuggestFix(const std::string& error_message) {
    std::string lower_error = error_message;
    std::transform(lower_error.begin(), lower_error.end(), lower_error.begin(), ::tolower);
    
    if (lower_error.find("undeclared") != std::string::npos) {
        return "Check if you've declared the variable or included the necessary library.";
    }
    
    if (lower_error.find("expected") != std::string::npos) {
        return "Check for missing semicolons, brackets, or parentheses.";
    }
    
    if (lower_error.find("does not name a type") != std::string::npos) {
        return "Make sure you've included the correct header files and check your type declarations.";
    }
    
    return "Review the error message carefully and check your syntax near the indicated line.";
}

std::string AIAssistant::GenerateCode(const std::string& request) {
    std::string lower_request = request;
    std::transform(lower_request.begin(), lower_request.end(), lower_request.begin(), ::tolower);
    
    // Check for code generation requests
    if (ContainsKeywords(lower_request, {"generate", "create", "write", "make", "code for"})) {
        if (ContainsKeywords(lower_request, {"gpio", "pin", "led", "button", "blink"})) {
            return GenerateGPIOCode(lower_request);
        }
        if (ContainsKeywords(lower_request, {"wifi", "network", "connect"})) {
            return GenerateWiFiCode();
        }
        if (ContainsKeywords(lower_request, {"bluetooth", "ble", "bt"})) {
            return GenerateBluetoothCode();
        }
        if (ContainsKeywords(lower_request, {"serial", "print", "debug"})) {
            return GenerateSerialCode();
        }
        if (ContainsKeywords(lower_request, {"sensor", "dht", "temperature", "humidity"})) {
            return GenerateSensorCode("DHT");
        }
    }
    
    return "";  // No code generated
}

std::string AIAssistant::GenerateGPIOCode(const std::string& request) const {
    // Check if it's about LED/blinking
    if (request.find("led") != std::string::npos || request.find("blink") != std::string::npos) {
        return "// LED Blink Example\n"
               "#define LED_PIN 2  // Built-in LED on most ESP32 boards\n\n"
               "void setup() {\n"
               "  pinMode(LED_PIN, OUTPUT);\n"
               "}\n\n"
               "void loop() {\n"
               "  digitalWrite(LED_PIN, HIGH);  // Turn LED on\n"
               "  delay(1000);                  // Wait 1 second\n"
               "  digitalWrite(LED_PIN, LOW);   // Turn LED off\n"
               "  delay(1000);                  // Wait 1 second\n"
               "}\n";
    }
    
    // Check if it's about button
    if (request.find("button") != std::string::npos) {
        return "// Button Input Example\n"
               "#define BUTTON_PIN 4\n"
               "#define LED_PIN 2\n\n"
               "void setup() {\n"
               "  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pull-up\n"
               "  pinMode(LED_PIN, OUTPUT);\n"
               "  Serial.begin(115200);\n"
               "}\n\n"
               "void loop() {\n"
               "  int buttonState = digitalRead(BUTTON_PIN);\n"
               "  \n"
               "  if (buttonState == LOW) {  // Button pressed (active LOW)\n"
               "    digitalWrite(LED_PIN, HIGH);\n"
               "    Serial.println(\"Button pressed!\");\n"
               "  } else {\n"
               "    digitalWrite(LED_PIN, LOW);\n"
               "  }\n"
               "  \n"
               "  delay(50);  // Debounce delay\n"
               "}\n";
    }
    
    // Generic GPIO example
    return "// GPIO Output Example\n"
           "#define OUTPUT_PIN 2\n\n"
           "void setup() {\n"
           "  pinMode(OUTPUT_PIN, OUTPUT);\n"
           "}\n\n"
           "void loop() {\n"
           "  digitalWrite(OUTPUT_PIN, HIGH);\n"
           "  delay(1000);\n"
           "  digitalWrite(OUTPUT_PIN, LOW);\n"
           "  delay(1000);\n"
           "}\n";
}

std::string AIAssistant::GenerateWiFiCode() const {
    return "// WiFi Connection Example\n"
           "#include <WiFi.h>\n\n"
           "const char* ssid = \"YOUR_SSID\";\n"
           "const char* password = \"YOUR_PASSWORD\";\n\n"
           "void setup() {\n"
           "  Serial.begin(115200);\n"
           "  \n"
           "  // Connect to WiFi\n"
           "  Serial.print(\"Connecting to \");\n"
           "  Serial.println(ssid);\n"
           "  \n"
           "  WiFi.begin(ssid, password);\n"
           "  \n"
           "  while (WiFi.status() != WL_CONNECTED) {\n"
           "    delay(500);\n"
           "    Serial.print(\".\");\n"
           "  }\n"
           "  \n"
           "  Serial.println(\"\");\n"
           "  Serial.println(\"WiFi connected!\");\n"
           "  Serial.print(\"IP address: \");\n"
           "  Serial.println(WiFi.localIP());\n"
           "}\n\n"
           "void loop() {\n"
           "  // Your code here\n"
           "}\n";
}

std::string AIAssistant::GenerateBluetoothCode() const {
    return "// Bluetooth Serial Example\n"
           "#include <BluetoothSerial.h>\n\n"
           "BluetoothSerial SerialBT;\n\n"
           "void setup() {\n"
           "  Serial.begin(115200);\n"
           "  SerialBT.begin(\"ESP32_BT\");  // Bluetooth device name\n"
           "  Serial.println(\"Bluetooth Started! Ready to pair...\");\n"
           "}\n\n"
           "void loop() {\n"
           "  // Read from Bluetooth\n"
           "  if (SerialBT.available()) {\n"
           "    char c = SerialBT.read();\n"
           "    Serial.write(c);\n"
           "  }\n"
           "  \n"
           "  // Read from Serial and send to Bluetooth\n"
           "  if (Serial.available()) {\n"
           "    char c = Serial.read();\n"
           "    SerialBT.write(c);\n"
           "  }\n"
           "}\n";
}

std::string AIAssistant::GenerateSerialCode() const {
    return "// Serial Communication Example\n"
           "void setup() {\n"
           "  Serial.begin(115200);\n"
           "  Serial.println(\"ESP32 Serial Monitor Started\");\n"
           "}\n\n"
           "void loop() {\n"
           "  // Print messages\n"
           "  Serial.print(\"Time: \");\n"
           "  Serial.print(millis());\n"
           "  Serial.println(\" ms\");\n"
           "  \n"
           "  // Read input if available\n"
           "  if (Serial.available() > 0) {\n"
           "    String input = Serial.readStringUntil('\\n');\n"
           "    Serial.print(\"You sent: \");\n"
           "    Serial.println(input);\n"
           "  }\n"
           "  \n"
           "  delay(1000);\n"
           "}\n";
}

std::string AIAssistant::GenerateSensorCode(const std::string& sensor_type) const {
    if (sensor_type == "DHT") {
        return "// DHT Temperature & Humidity Sensor Example\n"
               "#include <DHT.h>\n\n"
               "#define DHTPIN 4       // Pin connected to DHT sensor\n"
               "#define DHTTYPE DHT22  // DHT 22 (AM2302)\n\n"
               "DHT dht(DHTPIN, DHTTYPE);\n\n"
               "void setup() {\n"
               "  Serial.begin(115200);\n"
               "  dht.begin();\n"
               "  Serial.println(\"DHT Sensor initialized\");\n"
               "}\n\n"
               "void loop() {\n"
               "  // Wait a few seconds between measurements\n"
               "  delay(2000);\n"
               "  \n"
               "  // Read temperature and humidity\n"
               "  float humidity = dht.readHumidity();\n"
               "  float temperature = dht.readTemperature();\n"
               "  \n"
               "  // Check if readings are valid\n"
               "  if (isnan(humidity) || isnan(temperature)) {\n"
               "    Serial.println(\"Failed to read from DHT sensor!\");\n"
               "    return;\n"
               "  }\n"
               "  \n"
               "  Serial.print(\"Humidity: \");\n"
               "  Serial.print(humidity);\n"
               "  Serial.print(\"%  Temperature: \");\n"
               "  Serial.print(temperature);\n"
               "  Serial.println(\"°C\");\n"
               "}\n";
    }
    
    return "// Generic Sensor Example\n"
           "#define SENSOR_PIN 34  // Analog input pin\n\n"
           "void setup() {\n"
           "  Serial.begin(115200);\n"
           "}\n\n"
           "void loop() {\n"
           "  int sensorValue = analogRead(SENSOR_PIN);\n"
           "  Serial.print(\"Sensor Value: \");\n"
           "  Serial.println(sensorValue);\n"
           "  delay(500);\n"
           "}\n";
}

std::string AIAssistant::GenerateResponse(const std::string& query) const {
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
    
    if (ContainsKeywords(lower_query, {"gpio", "pin", "digital", "analog"})) {
        return HandleGPIOQuery();
    }
    
    if (ContainsKeywords(lower_query, {"wifi", "network", "connect", "internet"})) {
        return HandleWiFiQuery();
    }
    
    if (ContainsKeywords(lower_query, {"bluetooth", "ble", "bt"})) {
        return HandleBluetoothQuery();
    }
    
    if (ContainsKeywords(lower_query, {"serial", "print", "debug", "monitor"})) {
        return HandleSerialQuery();
    }
    
    if (ContainsKeywords(lower_query, {"error", "not working", "problem", "issue", "fix"})) {
        return HandleDebugQuery();
    }
    
    if (ContainsKeywords(lower_query, {"sensor", "read", "measure", "adc"})) {
        return HandleSensorQuery();
    }
    
    if (ContainsKeywords(lower_query, {"delay", "timing", "millis", "time"})) {
        return HandleTimingQuery();
    }
    
    return GetDefaultResponse();
}

std::string AIAssistant::HandleGPIOQuery() const {
    return "ESP32 GPIO Guide:\n"
           "- Use pinMode(pin, OUTPUT) for output pins\n"
           "- Use pinMode(pin, INPUT) or INPUT_PULLUP for input pins\n"
           "- digitalWrite(pin, HIGH/LOW) to control output\n"
           "- digitalRead(pin) to read digital input (0 or 1)\n"
           "- analogRead(pin) to read analog input (0-4095)\n"
           "- Note: Some pins are input-only (36-39)\n"
           "- GPIO pins use 3.3V logic, not 5V!";
}

std::string AIAssistant::HandleWiFiQuery() const {
    return "ESP32 WiFi Connection:\n"
           "1. Include the WiFi library: #include <WiFi.h>\n"
           "2. In setup():\n"
           "   WiFi.begin(\"SSID\", \"PASSWORD\");\n"
           "   while (WiFi.status() != WL_CONNECTED) {\n"
           "     delay(500);\n"
           "     Serial.print(\".\");\n"
           "   }\n"
           "3. Get IP: WiFi.localIP()\n"
           "4. Disconnect: WiFi.disconnect()";
}

std::string AIAssistant::HandleBluetoothQuery() const {
    return "ESP32 Bluetooth:\n"
           "Classic Bluetooth:\n"
           "- #include <BluetoothSerial.h>\n"
           "- BluetoothSerial SerialBT;\n"
           "- SerialBT.begin(\"ESP32_BT\");\n\n"
           "BLE (Bluetooth Low Energy):\n"
           "- More power-efficient for IoT\n"
           "- #include <BLEDevice.h>\n"
           "- Use BLE library for advertising and services\n"
           "- Better for battery-powered applications";
}

std::string AIAssistant::HandleSerialQuery() const {
    return "ESP32 Serial Communication:\n"
           "- Initialize: Serial.begin(115200);\n"
           "- Print: Serial.print(\"text\") or Serial.println(\"text\")\n"
           "- Read: Serial.read() or Serial.readString()\n"
           "- Check data: if (Serial.available() > 0) {...}\n"
           "- Common baud rates: 9600, 115200\n"
           "- Use for debugging and communication";
}

std::string AIAssistant::HandleDebugQuery() const {
    return "ESP32 Debugging Tips:\n"
           "1. Check your board and port selection\n"
           "2. Verify pin numbers (GPIO number, not board number)\n"
           "3. Ensure 3.3V logic levels (not 5V)\n"
           "4. Check if pins have special functions\n"
           "5. Use Serial.println() for debug output\n"
           "6. Check power supply (USB may not provide enough current)\n"
           "7. Review compile errors carefully\n"
           "8. Check for missing semicolons and brackets";
}

std::string AIAssistant::HandleSensorQuery() const {
    return "ESP32 Sensor Reading:\n"
           "Analog Sensors:\n"
           "- Use analogRead(pin) for ADC (0-4095 range)\n"
           "- ADC pins: 32-39, 25-27, 12-15, 2, 4, 0\n"
           "- Map values: map(value, 0, 4095, min, max)\n\n"
           "Digital Sensors:\n"
           "- Use appropriate libraries (DHT, BMP, etc.)\n"
           "- Install libraries via Library Manager\n"
           "- Follow sensor-specific wiring diagrams";
}

std::string AIAssistant::HandleTimingQuery() const {
    return "ESP32 Timing Functions:\n"
           "delay(ms) - Blocking delay:\n"
           "- Simple but blocks all code execution\n"
           "- Good for simple programs\n\n"
           "millis() - Non-blocking timing:\n"
           "- unsigned long currentMillis = millis();\n"
           "- if (currentMillis - previousMillis >= interval) {...}\n"
           "- Allows multiple tasks simultaneously\n"
           "- Better for complex applications\n\n"
           "micros() - Microsecond precision timing";
}

std::string AIAssistant::GetDefaultResponse() const {
    return "I can help you with ESP32 development!\n"
           "Topics I can assist with:\n"
           "- GPIO pins and digital/analog I/O\n"
           "- WiFi and network connectivity\n"
           "- Bluetooth (Classic and BLE)\n"
           "- Serial communication and debugging\n"
           "- Sensor interfacing\n"
           "- Timing and delays\n"
           "- Code analysis and troubleshooting\n\n"
           "Ask me a specific question about your ESP32 project!";
}

void AIAssistant::AddMessage(Message::Sender sender, const std::string& content) {
    Message msg;
    msg.sender = sender;
    msg.content = content;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    history_.push_back(msg);
}

bool AIAssistant::ContainsKeywords(const std::string& text, 
                                   const std::vector<std::string>& keywords) const {
    for (const auto& keyword : keywords) {
        if (text.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// AI-powered code refactoring
std::string AIAssistant::RefactorCode(const std::string& code, const std::string& refactor_type) {
    std::string lower_type = refactor_type;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
    
    if (lower_type == "optimize" || lower_type == "performance") {
        return OptimizeCode(code);
    } else if (lower_type == "readability" || lower_type == "clean") {
        return ImproveReadability(code);
    }
    
    // Default: comprehensive refactoring
    std::string refactored = code;
    
    // Replace blocking delays with millis() pattern
    if (refactored.find("delay(") != std::string::npos && 
        refactored.find("loop()") != std::string::npos) {
        std::string suggestion = "// Refactoring suggestion: Replace delay() with millis() for non-blocking operation\n"
                                "// Example:\n"
                                "// unsigned long previousMillis = 0;\n"
                                "// const long interval = 1000;\n"
                                "// In loop(): if (millis() - previousMillis >= interval) { ... }\n\n";
        refactored = suggestion + refactored;
    }
    
    return refactored;
}

std::string AIAssistant::OptimizeCode(const std::string& code) {
    std::string optimized = code;
    std::string suggestions = "// Optimization suggestions:\n";
    bool has_suggestions = false;
    
    // Check for repeated calculations
    if (code.find("analogRead") != std::string::npos) {
        suggestions += "// - Cache analogRead() results if reading same pin multiple times\n";
        has_suggestions = true;
    }
    
    // Check for String usage (memory intensive)
    if (code.find("String ") != std::string::npos) {
        suggestions += "// - Consider using char arrays instead of String for better memory management\n";
        has_suggestions = true;
    }
    
    // Check for Serial.print in loops
    if (code.find("Serial.print") != std::string::npos && code.find("for(") != std::string::npos) {
        suggestions += "// - Minimize Serial.print() calls in loops for better performance\n";
        has_suggestions = true;
    }
    
    if (has_suggestions) {
        optimized = suggestions + "\n" + optimized;
    }
    
    return optimized;
}

std::string AIAssistant::ImproveReadability(const std::string& code) {
    std::string improved = code;
    std::string suggestions = "// Readability improvements:\n";
    bool has_suggestions = false;
    
    // Check for magic numbers
    if (code.find("pinMode(") != std::string::npos) {
        size_t pos = code.find("pinMode(");
        size_t next_comma = code.find(",", pos);
        if (next_comma != std::string::npos) {
            std::string pin_arg = code.substr(pos + 8, next_comma - pos - 8);
            // Trim whitespace
            pin_arg.erase(0, pin_arg.find_first_not_of(" \t\n\r"));
            pin_arg.erase(pin_arg.find_last_not_of(" \t\n\r") + 1);
            // Check if it's a numeric literal
            if (!pin_arg.empty() && std::all_of(pin_arg.begin(), pin_arg.end(), ::isdigit)) {
                suggestions += "// - Define pin numbers as constants (e.g., const int LED_PIN = 13;)\n";
                has_suggestions = true;
            }
        }
    }
    
    // Check for lack of comments
    if (code.size() > 100 && code.find("//") == std::string::npos && code.find("/*") == std::string::npos) {
        suggestions += "// - Add comments to explain complex logic\n";
        has_suggestions = true;
    }
    
    if (has_suggestions) {
        improved = suggestions + "\n" + improved;
    }
    
    return improved;
}

// Automatic bug detection
std::vector<AIAssistant::BugReport> AIAssistant::DetectBugs(const std::string& code) {
    std::vector<BugReport> bugs;
    int line_num = 1;
    size_t pos = 0;
    
    // Track line numbers
    auto get_line_number = [&](size_t position) {
        int line = 1;
        for (size_t i = 0; i < position && i < code.size(); i++) {
            if (code[i] == '\n') line++;
        }
        return line;
    };
    
    // Check for Serial usage without initialization
    if (code.find("Serial.") != std::string::npos) {
        if (code.find("Serial.begin") == std::string::npos) {
            BugReport bug;
            bug.severity = "critical";
            bug.line_number = get_line_number(code.find("Serial."));
            bug.description = "Serial used without initialization";
            bug.suggested_fix = "Add Serial.begin(115200); in setup() function";
            bugs.push_back(bug);
        }
    }
    
    // Check for pinMode missing before GPIO operations
    if ((code.find("digitalWrite") != std::string::npos || code.find("digitalRead") != std::string::npos)) {
        if (code.find("pinMode") == std::string::npos) {
            BugReport bug;
            bug.severity = "critical";
            bug.line_number = get_line_number(code.find("digitalWrite"));
            bug.description = "GPIO operations without pinMode configuration";
            bug.suggested_fix = "Add pinMode(pin, MODE); in setup() before using the pin";
            bugs.push_back(bug);
        }
    }
    
    // Check for missing WiFi include
    if (code.find("WiFi.") != std::string::npos) {
        if (code.find("#include <WiFi.h>") == std::string::npos && 
            code.find("#include \"WiFi.h\"") == std::string::npos) {
            BugReport bug;
            bug.severity = "critical";
            bug.line_number = 1;
            bug.description = "WiFi used without including WiFi.h";
            bug.suggested_fix = "Add #include <WiFi.h> at the top of the file";
            bugs.push_back(bug);
        }
    }
    
    // Check for delay in time-critical code
    if (code.find("delay(") != std::string::npos && 
        (code.find("interrupt") != std::string::npos || code.find("ISR") != std::string::npos)) {
        BugReport bug;
        bug.severity = "warning";
        bug.line_number = get_line_number(code.find("delay("));
        bug.description = "Delay used in interrupt-related code";
        bug.suggested_fix = "Use millis() or hardware timers instead of delay()";
        bugs.push_back(bug);
    }
    
    // Check for floating point in loop counters
    if (code.find("for") != std::string::npos && code.find("float") != std::string::npos) {
        BugReport bug;
        bug.severity = "suggestion";
        bug.line_number = get_line_number(code.find("for"));
        bug.description = "Possible floating-point loop counter";
        bug.suggested_fix = "Use integer loop counters for better performance";
        bugs.push_back(bug);
    }
    
    return bugs;
}

std::string AIAssistant::AutoFixBugs(const std::string& code) {
    std::string fixed = code;
    std::vector<BugReport> bugs = DetectBugs(code);
    
    // Apply automatic fixes for common bugs
    for (const auto& bug : bugs) {
        if (bug.severity == "critical") {
            // Add Serial.begin if missing
            if (bug.description.find("Serial used without initialization") != std::string::npos) {
                size_t setup_pos = fixed.find("void setup()");
                if (setup_pos != std::string::npos) {
                    size_t brace_pos = fixed.find("{", setup_pos);
                    if (brace_pos != std::string::npos) {
                        fixed.insert(brace_pos + 1, "\n  Serial.begin(115200);");
                    }
                }
            }
            
            // Add WiFi include if missing
            if (bug.description.find("WiFi used without including") != std::string::npos) {
                fixed = "#include <WiFi.h>\n" + fixed;
            }
        }
    }
    
    return fixed;
}

// Code completion suggestions
std::vector<AIAssistant::CompletionSuggestion> AIAssistant::GetCompletionSuggestions(
    const std::string& code, 
    int cursor_position,
    const std::string& current_line) {
    
    std::vector<CompletionSuggestion> suggestions;
    
    std::string lower_line = current_line;
    std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
    
    // GPIO suggestions
    if (lower_line.find("pin") != std::string::npos || lower_line.find("gpio") != std::string::npos) {
        suggestions.push_back({"pinMode(pin, OUTPUT);", "Set pin as output", 90});
        suggestions.push_back({"pinMode(pin, INPUT);", "Set pin as input", 90});
        suggestions.push_back({"pinMode(pin, INPUT_PULLUP);", "Set pin as input with pullup", 85});
        suggestions.push_back({"digitalWrite(pin, HIGH);", "Set pin HIGH", 88});
        suggestions.push_back({"digitalWrite(pin, LOW);", "Set pin LOW", 88});
        suggestions.push_back({"digitalRead(pin)", "Read digital pin", 87});
        suggestions.push_back({"analogRead(pin)", "Read analog pin", 86});
    }
    
    // Serial suggestions
    if (lower_line.find("serial") != std::string::npos) {
        suggestions.push_back({"Serial.begin(115200);", "Initialize serial", 95});
        suggestions.push_back({"Serial.println();", "Print line to serial", 90});
        suggestions.push_back({"Serial.print();", "Print to serial", 88});
        suggestions.push_back({"Serial.available()", "Check if data available", 85});
        suggestions.push_back({"Serial.read()", "Read one byte", 84});
    }
    
    // WiFi suggestions
    if (lower_line.find("wifi") != std::string::npos) {
        suggestions.push_back({"WiFi.begin(ssid, password);", "Connect to WiFi", 95});
        suggestions.push_back({"WiFi.status()", "Get WiFi status", 90});
        suggestions.push_back({"WiFi.localIP()", "Get IP address", 88});
        suggestions.push_back({"WiFi.disconnect();", "Disconnect WiFi", 85});
    }
    
    // Delay/timing suggestions
    if (lower_line.find("delay") != std::string::npos || lower_line.find("time") != std::string::npos) {
        suggestions.push_back({"delay(1000);", "Delay 1 second (blocking)", 90});
        suggestions.push_back({"millis()", "Get milliseconds since start", 92});
        suggestions.push_back({"unsigned long currentMillis = millis();", "Non-blocking timing", 95});
    }
    
    // Setup/loop template
    if (lower_line.find("setup") != std::string::npos || code.empty()) {
        suggestions.push_back({"void setup() {\n  // Initialize\n}", "Setup function", 100});
        suggestions.push_back({"void loop() {\n  // Main code\n}", "Loop function", 100});
    }
    
    // Sort by priority
    std::sort(suggestions.begin(), suggestions.end(), 
              [](const CompletionSuggestion& a, const CompletionSuggestion& b) {
                  return a.priority > b.priority;
              });
    
    return suggestions;
}

// Enhanced code generation
std::string AIAssistant::GenerateCompleteSketch(const std::string& description) {
    std::string lower_desc = description;
    std::transform(lower_desc.begin(), lower_desc.end(), lower_desc.begin(), ::tolower);
    
    std::string code;
    
    // Detect what features are needed
    bool needs_wifi = lower_desc.find("wifi") != std::string::npos || 
                      lower_desc.find("web") != std::string::npos ||
                      lower_desc.find("internet") != std::string::npos;
    bool needs_sensor = lower_desc.find("sensor") != std::string::npos ||
                       lower_desc.find("temperature") != std::string::npos ||
                       lower_desc.find("humidity") != std::string::npos;
    bool needs_led = lower_desc.find("led") != std::string::npos ||
                    lower_desc.find("blink") != std::string::npos;
    
    // Add includes
    if (needs_wifi) {
        code += "#include <WiFi.h>\n";
    }
    code += "\n";
    
    // Add constants
    if (needs_wifi) {
        code += "const char* ssid = \"YOUR_SSID\";\n";
        code += "const char* password = \"YOUR_PASSWORD\";\n\n";
    }
    
    if (needs_led) {
        code += "const int LED_PIN = 2;  // Built-in LED\n\n";
    }
    
    // Setup function
    code += "void setup() {\n";
    code += "  Serial.begin(115200);\n";
    code += "  delay(100);\n";
    code += "  Serial.println(\"ESP32 Starting...\");\n\n";
    
    if (needs_wifi) {
        code += "  // Connect to WiFi\n";
        code += "  WiFi.begin(ssid, password);\n";
        code += "  Serial.print(\"Connecting to WiFi\");\n";
        code += "  while (WiFi.status() != WL_CONNECTED) {\n";
        code += "    delay(500);\n";
        code += "    Serial.print(\".\");\n";
        code += "  }\n";
        code += "  Serial.println(\"\\nConnected!\");\n";
        code += "  Serial.print(\"IP Address: \");\n";
        code += "  Serial.println(WiFi.localIP());\n\n";
    }
    
    if (needs_led) {
        code += "  pinMode(LED_PIN, OUTPUT);\n\n";
    }
    
    code += "}\n\n";
    
    // Loop function
    code += "void loop() {\n";
    
    if (needs_led) {
        code += "  digitalWrite(LED_PIN, HIGH);\n";
        code += "  delay(1000);\n";
        code += "  digitalWrite(LED_PIN, LOW);\n";
        code += "  delay(1000);\n";
    } else {
        code += "  // Main code here\n";
        code += "  delay(1000);\n";
    }
    
    code += "}\n";
    
    return code;
}

std::string AIAssistant::GenerateWebServerCode(const std::string& endpoint_description) {
    return R"(#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WebServer server(80);

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32 Web Server</h1>";
  html += "<p>)" + endpoint_description + R"(</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
)";
}

std::string AIAssistant::GenerateMQTTCode(const std::string& topic) {
    return R"(#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "mqtt.example.com";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(")" + topic + R"(");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Publish example
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    String msg = "Hello from ESP32";
    client.publish(")" + topic + R"(", msg.c_str());
  }
}
)";
}

std::string AIAssistant::GenerateOTAUpdateCode() {
    return R"(#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // OTA Setup
  ArduinoOTA.setHostname("ESP32-OTA");
  ArduinoOTA.setPassword("admin");
  
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Update Starting...");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Complete!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void loop() {
  ArduinoOTA.handle();
  // Your code here
}
)";
}

std::string AIAssistant::GenerateDeepSleepCode(int sleep_seconds) {
    std::string code = R"(#include <esp_sleep.h>

#define uS_TO_S_FACTOR 1000000

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("ESP32 Waking up");
  
  // Your code here
  Serial.println("Going to sleep for )" + std::to_string(sleep_seconds) + R"( seconds");
  
  // Configure deep sleep
  esp_sleep_enable_timer_wakeup()" + std::to_string(sleep_seconds) + R"( * uS_TO_S_FACTOR);
  
  // Enter deep sleep
  Serial.println("Entering deep sleep...");
  esp_deep_sleep_start();
}

void loop() {
  // This will never run due to deep sleep
}
)";
    return code;
}

// ============================================================================
// Version 1.3.0 Features: Natural Language Commands
// ============================================================================

AIAssistant::CommandInterpretation AIAssistant::InterpretNaturalLanguage(const std::string& command) {
    CommandInterpretation result;
    result.raw_command = command;
    result.confidence = 0.0f;
    
    std::string lower_cmd = command;
    std::transform(lower_cmd.begin(), lower_cmd.end(), lower_cmd.begin(), ::tolower);
    
    // Detect action verbs
    if (ContainsKeywords(lower_cmd, {"create", "make", "generate", "write"})) {
        result.action = "generate_code";
        result.confidence = 0.8f;
        
        // Detect target
        if (ContainsKeywords(lower_cmd, {"led", "blink"})) {
            result.target = "led_blink";
            result.parameters["type"] = "gpio";
            result.confidence = 0.9f;
        } else if (ContainsKeywords(lower_cmd, {"wifi", "network"})) {
            result.target = "wifi_connection";
            result.parameters["type"] = "wifi";
            result.confidence = 0.9f;
        } else if (ContainsKeywords(lower_cmd, {"bluetooth", "ble"})) {
            result.target = "bluetooth";
            result.parameters["type"] = "bluetooth";
            result.confidence = 0.9f;
        } else if (ContainsKeywords(lower_cmd, {"web", "server"})) {
            result.target = "web_server";
            result.parameters["type"] = "web";
            result.confidence = 0.85f;
        }
    } else if (ContainsKeywords(lower_cmd, {"optimize", "improve", "refactor"})) {
        result.action = "optimize_code";
        result.confidence = 0.85f;
        
        if (ContainsKeywords(lower_cmd, {"performance", "speed", "faster"})) {
            result.parameters["focus"] = "performance";
        } else if (ContainsKeywords(lower_cmd, {"readability", "clean", "readable"})) {
            result.parameters["focus"] = "readability";
        }
    } else if (ContainsKeywords(lower_cmd, {"analyze", "check", "scan", "review"})) {
        result.action = "analyze_code";
        result.confidence = 0.8f;
        
        if (ContainsKeywords(lower_cmd, {"security", "vulnerability", "secure"})) {
            result.parameters["analysis_type"] = "security";
            result.confidence = 0.9f;
        } else if (ContainsKeywords(lower_cmd, {"performance", "optimize"})) {
            result.parameters["analysis_type"] = "performance";
            result.confidence = 0.9f;
        } else if (ContainsKeywords(lower_cmd, {"smell", "quality"})) {
            result.parameters["analysis_type"] = "code_smell";
            result.confidence = 0.85f;
        }
    } else if (ContainsKeywords(lower_cmd, {"fix", "repair", "correct"})) {
        result.action = "fix_code";
        result.confidence = 0.75f;
        
        if (ContainsKeywords(lower_cmd, {"bug", "error", "issue"})) {
            result.parameters["fix_type"] = "bug";
            result.confidence = 0.85f;
        }
    }
    
    return result;
}

std::string AIAssistant::ExecuteNaturalLanguageCommand(const std::string& command) {
    auto interpretation = InterpretNaturalLanguage(command);
    
    if (interpretation.confidence < 0.5f) {
        return "I'm not sure what you want me to do. Could you rephrase that? "
               "Try commands like:\n"
               "- 'Create a LED blink program'\n"
               "- 'Generate WiFi connection code'\n"
               "- 'Analyze my code for security issues'\n"
               "- 'Optimize this code for performance'";
    }
    
    if (interpretation.action == "generate_code") {
        return GenerateCode(command);
    } else if (interpretation.action == "optimize_code") {
        if (interpretation.parameters.count("focus")) {
            std::string focus = interpretation.parameters.at("focus");
            if (focus == "performance") {
                return "I can optimize your code for performance. Please paste your code, "
                       "and I'll suggest improvements to reduce execution time and memory usage.";
            } else if (focus == "readability") {
                return "I can improve code readability. Please paste your code, "
                       "and I'll suggest ways to make it clearer and more maintainable.";
            }
        }
        return "I can help optimize your code. Please paste the code you'd like me to improve.";
    } else if (interpretation.action == "analyze_code") {
        return "I can analyze your code. Please paste it, and I'll check for:\n"
               "- Security vulnerabilities\n"
               "- Performance issues\n"
               "- Code smells and quality concerns";
    } else if (interpretation.action == "fix_code") {
        return "I can help fix issues in your code. Please paste the code and describe "
               "the problem you're experiencing.";
    }
    
    return "Command interpreted but no handler available yet. Interpreted as: " + 
           interpretation.action + " -> " + interpretation.target;
}

// ============================================================================
// Version 1.3.0 Features: Advanced Code Analysis
// ============================================================================

std::vector<AIAssistant::SecurityIssue> AIAssistant::ScanSecurityVulnerabilities(const std::string& code) {
    std::vector<SecurityIssue> issues;
    auto lines = ExtractCodeLines(code);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        
        // Check for hardcoded credentials
        if (IsHardcodedCredential(line)) {
            SecurityIssue issue;
            issue.type = "hardcoded_credentials";
            issue.severity = "high";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "Hardcoded credentials detected in code";
            issue.recommendation = "Move credentials to secure storage or configuration file. "
                                   "Consider using WiFiManager for WiFi credentials.";
            issues.push_back(issue);
        }
        
        // Check for buffer overflow risks
        if (HasBufferOverflowRisk(line)) {
            SecurityIssue issue;
            issue.type = "buffer_overflow";
            issue.severity = "critical";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "Potential buffer overflow detected";
            issue.recommendation = "Use safe string functions like strncpy() instead of strcpy(), "
                                   "or use std::string for automatic memory management.";
            issues.push_back(issue);
        }
        
        // Check for unsafe Serial input
        if (line.find("Serial.read") != std::string::npos && 
            line.find("while") != std::string::npos) {
            SecurityIssue issue;
            issue.type = "unbounded_input";
            issue.severity = "medium";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "Unbounded serial input may cause memory issues";
            issue.recommendation = "Limit input size using Serial.readBytesUntil() with a "
                                   "maximum length parameter.";
            issues.push_back(issue);
        }
        
        // Check for insecure HTTP connections
        if (line.find("http://") != std::string::npos && 
            (line.find("HTTPClient") != std::string::npos || line.find("WiFiClient") != std::string::npos)) {
            SecurityIssue issue;
            issue.type = "insecure_connection";
            issue.severity = "medium";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "Using insecure HTTP connection";
            issue.recommendation = "Use HTTPS (https://) for secure communication. "
                                   "Use WiFiClientSecure instead of WiFiClient.";
            issues.push_back(issue);
        }
    }
    
    return issues;
}

std::vector<AIAssistant::PerformanceIssue> AIAssistant::SuggestPerformanceOptimizations(const std::string& code) {
    std::vector<PerformanceIssue> issues;
    auto lines = ExtractCodeLines(code);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        
        // Check for delay() in loop
        if (line.find("delay(") != std::string::npos) {
            bool in_loop = false;
            for (size_t j = i; j > 0 && j > i - 20; --j) {
                std::string check_line = lines[j];
                // Remove leading/trailing whitespace for comparison
                check_line.erase(0, check_line.find_first_not_of(" \t\n\r"));
                if (check_line.find("void loop()") != std::string::npos || 
                    check_line.find("void loop(") != std::string::npos) {
                    in_loop = true;
                    break;
                }
            }
            
            if (in_loop) {
                PerformanceIssue issue;
                issue.type = "blocking_delay";
                issue.line_number = static_cast<int>(i + 1);
                issue.description = "Blocking delay() call in loop() function";
                issue.optimization = "Use millis() for non-blocking timing:\n"
                                     "unsigned long previousMillis = 0;\n"
                                     "const long interval = 1000;\n"
                                     "if (millis() - previousMillis >= interval) {\n"
                                     "  previousMillis = millis();\n"
                                     "  // Your code here\n"
                                     "}";
                issue.impact_score = 8;
                issues.push_back(issue);
            }
        }
        
        // Check for String concatenation in loops
        if (line.find("String") != std::string::npos && line.find("+=") != std::string::npos) {
            PerformanceIssue issue;
            issue.type = "string_concatenation";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "String concatenation can cause memory fragmentation";
            issue.optimization = "Pre-allocate String with reserve() or use char arrays for "
                                 "better performance and memory efficiency";
            issue.impact_score = 6;
            issues.push_back(issue);
        }
        
        // Check for repeated analogRead
        if (line.find("analogRead") != std::string::npos) {
            int analog_count = 0;
            for (const auto& l : lines) {
                if (l.find("analogRead") != std::string::npos) {
                    analog_count++;
                }
            }
            
            if (analog_count > 3) {
                PerformanceIssue issue;
                issue.type = "excessive_analog_reads";
                issue.line_number = static_cast<int>(i + 1);
                issue.description = "Multiple analogRead() calls can be slow";
                issue.optimization = "Cache analog readings or use a lower sampling rate. "
                                     "Consider using analogReadMilliVolts() for better accuracy.";
                issue.impact_score = 5;
                issues.push_back(issue);
                break;  // Only report once
            }
        }
        
        // Check for inefficient loops
        if (line.find("for") != std::string::npos && 
            (line.find("String") != std::string::npos || line.find(".length()") != std::string::npos)) {
            PerformanceIssue issue;
            issue.type = "inefficient_loop";
            issue.line_number = static_cast<int>(i + 1);
            issue.description = "Loop condition evaluated every iteration";
            issue.optimization = "Cache the length value before the loop:\n"
                                 "int len = myString.length();\n"
                                 "for (int i = 0; i < len; i++)";
            issue.impact_score = 4;
            issues.push_back(issue);
        }
    }
    
    return issues;
}

std::vector<AIAssistant::CodeSmell> AIAssistant::DetectCodeSmells(const std::string& code) {
    std::vector<CodeSmell> smells;
    auto lines = ExtractCodeLines(code);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        
        // Check for magic numbers
        if (std::regex_search(line, std::regex("\\b(\\d{3,})\\b")) && 
            line.find("//") == std::string::npos &&
            line.find("#define") == std::string::npos) {
            CodeSmell smell;
            smell.type = "magic_number";
            smell.line_number = static_cast<int>(i + 1);
            smell.description = "Magic number without explanation";
            smell.refactoring_suggestion = "Define constants with meaningful names:\n"
                                           "const int SENSOR_THRESHOLD = <value>;\n"
                                           "const int BAUD_RATE = <value>;";
            smells.push_back(smell);
        }
        
        // Check for long lines
        if (line.length() > 120) {
            CodeSmell smell;
            smell.type = "long_line";
            smell.line_number = static_cast<int>(i + 1);
            smell.description = "Line exceeds recommended length";
            smell.refactoring_suggestion = "Break long lines into multiple lines for better readability";
            smells.push_back(smell);
        }
        
        // Check for commented-out code
        if (line.find("//") != std::string::npos) {
            std::string after_comment = line.substr(line.find("//") + 2);
            if (after_comment.find("(") != std::string::npos || 
                after_comment.find(";") != std::string::npos) {
                CodeSmell smell;
                smell.type = "commented_code";
                smell.line_number = static_cast<int>(i + 1);
                smell.description = "Commented-out code detected";
                smell.refactoring_suggestion = "Remove commented code - use version control instead";
                smells.push_back(smell);
            }
        }
        
        // Check for duplicate code patterns
        if (line.find("digitalWrite") != std::string::npos || 
            line.find("analogWrite") != std::string::npos) {
            int similar_count = 0;
            for (const auto& l : lines) {
                if (l.find(line.substr(0, std::min(size_t(30), line.length()))) != std::string::npos) {
                    similar_count++;
                }
            }
            
            if (similar_count > 3) {
                CodeSmell smell;
                smell.type = "duplicate_code";
                smell.line_number = static_cast<int>(i + 1);
                smell.description = "Duplicate code pattern detected";
                smell.refactoring_suggestion = "Extract repeated code into a function";
                smells.push_back(smell);
                break;  // Only report once
            }
        }
    }
    
    return smells;
}

std::string AIAssistant::GenerateSecurityReport(const std::string& code) {
    auto issues = ScanSecurityVulnerabilities(code);
    
    if (issues.empty()) {
        return "✓ Security Scan Complete: No critical vulnerabilities detected.\n\n"
               "Your code follows basic security best practices.";
    }
    
    std::string report = "🔒 Security Analysis Report\n";
    report += "==========================\n\n";
    
    int critical = 0, high = 0, medium = 0, low = 0;
    for (const auto& issue : issues) {
        if (issue.severity == "critical") critical++;
        else if (issue.severity == "high") high++;
        else if (issue.severity == "medium") medium++;
        else if (issue.severity == "low") low++;
    }
    
    report += "Summary: " + std::to_string(issues.size()) + " issue(s) found\n";
    report += "  Critical: " + std::to_string(critical) + "\n";
    report += "  High: " + std::to_string(high) + "\n";
    report += "  Medium: " + std::to_string(medium) + "\n";
    report += "  Low: " + std::to_string(low) + "\n\n";
    
    report += "Issues:\n";
    report += "-------\n";
    
    for (const auto& issue : issues) {
        report += "\n[" + issue.severity + "] Line " + std::to_string(issue.line_number) + ": " + 
                  issue.type + "\n";
        report += "Description: " + issue.description + "\n";
        report += "Recommendation: " + issue.recommendation + "\n";
    }
    
    return report;
}

std::string AIAssistant::GeneratePerformanceReport(const std::string& code) {
    auto issues = SuggestPerformanceOptimizations(code);
    
    if (issues.empty()) {
        return "✓ Performance Analysis Complete: No major issues detected.\n\n"
               "Your code follows good performance practices.";
    }
    
    std::string report = "⚡ Performance Analysis Report\n";
    report += "=============================\n\n";
    
    report += "Found " + std::to_string(issues.size()) + " optimization opportunity(s)\n\n";
    
    // Sort by impact score
    std::vector<PerformanceIssue> sorted_issues = issues;
    std::sort(sorted_issues.begin(), sorted_issues.end(), 
              [](const PerformanceIssue& a, const PerformanceIssue& b) {
                  return a.impact_score > b.impact_score;
              });
    
    for (const auto& issue : sorted_issues) {
        report += "\n[Impact: " + std::to_string(issue.impact_score) + "/10] Line " + 
                  std::to_string(issue.line_number) + ": " + issue.type + "\n";
        report += "Issue: " + issue.description + "\n";
        report += "Optimization: " + issue.optimization + "\n";
    }
    
    return report;
}

// ============================================================================
// Version 1.3.0 Features: Learning Mode
// ============================================================================

void AIAssistant::EnableLearningMode(bool enabled) {
    learning_mode_enabled_ = enabled;
    if (enabled) {
        AddMessage(Message::Sender::ASSISTANT, 
                   "Learning mode enabled. I'll track your usage patterns to provide "
                   "personalized suggestions.");
    }
}

bool AIAssistant::IsLearningModeEnabled() const {
    return learning_mode_enabled_;
}

void AIAssistant::RecordUsagePattern(const std::string& feature, 
                                     const std::map<std::string, std::string>& params) {
    if (!learning_mode_enabled_) {
        return;
    }
    
    if (usage_patterns_.find(feature) == usage_patterns_.end()) {
        UsagePattern pattern;
        pattern.feature = feature;
        pattern.frequency = 0;
        pattern.last_used = std::chrono::system_clock::now();
        usage_patterns_[feature] = pattern;
    }
    
    auto& pattern = usage_patterns_[feature];
    pattern.frequency++;
    pattern.last_used = std::chrono::system_clock::now();
    
    // Track common parameters
    for (const auto& param : params) {
        std::string param_str = param.first + "=" + param.second;
        if (std::find(pattern.common_parameters.begin(), 
                      pattern.common_parameters.end(), 
                      param_str) == pattern.common_parameters.end()) {
            pattern.common_parameters.push_back(param_str);
        }
    }
}

std::vector<AIAssistant::PersonalizedSuggestion> AIAssistant::GetPersonalizedSuggestions(
    const std::string& context) {
    std::vector<PersonalizedSuggestion> suggestions;
    
    if (!learning_mode_enabled_ || usage_patterns_.empty()) {
        return suggestions;
    }
    
    std::string lower_context = context;
    std::transform(lower_context.begin(), lower_context.end(), lower_context.begin(), ::tolower);
    
    // Find most frequently used features
    std::vector<std::pair<std::string, int>> sorted_features;
    for (const auto& pair : usage_patterns_) {
        sorted_features.push_back({pair.first, pair.second.frequency});
    }
    std::sort(sorted_features.begin(), sorted_features.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Generate suggestions based on usage patterns
    for (size_t i = 0; i < std::min(size_t(3), sorted_features.size()); ++i) {
        const auto& feature = sorted_features[i].first;
        const auto& pattern = usage_patterns_.at(feature);
        
        PersonalizedSuggestion suggestion;
        
        if (feature == "wifi_connection" && pattern.frequency > 5) {
            suggestion.suggestion = "Create a WiFi connection helper function";
            suggestion.reasoning = "You frequently work with WiFi. A helper function would save time.";
            suggestion.relevance_score = 0.9f;
            suggestion.category = "code_pattern";
            suggestions.push_back(suggestion);
        } else if (feature == "gpio_operations" && pattern.frequency > 10) {
            suggestion.suggestion = "Consider using hardware interrupts for GPIO";
            suggestion.reasoning = "You use GPIO frequently. Interrupts can improve responsiveness.";
            suggestion.relevance_score = 0.8f;
            suggestion.category = "optimization";
            suggestions.push_back(suggestion);
        } else if (feature == "sensor_reading" && pattern.frequency > 7) {
            suggestion.suggestion = "Implement sensor data filtering";
            suggestion.reasoning = "Frequent sensor readings benefit from filtering for accuracy.";
            suggestion.relevance_score = 0.85f;
            suggestion.category = "feature";
            suggestions.push_back(suggestion);
        }
    }
    
    // Context-aware suggestions
    if (ContainsKeywords(lower_context, {"wifi", "network"})) {
        bool has_wifi_pattern = usage_patterns_.find("wifi_connection") != usage_patterns_.end();
        if (has_wifi_pattern && usage_patterns_.at("wifi_connection").frequency > 3) {
            PersonalizedSuggestion suggestion;
            suggestion.suggestion = "Use your usual WiFi configuration";
            suggestion.reasoning = "Based on your previous WiFi setups";
            suggestion.relevance_score = 0.95f;
            suggestion.category = "code_pattern";
            suggestions.push_back(suggestion);
        }
    }
    
    return suggestions;
}

std::vector<AIAssistant::UsagePattern> AIAssistant::GetUsagePatterns() const {
    std::vector<UsagePattern> patterns;
    for (const auto& pair : usage_patterns_) {
        patterns.push_back(pair.second);
    }
    return patterns;
}

void AIAssistant::ClearUsageHistory() {
    usage_patterns_.clear();
}

// ============================================================================
// Helper Methods
// ============================================================================

std::vector<std::string> AIAssistant::ExtractCodeLines(const std::string& code) const {
    std::vector<std::string> lines;
    std::istringstream stream(code);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

bool AIAssistant::IsHardcodedCredential(const std::string& line) const {
    // Check for common credential patterns
    std::string lower_line = line;
    std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
    
    if ((lower_line.find("password") != std::string::npos || 
         lower_line.find("pass") != std::string::npos ||
         lower_line.find("pwd") != std::string::npos ||
         lower_line.find("ssid") != std::string::npos ||
         lower_line.find("api_key") != std::string::npos ||
         lower_line.find("token") != std::string::npos) &&
        line.find("\"") != std::string::npos &&
        line.find("=") != std::string::npos) {
        
        // Check if it's not a placeholder
        if (line.find("YOUR_") == std::string::npos &&
            line.find("CHANGE_") == std::string::npos &&
            line.find("***") == std::string::npos) {
            return true;
        }
    }
    
    return false;
}

bool AIAssistant::HasBufferOverflowRisk(const std::string& line) const {
    // Check for unsafe functions
    return line.find("strcpy(") != std::string::npos ||
           line.find("strcat(") != std::string::npos ||
           line.find("sprintf(") != std::string::npos ||
           line.find("gets(") != std::string::npos;
}

int AIAssistant::CalculateComplexity(const std::string& code) const {
    int complexity = 1;  // Base complexity
    
    // Count decision points
    size_t pos = 0;
    std::vector<std::string> keywords = {"if", "else", "for", "while", "case", "&&", "||"};
    
    for (const auto& keyword : keywords) {
        pos = 0;
        while ((pos = code.find(keyword, pos)) != std::string::npos) {
            complexity++;
            pos += keyword.length();
        }
    }
    
    return complexity;
}

} // namespace esp32_ide
