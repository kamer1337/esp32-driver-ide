#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace esp32_ide {

/**
 * @brief File management system for the IDE
 * 
 * Handles file operations, project management, and file storage
 */
class FileManager {
public:
    struct FileInfo {
        std::string name;
        std::string content;
        std::string path;
        bool is_modified;
        bool is_open;
    };
    
    FileManager();
    ~FileManager();
    
    // File operations
    bool CreateFile(const std::string& name, const std::string& initial_content = "");
    bool OpenFile(const std::string& name);
    bool SaveFile(const std::string& name);
    bool SaveFileAs(const std::string& old_name, const std::string& new_name);
    bool CloseFile(const std::string& name);
    bool DeleteFile(const std::string& name);
    
    // File content
    bool SetFileContent(const std::string& name, const std::string& content);
    std::string GetFileContent(const std::string& name) const;
    
    // File information
    bool FileExists(const std::string& name) const;
    bool IsFileModified(const std::string& name) const;
    std::vector<std::string> GetFileList() const;
    FileInfo GetFileInfo(const std::string& name) const;
    
    // Current file
    void SetCurrentFile(const std::string& name);
    std::string GetCurrentFile() const;
    
    // Project operations
    bool LoadProject(const std::string& project_path);
    bool SaveProject(const std::string& project_path);
    
    // Default sketch
    static std::string GetDefaultSketch();
    
private:
    std::map<std::string, FileInfo> files_;
    std::string current_file_;
    std::string project_path_;
    
    void MarkAsModified(const std::string& name, bool modified = true);
};

} // namespace esp32_ide

#endif // FILE_MANAGER_H
