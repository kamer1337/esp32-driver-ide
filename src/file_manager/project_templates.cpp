#include "file_manager/project_templates.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>

namespace esp32_ide {

// ProjectTemplate implementation

ProjectTemplate::ProjectTemplate(const std::string& id, const std::string& name)
    : id_(id), name_(name), category_("General") {
}

void ProjectTemplate::AddFile(const std::string& path, const std::string& content) {
    TemplateFile file;
    file.path = path;
    file.content = content;
    file.is_directory = false;
    files_.push_back(file);
}

void ProjectTemplate::AddDirectory(const std::string& path) {
    TemplateFile dir;
    dir.path = path;
    dir.is_directory = true;
    files_.push_back(dir);
}

void ProjectTemplate::SetVariable(const std::string& name, const std::string& value) {
    variables_[name] = value;
}

std::string ProjectTemplate::ProcessContent(const std::string& content) const {
    std::string result = content;
    
    for (const auto& var : variables_) {
        std::string placeholder = "${" + var.first + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), var.second);
            pos += var.second.length();
        }
    }
    
    return result;
}

void ProjectTemplate::ApplyVariables(const std::map<std::string, std::string>& user_vars) {
    for (const auto& var : user_vars) {
        variables_[var.first] = var.second;
    }
}

// ProjectTemplateManager implementation

ProjectTemplateManager::ProjectTemplateManager() {
}

bool ProjectTemplateManager::Initialize() {
    RegisterBuiltInTemplates();
    return true;
}

void ProjectTemplateManager::Shutdown() {
    templates_.clear();
}

void ProjectTemplateManager::RegisterTemplate(std::unique_ptr<ProjectTemplate> template_obj) {
    if (template_obj) {
        templates_[template_obj->GetId()] = std::move(template_obj);
    }
}

bool ProjectTemplateManager::UnregisterTemplate(const std::string& template_id) {
    return templates_.erase(template_id) > 0;
}

ProjectTemplate* ProjectTemplateManager::GetTemplate(const std::string& template_id) const {
    auto it = templates_.find(template_id);
    return it != templates_.end() ? it->second.get() : nullptr;
}

std::vector<std::string> ProjectTemplateManager::GetTemplateIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : templates_) {
        ids.push_back(pair.first);
    }
    return ids;
}

std::vector<ProjectTemplate*> ProjectTemplateManager::GetAllTemplates() const {
    std::vector<ProjectTemplate*> result;
    for (const auto& pair : templates_) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::vector<ProjectTemplate*> ProjectTemplateManager::GetTemplatesByCategory(const std::string& category) const {
    std::vector<ProjectTemplate*> result;
    for (const auto& pair : templates_) {
        if (pair.second->GetCategory() == category) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

std::vector<ProjectTemplate*> ProjectTemplateManager::GetTemplatesByTag(const std::string& tag) const {
    std::vector<ProjectTemplate*> result;
    for (const auto& pair : templates_) {
        const auto& tags = pair.second->GetTags();
        if (std::find(tags.begin(), tags.end(), tag) != tags.end()) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

std::vector<std::string> ProjectTemplateManager::GetCategories() const {
    std::set<std::string> categories;
    for (const auto& pair : templates_) {
        categories.insert(pair.second->GetCategory());
    }
    return std::vector<std::string>(categories.begin(), categories.end());
}

std::vector<std::string> ProjectTemplateManager::GetTags() const {
    std::set<std::string> all_tags;
    for (const auto& pair : templates_) {
        const auto& tags = pair.second->GetTags();
        all_tags.insert(tags.begin(), tags.end());
    }
    return std::vector<std::string>(all_tags.begin(), all_tags.end());
}

bool ProjectTemplateManager::CreateProject(const CreateProjectOptions& options) {
    return CreateProjectFromTemplate(options.template_id, options.project_path, options.variables);
}

bool ProjectTemplateManager::CreateProjectFromTemplate(const std::string& template_id, const std::string& project_path,
                                                       const std::map<std::string, std::string>& variables) {
    ProjectTemplate* tmpl = GetTemplate(template_id);
    if (!tmpl) return false;

    // Apply user variables
    ProjectTemplate temp_copy = *tmpl;
    temp_copy.ApplyVariables(variables);

    // Create directory structure and files
    if (!CreateDirectoryStructure(project_path, temp_copy.GetFiles())) {
        return false;
    }

    if (!WriteTemplateFiles(project_path, temp_copy.GetFiles())) {
        return false;
    }

    NotifyProjectCreated(project_path);
    return true;
}

void ProjectTemplateManager::RegisterBuiltInTemplates() {
    RegisterTemplate(CreateBasicSketchTemplate());
    RegisterTemplate(CreateWiFiProjectTemplate());
    RegisterTemplate(CreateBluetoothProjectTemplate());
    RegisterTemplate(CreateWebServerTemplate());
    RegisterTemplate(CreateIoTSensorTemplate());
    RegisterTemplate(CreateLEDControlTemplate());
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateBasicSketchTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("basic_sketch", "Basic Arduino Sketch");
    tmpl->SetDescription("A minimal Arduino sketch for ESP32");
    tmpl->SetCategory("Basic");
    tmpl->AddTag("beginner");
    tmpl->AddTag("simple");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("${PROJECT_NAME} started");
}

void loop() {
  // Your code here
  delay(1000);
}
)");

    return tmpl;
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateWiFiProjectTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("wifi_project", "WiFi Project");
    tmpl->SetDescription("ESP32 project with WiFi connectivity");
    tmpl->SetCategory("Connectivity");
    tmpl->AddTag("wifi");
    tmpl->AddTag("network");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
#include <WiFi.h>

const char* ssid = "${WIFI_SSID}";
const char* password = "${WIFI_PASSWORD}";

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Your WiFi-enabled code here
  delay(1000);
}
)");

    tmpl->SetVariable("WIFI_SSID", "your_ssid");
    tmpl->SetVariable("WIFI_PASSWORD", "your_password");

    return tmpl;
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateBluetoothProjectTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("bluetooth_project", "Bluetooth Project");
    tmpl->SetDescription("ESP32 project with Bluetooth Classic support");
    tmpl->SetCategory("Connectivity");
    tmpl->AddTag("bluetooth");
    tmpl->AddTag("wireless");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("${BT_DEVICE_NAME}");
  Serial.println("Bluetooth device started, you can pair it now");
}

void loop() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    Serial.write(c);
  }
  
  if (Serial.available()) {
    char c = Serial.read();
    SerialBT.write(c);
  }
  
  delay(20);
}
)");

    tmpl->SetVariable("BT_DEVICE_NAME", "ESP32_BT");

    return tmpl;
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateWebServerTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("web_server", "Web Server");
    tmpl->SetDescription("ESP32 web server with basic HTML interface");
    tmpl->SetCategory("Web");
    tmpl->AddTag("http");
    tmpl->AddTag("server");
    tmpl->AddTag("iot");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "${WIFI_SSID}";
