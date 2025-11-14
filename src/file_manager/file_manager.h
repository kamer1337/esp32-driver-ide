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
    
    // File tree structure for hierarchical navigation
    struct FileTreeNode {
        std::string name;
        std::string path;
        bool is_folder;
        FileTreeNode* parent;
        std::vector<std::unique_ptr<FileTreeNode>> children;
        
        FileTreeNode(const std::string& n, const std::string& p, bool folder, FileTreeNode* par = nullptr)
            : name(n), path(p), is_folder(folder), parent(par) {}
    };
    
    // File tree operations
    FileTreeNode* GetFileTree() const { return file_tree_root_.get(); }
    bool CreateFolder(const std::string& path);
    bool MoveFileOrFolder(const std::string& src_path, const std::string& dest_path);
    bool RenameFileOrFolder(const std::string& path, const std::string& new_name);
    FileTreeNode* FindNodeByPath(const std::string& path) const;
    std::string GetNodeFullPath(const FileTreeNode* node) const;
    
    // Custom code templates
    struct CodeTemplate {
        std::string name;
        std::string description;
        std::string code;
        std::vector<std::string> tags;
    };
    
    void AddTemplate(const std::string& name, const std::string& code, 
                     const std::string& description = "", 
                     const std::vector<std::string>& tags = {});
    bool DeleteTemplate(const std::string& name);
    std::vector<CodeTemplate> GetTemplates() const;
    std::vector<CodeTemplate> GetTemplatesByTag(const std::string& tag) const;
    CodeTemplate GetTemplate(const std::string& name) const;
    bool TemplateExists(const std::string& name) const;
    std::string ApplyTemplate(const std::string& template_name, 
                             const std::map<std::string, std::string>& variables = {});
    
private:
    std::map<std::string, FileInfo> files_;
    std::string current_file_;
    std::string project_path_;
    std::map<std::string, CodeTemplate> templates_;
    std::unique_ptr<FileTreeNode> file_tree_root_;
    
    void MarkAsModified(const std::string& name, bool modified = true);
    void InitializeDefaultTemplates();
    void InitializeFileTree();
    void RebuildFileTree();
    FileTreeNode* FindNodeByPathRecursive(FileTreeNode* node, const std::string& path) const;
};

} // namespace esp32_ide

#endif // FILE_MANAGER_H
