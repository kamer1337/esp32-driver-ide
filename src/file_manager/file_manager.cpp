#include "file_manager/file_manager.h"
#include <fstream>
#include <algorithm>

namespace esp32_ide {

FileManager::FileManager() : current_file_("") {}

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

} // namespace esp32_ide
