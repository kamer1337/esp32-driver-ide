#ifndef FILE_TREE_H
#define FILE_TREE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace esp32_ide {

/**
 * @brief File tree node representing files and directories
 */
class FileTreeNode {
public:
    enum class NodeType {
        FILE,
        DIRECTORY
    };

    FileTreeNode(const std::string& name, NodeType type, FileTreeNode* parent = nullptr);
    ~FileTreeNode() = default;

    // Getters
    const std::string& GetName() const { return name_; }
    const std::string& GetPath() const { return path_; }
    NodeType GetType() const { return type_; }
    FileTreeNode* GetParent() const { return parent_; }
    const std::vector<std::unique_ptr<FileTreeNode>>& GetChildren() const { return children_; }
    bool IsExpanded() const { return is_expanded_; }
    bool IsSelected() const { return is_selected_; }

    // Setters
    void SetName(const std::string& name);
    void SetExpanded(bool expanded) { is_expanded_ = expanded; }
    void SetSelected(bool selected) { is_selected_ = selected; }

    // Children management
    FileTreeNode* AddChild(const std::string& name, NodeType type);
    bool RemoveChild(const std::string& name);
    FileTreeNode* FindChild(const std::string& name) const;
    bool HasChildren() const { return !children_.empty(); }
    size_t GetChildCount() const { return children_.size(); }

    // Path operations
    std::string GetFullPath() const;
    void UpdatePath();

    // Sorting
    void SortChildren();

private:
    std::string name_;
    std::string path_;
    NodeType type_;
    FileTreeNode* parent_;
    std::vector<std::unique_ptr<FileTreeNode>> children_;
    bool is_expanded_;
    bool is_selected_;

    void UpdateChildrenPaths();
};

/**
 * @brief File tree manager with drag-and-drop support
 */
class FileTree {
public:
    FileTree();
    ~FileTree() = default;

    // Tree operations
    bool Initialize(const std::string& root_path = "");
    void Clear();

    // Node operations
    FileTreeNode* AddFile(const std::string& path);
    FileTreeNode* AddDirectory(const std::string& path);
    bool RemoveNode(const std::string& path);
    FileTreeNode* FindNode(const std::string& path) const;
    bool NodeExists(const std::string& path) const;

    // Drag and drop operations
    bool CanDrop(const std::string& source_path, const std::string& target_path) const;
    bool MoveNode(const std::string& source_path, const std::string& target_path);
    bool CopyNode(const std::string& source_path, const std::string& target_path);

    // Navigation
    FileTreeNode* GetRoot() const { return root_.get(); }
    std::vector<FileTreeNode*> GetAllNodes() const;
    std::vector<FileTreeNode*> GetVisibleNodes() const;

    // Selection
    void SelectNode(const std::string& path);
    void ClearSelection();
    FileTreeNode* GetSelectedNode() const { return selected_node_; }

    // Expansion
    void ExpandNode(const std::string& path);
    void CollapseNode(const std::string& path);
    void ExpandAll();
    void CollapseAll();

    // File operations callbacks
    using FileOperationCallback = std::function<void(const std::string& path, const std::string& operation)>;
    void SetFileOperationCallback(FileOperationCallback callback) { file_operation_callback_ = callback; }

    // Refresh from disk
    bool RefreshFromDisk(const std::string& base_path);

    // Get all file paths
    std::vector<std::string> GetAllFilePaths() const;
    std::vector<std::string> GetAllDirectoryPaths() const;

private:
    std::unique_ptr<FileTreeNode> root_;
    FileTreeNode* selected_node_;
    FileOperationCallback file_operation_callback_;

    // Helper methods
    void CollectNodes(FileTreeNode* node, std::vector<FileTreeNode*>& nodes) const;
    void CollectVisibleNodes(FileTreeNode* node, std::vector<FileTreeNode*>& nodes) const;
    FileTreeNode* FindNodeRecursive(FileTreeNode* node, const std::string& path) const;
    void ExpandRecursive(FileTreeNode* node);
    void CollapseRecursive(FileTreeNode* node);
    void CollectFilePaths(FileTreeNode* node, std::vector<std::string>& paths) const;
    void CollectDirectoryPaths(FileTreeNode* node, std::vector<std::string>& paths) const;

    // Path utilities
    std::string NormalizePath(const std::string& path) const;
    std::vector<std::string> SplitPath(const std::string& path) const;
    std::string JoinPath(const std::vector<std::string>& parts) const;
};

} // namespace esp32_ide

#endif // FILE_TREE_H
