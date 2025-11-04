#include "file_manager/file_manager.h"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace esp32_ide {

FileManager::FileManager() : current_file_("") {
    InitializeDefaultTemplates();
}

FileManager::~FileManager() = default;

bool FileManager::CreateFile(const std::string& name, const std::string& initial_content) {
    if (FileExists(name)) {
        return false; // File already exists
    }
    
    FileInfo info;
    info.name = name;
    info.content = initial_content.empty() ? GetDefaultSketch() : initial_content;
    info.path = name;
    info.is_modified = false;
    info.is_open = false;
    
    files_[name] = info;
    return true;
}

bool FileManager::OpenFile(const std::string& name) {
    if (!FileExists(name)) {
        return false;
    }
    
    files_[name].is_open = true;
    SetCurrentFile(name);
    return true;
}

bool FileManager::SaveFile(const std::string& name) {
    if (!FileExists(name)) {
        return false;
    }
    
    // In a real implementation, this would write to disk
    files_[name].is_modified = false;
    return true;
}

bool FileManager::SaveFileAs(const std::string& old_name, const std::string& new_name) {
    if (!FileExists(old_name) || FileExists(new_name)) {
        return false;
    }
    
    FileInfo info = files_[old_name];
    info.name = new_name;
    info.path = new_name;
    files_[new_name] = info;
    
    if (current_file_ == old_name) {
        current_file_ = new_name;
    }
    
    return true;
}

bool FileManager::CloseFile(const std::string& name) {
    if (!FileExists(name)) {
        return false;
    }
    
    files_[name].is_open = false;
    if (current_file_ == name) {
        current_file_ = "";
    }
    return true;
}

bool FileManager::DeleteFile(const std::string& name) {
    if (!FileExists(name)) {
        return false;
    }
    
    files_.erase(name);
    if (current_file_ == name) {
        current_file_ = "";
    }
    return true;
}

bool FileManager::SetFileContent(const std::string& name, const std::string& content) {
    if (!FileExists(name)) {
        return false;
    }
    
    files_[name].content = content;
    MarkAsModified(name);
    return true;
}

std::string FileManager::GetFileContent(const std::string& name) const {
    auto it = files_.find(name);
    if (it != files_.end()) {
        return it->second.content;
    }
    return "";
}

bool FileManager::FileExists(const std::string& name) const {
    return files_.find(name) != files_.end();
}

bool FileManager::IsFileModified(const std::string& name) const {
    auto it = files_.find(name);
    if (it != files_.end()) {
        return it->second.is_modified;
    }
    return false;
}

std::vector<std::string> FileManager::GetFileList() const {
    std::vector<std::string> list;
    for (const auto& pair : files_) {
        list.push_back(pair.first);
    }
    return list;
}

FileManager::FileInfo FileManager::GetFileInfo(const std::string& name) const {
    auto it = files_.find(name);
    if (it != files_.end()) {
        return it->second;
    }
    return FileInfo();
}

void FileManager::SetCurrentFile(const std::string& name) {
    if (FileExists(name)) {
        current_file_ = name;
    }
}

std::string FileManager::GetCurrentFile() const {
    return current_file_;
}

bool FileManager::LoadProject(const std::string& project_path) {
    project_path_ = project_path;
    // In a real implementation, this would load files from disk
    return true;
}

bool FileManager::SaveProject(const std::string& project_path) {
    project_path_ = project_path;
    // In a real implementation, this would save files to disk
    return true;
}

std::string FileManager::GetDefaultSketch() {
    return R"(// ESP32 Blink Example
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
})";
}

void FileManager::MarkAsModified(const std::string& name, bool modified) {
    if (FileExists(name)) {
        files_[name].is_modified = modified;
    }
}

// Custom code templates implementation
void FileManager::InitializeDefaultTemplates() {
    // Basic Arduino template
    AddTemplate("basic", R"(void setup() {
  // Initialize
}

void loop() {
  // Main code
})", "Basic Arduino sketch", {"basic", "arduino"});
    
    // LED Blink template
    AddTemplate("led_blink", R"(#define LED_PIN {{pin}}

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
})", "LED blink program", {"led", "gpio", "basic"});
    
    // WiFi template
    AddTemplate("wifi_connect", R"(#include <WiFi.h>

const char* ssid = "{{ssid}}";
const char* password = "{{password}}";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Your code here
})", "WiFi connection setup", {"wifi", "network"});
    
    // Web Server template
    AddTemplate("web_server", R"(#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "{{ssid}}";
const char* password = "{{password}}";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", "<h1>ESP32 Web Server</h1>");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
})", "Basic web server", {"wifi", "server", "web"});
    
    // Sensor Reading template
    AddTemplate("sensor_read", R"(#define SENSOR_PIN {{pin}}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  int value = analogRead(SENSOR_PIN);
  Serial.print("Sensor Value: ");
  Serial.println(value);
  delay(1000);
})", "Analog sensor reading", {"sensor", "analog", "adc"});
}

void FileManager::AddTemplate(const std::string& name, const std::string& code, 
                              const std::string& description, 
                              const std::vector<std::string>& tags) {
    CodeTemplate tmpl;
    tmpl.name = name;
    tmpl.code = code;
    tmpl.description = description;
    tmpl.tags = tags;
    templates_[name] = tmpl;
}

bool FileManager::DeleteTemplate(const std::string& name) {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        templates_.erase(it);
        return true;
    }
    return false;
}

std::vector<FileManager::CodeTemplate> FileManager::GetTemplates() const {
    std::vector<CodeTemplate> result;
    for (const auto& pair : templates_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<FileManager::CodeTemplate> FileManager::GetTemplatesByTag(const std::string& tag) const {
    std::vector<CodeTemplate> result;
    for (const auto& pair : templates_) {
        const auto& tmpl = pair.second;
        if (std::find(tmpl.tags.begin(), tmpl.tags.end(), tag) != tmpl.tags.end()) {
            result.push_back(tmpl);
        }
    }
    return result;
}

FileManager::CodeTemplate FileManager::GetTemplate(const std::string& name) const {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        return it->second;
    }
    return CodeTemplate(); // Return empty template
}

bool FileManager::TemplateExists(const std::string& name) const {
    return templates_.find(name) != templates_.end();
}

std::string FileManager::ApplyTemplate(const std::string& template_name, 
                                      const std::map<std::string, std::string>& variables) {
    if (!TemplateExists(template_name)) {
        return "";
    }
    
    std::string code = templates_[template_name].code;
    
    // Replace variables in the template
    for (const auto& var : variables) {
        std::string placeholder = "{{" + var.first + "}}";
        size_t pos = 0;
        while ((pos = code.find(placeholder, pos)) != std::string::npos) {
            code.replace(pos, placeholder.length(), var.second);
            pos += var.second.length();
        }
    }
    
    return code;
}

} // namespace esp32_ide