const char* password = "${WIFI_PASSWORD}";

WebServer server(80);

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>${PROJECT_NAME}</h1>";
  html += "<p>ESP32 Web Server</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
)");

    tmpl->SetVariable("WIFI_SSID", "your_ssid");
    tmpl->SetVariable("WIFI_PASSWORD", "your_password");

    return tmpl;
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateIoTSensorTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("iot_sensor", "IoT Sensor Project");
    tmpl->SetDescription("ESP32 IoT project with sensor data collection");
    tmpl->SetCategory("IoT");
    tmpl->AddTag("sensor");
    tmpl->AddTag("iot");
    tmpl->AddTag("data");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
#include <WiFi.h>

const char* ssid = "${WIFI_SSID}";
const char* password = "${WIFI_PASSWORD}";

const int sensorPin = ${SENSOR_PIN};

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  
  Serial.print("Sensor value: ");
  Serial.println(sensorValue);
  
  // Send data to cloud or server here
  
  delay(5000);
}
)");

    tmpl->SetVariable("WIFI_SSID", "your_ssid");
    tmpl->SetVariable("WIFI_PASSWORD", "your_password");
    tmpl->SetVariable("SENSOR_PIN", "34");

    return tmpl;
}

std::unique_ptr<ProjectTemplate> ProjectTemplateManager::CreateLEDControlTemplate() {
    auto tmpl = std::make_unique<ProjectTemplate>("led_control", "LED Control");
    tmpl->SetDescription("Simple LED control with button");
    tmpl->SetCategory("Basic");
    tmpl->AddTag("led");
    tmpl->AddTag("gpio");
    tmpl->AddTag("beginner");

    tmpl->AddFile("${PROJECT_NAME}.ino", R"(
const int ledPin = ${LED_PIN};
const int buttonPin = ${BUTTON_PIN};

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  
  if (buttonState == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF");
  }
  
  delay(100);
}
)");

    tmpl->SetVariable("LED_PIN", "2");
    tmpl->SetVariable("BUTTON_PIN", "4");

    return tmpl;
}

bool ProjectTemplateManager::CreateDirectoryStructure(const std::string& base_path, 
                                                      const std::vector<TemplateFile>& files) {
    // Extract unique directory paths from files
    std::set<std::string> directories;
    for (const auto& file : files) {
        size_t last_slash = file.path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            std::string dir = file.path.substr(0, last_slash);
            directories.insert(dir);
        }
    }
    
    // Create each directory
    // Note: In a cross-platform implementation, would use std::filesystem (C++17)
    // or platform-specific APIs. For now, using basic approach.
    for (const auto& dir : directories) {
        std::string full_path = base_path + "/" + dir;
        
        // Try to create directory by creating a test file
        // This is a workaround for systems without filesystem support
        std::ofstream test(full_path + "/.gitkeep");
        if (test.is_open()) {
            test.close();
        }
    }
    
    return true;
}

bool ProjectTemplateManager::WriteTemplateFiles(const std::string& base_path, 
                                               const std::vector<TemplateFile>& files) {
    bool all_written = true;
    
    for (const auto& file : files) {
        std::string full_path = base_path + "/" + file.path;
        
        // Create directory for file if needed
        size_t last_slash = file.path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            std::string dir = file.path.substr(0, last_slash);
            std::string dir_path = base_path + "/" + dir;
            
            // Ensure directory exists
            std::ofstream dir_test(dir_path + "/.gitkeep");
            if (dir_test.is_open()) {
                dir_test.close();
            }
        }
        
        // Write file content
        std::ofstream out_file(full_path);
        if (!out_file.is_open()) {
            all_written = false;
            continue;
        }
        
        out_file << file.content;
        out_file.close();
    }
    
    return all_written;
}

void ProjectTemplateManager::NotifyProjectCreated(const std::string& path) {
    if (project_created_callback_) {
        project_created_callback_(path);
    }
}

} // namespace esp32_ide
