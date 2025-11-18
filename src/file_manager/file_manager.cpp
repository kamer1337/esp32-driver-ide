#include "file_manager/file_manager.h"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace esp32_ide {

FileManager::FileManager() : current_file_("") {
    InitializeDefaultTemplates();
    InitializeFileTree();
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
    
    // Write file to disk
    std::ofstream file(files_[name].path);
    if (!file.is_open()) {
        return false;
    }
    
    file << files_[name].content;
    file.close();
    
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
    
    // Check if directory exists and is accessible
    std::ifstream test_file(project_path + "/.project");
    if (!test_file.is_open()) {
        // Try to load individual files from directory
        // For now, just set the path
        return true;
    }
    test_file.close();
    
    // In a full implementation, would:
    // 1. Read .project file for project metadata
    // 2. Scan directory for source files
    // 3. Load each file into memory
    // 4. Build file tree structure
    
    return true;
}

bool FileManager::SaveProject(const std::string& project_path) {
    project_path_ = project_path;
    
    // Save all modified files in the project
    bool all_saved = true;
    for (auto& pair : files_) {
        if (pair.second.is_modified) {
            // Construct full path
            std::string full_path = project_path + "/" + pair.second.path;
            
            // Write file to disk
            std::ofstream file(full_path);
            if (!file.is_open()) {
                all_saved = false;
                continue;
            }
            
            file << pair.second.content;
            file.close();
            
            pair.second.is_modified = false;
        }
    }
    
    // Save project metadata
    std::ofstream project_file(project_path + "/.project");
    if (project_file.is_open()) {
        project_file << "# ESP32 Driver IDE Project\n";
        project_file << "files=" << files_.size() << "\n";
        project_file.close();
    }
    
    return all_saved;
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
    
    // Bluetooth Low Energy template
    AddTemplate("ble_beacon", R"(#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "{{service_uuid}}"
#define CHARACTERISTIC_UUID "{{char_uuid}}"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE");
  
  BLEDevice::init("ESP32-BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  
  pCharacteristic->setValue("Hello BLE");
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("BLE Advertising Started");
}

void loop() {
  delay(2000);
})", "Bluetooth Low Energy beacon", {"bluetooth", "ble", "wireless"});
    
    // MQTT IoT template  
    AddTemplate("mqtt_iot", R"(#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "{{ssid}}";
const char* password = "{{password}}";
const char* mqtt_server = "{{mqtt_server}}";
const char* topic = "{{topic}}";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe(topic);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
})", "MQTT IoT communication", {"mqtt", "iot", "cloud"});
    
    // Deep Sleep template
    AddTemplate("deep_sleep", R"(#include <esp_sleep.h>

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  {{sleep_seconds}}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("Going to sleep...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Never runs
})", "Deep sleep power saving", {"power", "sleep", "battery"});
    
    // Real-time Clock template
    AddTemplate("rtc_time", R"(#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  
  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();
  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(' ');
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.println(now.second());
  delay(1000);
})", "Real-time clock with DS3231", {"rtc", "time", "i2c"});
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

// File tree operations
void FileManager::InitializeFileTree() {
    file_tree_root_ = std::make_unique<FileTreeNode>("Project", "/", true, nullptr);
}

void FileManager::RebuildFileTree() {
    if (!file_tree_root_) {
        InitializeFileTree();
    }
    
    // Clear existing children
    file_tree_root_->children.clear();
    
    // Build tree from files
    for (const auto& file : files_) {
        const std::string& path = file.second.path;
        
        // Parse path and create folder structure
        size_t pos = 0;
        size_t last_pos = 0;
        FileTreeNode* current = file_tree_root_.get();
        
        while ((pos = path.find('/', last_pos)) != std::string::npos) {
            std::string folder_name = path.substr(last_pos, pos - last_pos);
            if (!folder_name.empty()) {
                // Check if folder exists in children
                FileTreeNode* found = nullptr;
                for (auto& child : current->children) {
                    if (child->name == folder_name && child->is_folder) {
                        found = child.get();
                        break;
                    }
                }
                
                if (!found) {
                    // Create new folder
                    auto new_folder = std::make_unique<FileTreeNode>(
                        folder_name, 
                        path.substr(0, pos), 
                        true, 
                        current
                    );
                    found = new_folder.get();
                    current->children.push_back(std::move(new_folder));
                }
                
                current = found;
            }
            last_pos = pos + 1;
        }
        
        // Add file node
        std::string filename = path.substr(last_pos);
        if (!filename.empty()) {
            auto file_node = std::make_unique<FileTreeNode>(
                filename,
                path,
                false,
                current
            );
            current->children.push_back(std::move(file_node));
        }
    }
}

bool FileManager::CreateFolder(const std::string& path) {
    if (!file_tree_root_) {
        InitializeFileTree();
    }
    
    // Just trigger a rebuild - folders are virtual in this implementation
    RebuildFileTree();
    return true;
}

bool FileManager::MoveFileOrFolder(const std::string& src_path, const std::string& dest_path) {
    // Check if source file exists
    if (!FileExists(src_path)) {
        return false;
    }
    
    // Get file content
    FileInfo info = files_[src_path];
    
    // Update path
    info.path = dest_path;
    info.name = dest_path;
    
    // Remove old entry and add new
    files_.erase(src_path);
    files_[dest_path] = info;
    
    // Update current file if needed
    if (current_file_ == src_path) {
        current_file_ = dest_path;
    }
    
    // Rebuild tree
    RebuildFileTree();
    return true;
}

bool FileManager::RenameFileOrFolder(const std::string& path, const std::string& new_name) {
    // Extract directory from path
    size_t last_slash = path.find_last_of('/');
    std::string dir = (last_slash != std::string::npos) ? path.substr(0, last_slash + 1) : "";
    std::string new_path = dir + new_name;
    
    return MoveFileOrFolder(path, new_path);
}

FileManager::FileTreeNode* FileManager::FindNodeByPath(const std::string& path) const {
    if (!file_tree_root_) {
        return nullptr;
    }
    return FindNodeByPathRecursive(file_tree_root_.get(), path);
}

FileManager::FileTreeNode* FileManager::FindNodeByPathRecursive(
    FileTreeNode* node, 
    const std::string& path
) const {
    if (!node) {
        return nullptr;
    }
    
    if (node->path == path) {
        return node;
    }
    
    for (const auto& child : node->children) {
        FileTreeNode* found = FindNodeByPathRecursive(child.get(), path);
        if (found) {
            return found;
        }
    }
    
    return nullptr;
}

std::string FileManager::GetNodeFullPath(const FileTreeNode* node) const {
    if (!node) {
        return "";
    }
    return node->path;
}

} // namespace esp32_ide
