#include "ai_assistant/ai_assistant.h"
#include <algorithm>
#include <cctype>
#include <chrono>

namespace esp32_ide {

AIAssistant::AIAssistant() {
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

} // namespace esp32_ide
