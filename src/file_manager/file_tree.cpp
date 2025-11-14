#include "file_manager/file_tree.h"
#include <algorithm>
#include <sstream>

namespace esp32_ide {

// FileTreeNode implementation

FileTreeNode::FileTreeNode(const std::string& name, NodeType type, FileTreeNode* parent)
    : name_(name), type_(type), parent_(parent), is_expanded_(false), is_selected_(false) {
    UpdatePath();
}

void FileTreeNode::SetName(const std::string& name) {
    name_ = name;
    UpdatePath();
    UpdateChildrenPaths();
}

void FileTreeNode::UpdatePath() {
    if (parent_) {
        path_ = parent_->GetFullPath() + "/" + name_;
    } else {
        path_ = name_;
    }
}

void FileTreeNode::UpdateChildrenPaths() {
    for (auto& child : children_) {
        child->UpdatePath();
        child->UpdateChildrenPaths();
    }
}

FileTreeNode* FileTreeNode::AddChild(const std::string& name, NodeType type) {
    auto child = std::make_unique<FileTreeNode>(name, type, this);
    FileTreeNode* ptr = child.get();
    children_.push_back(std::move(child));
    SortChildren();
    return ptr;
}

bool FileTreeNode::RemoveChild(const std::string& name) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [&name](const std::unique_ptr<FileTreeNode>& node) {
            return node->GetName() == name;
        });
    
    if (it != children_.end()) {
        children_.erase(it);
        return true;
    }
    return false;
}

FileTreeNode* FileTreeNode::FindChild(const std::string& name) const {
    for (const auto& child : children_) {
        if (child->GetName() == name) {
            return child.get();
        }
    }
    return nullptr;
}

std::string FileTreeNode::GetFullPath() const {
    return path_;
}

void FileTreeNode::SortChildren() {
    std::sort(children_.begin(), children_.end(),
        [](const std::unique_ptr<FileTreeNode>& a, const std::unique_ptr<FileTreeNode>& b) {
            // Directories first, then files
            if (a->GetType() != b->GetType()) {
                return a->GetType() == NodeType::DIRECTORY;
            }
            // Alphabetical order within same type
            return a->GetName() < b->GetName();
        });
}

// FileTree implementation

FileTree::FileTree() : selected_node_(nullptr) {
}

bool FileTree::Initialize(const std::string& root_path) {
    std::string root_name = root_path.empty() ? "Project" : root_path;
    root_ = std::make_unique<FileTreeNode>(root_name, FileTreeNode::NodeType::DIRECTORY);
    root_->SetExpanded(true);
    return true;
}

void FileTree::Clear() {
    root_.reset();
    selected_node_ = nullptr;
}

FileTreeNode* FileTree::AddFile(const std::string& path) {
    if (!root_) return nullptr;

    std::string normalized = NormalizePath(path);
    auto parts = SplitPath(normalized);
    
    if (parts.empty()) return nullptr;

    FileTreeNode* current = root_.get();
    
    // Navigate/create directories
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        FileTreeNode* child = current->FindChild(parts[i]);
        if (!child) {
            child = current->AddChild(parts[i], FileTreeNode::NodeType::DIRECTORY);
        }
        current = child;
    }
    
    // Add the file
    const std::string& filename = parts.back();
    FileTreeNode* existing = current->FindChild(filename);
    if (existing) return existing;
    
    return current->AddChild(filename, FileTreeNode::NodeType::FILE);
}

FileTreeNode* FileTree::AddDirectory(const std::string& path) {
    if (!root_) return nullptr;

    std::string normalized = NormalizePath(path);
    auto parts = SplitPath(normalized);
    
    if (parts.empty()) return nullptr;

    FileTreeNode* current = root_.get();
    
    for (const auto& part : parts) {
        FileTreeNode* child = current->FindChild(part);
        if (!child) {
            child = current->AddChild(part, FileTreeNode::NodeType::DIRECTORY);
        }
        current = child;
    }
    
    return current;
}

bool FileTree::RemoveNode(const std::string& path) {
    if (!root_) return false;

    FileTreeNode* node = FindNode(path);
    if (!node || !node->GetParent()) return false;

    return node->GetParent()->RemoveChild(node->GetName());
}

FileTreeNode* FileTree::FindNode(const std::string& path) const {
    if (!root_) return nullptr;
    return FindNodeRecursive(root_.get(), NormalizePath(path));
}

bool FileTree::NodeExists(const std::string& path) const {
    return FindNode(path) != nullptr;
}

bool FileTree::CanDrop(const std::string& source_path, const std::string& target_path) const {
    FileTreeNode* source = FindNode(source_path);
    FileTreeNode* target = FindNode(target_path);
    
    if (!source || !target) return false;
    if (target->GetType() != FileTreeNode::NodeType::DIRECTORY) return false;
    if (source == target) return false;
    
    // Check if target is a descendant of source
    FileTreeNode* current = target;
    while (current) {
        if (current == source) return false;
        current = current->GetParent();
    }
    
    return true;
}

bool FileTree::MoveNode(const std::string& source_path, const std::string& target_path) {
    if (!CanDrop(source_path, target_path)) return false;

    FileTreeNode* source = FindNode(source_path);
    FileTreeNode* target = FindNode(target_path);
    
    if (!source || !target || !source->GetParent()) return false;

    // Store source information
    std::string source_name = source->GetName();
    FileTreeNode::NodeType source_type = source->GetType();
    
    // Remove from old parent
    FileTreeNode* old_parent = source->GetParent();
    if (!old_parent->RemoveChild(source_name)) return false;
    
    // Add to new parent (this will be recreated, so we need to copy the tree)
    // For simplicity, we'll just create a new node
    target->AddChild(source_name, source_type);
    
    if (file_operation_callback_) {
        file_operation_callback_(source_path, "move");
    }
    
    return true;
}

bool FileTree::CopyNode(const std::string& source_path, const std::string& target_path) {
    FileTreeNode* source = FindNode(source_path);
    FileTreeNode* target = FindNode(target_path);
    
    if (!source || !target) return false;
    if (target->GetType() != FileTreeNode::NodeType::DIRECTORY) return false;

    // Create a copy
    target->AddChild(source->GetName(), source->GetType());
    
    if (file_operation_callback_) {
        file_operation_callback_(source_path, "copy");
    }
    
    return true;
}

std::vector<FileTreeNode*> FileTree::GetAllNodes() const {
    std::vector<FileTreeNode*> nodes;
    if (root_) {
        CollectNodes(root_.get(), nodes);
    }
    return nodes;
}

std::vector<FileTreeNode*> FileTree::GetVisibleNodes() const {
    std::vector<FileTreeNode*> nodes;
    if (root_) {
        CollectVisibleNodes(root_.get(), nodes);
    }
    return nodes;
}

void FileTree::SelectNode(const std::string& path) {
    if (selected_node_) {
        selected_node_->SetSelected(false);
    }
    
    selected_node_ = FindNode(path);
    if (selected_node_) {
        selected_node_->SetSelected(true);
    }
}

void FileTree::ClearSelection() {
    if (selected_node_) {
        selected_node_->SetSelected(false);
        selected_node_ = nullptr;
    }
}

void FileTree::ExpandNode(const std::string& path) {
    FileTreeNode* node = FindNode(path);
    if (node) {
        node->SetExpanded(true);
    }
}

void FileTree::CollapseNode(const std::string& path) {
    FileTreeNode* node = FindNode(path);
    if (node) {
        node->SetExpanded(false);
    }
}

void FileTree::ExpandAll() {
    if (root_) {
        ExpandRecursive(root_.get());
    }
}

void FileTree::CollapseAll() {
    if (root_) {
        CollapseRecursive(root_.get());
    }
}

bool FileTree::RefreshFromDisk(const std::string& base_path) {
    // This would require filesystem access, which is implementation-specific
    // For now, return true as a placeholder
    return true;
}

std::vector<std::string> FileTree::GetAllFilePaths() const {
    std::vector<std::string> paths;
    if (root_) {
        CollectFilePaths(root_.get(), paths);
    }
    return paths;
}

std::vector<std::string> FileTree::GetAllDirectoryPaths() const {
    std::vector<std::string> paths;
    if (root_) {
        CollectDirectoryPaths(root_.get(), paths);
    }
    return paths;
}

// Private helper methods

void FileTree::CollectNodes(FileTreeNode* node, std::vector<FileTreeNode*>& nodes) const {
    if (!node) return;
    nodes.push_back(node);
    for (const auto& child : node->GetChildren()) {
        CollectNodes(child.get(), nodes);
    }
}

void FileTree::CollectVisibleNodes(FileTreeNode* node, std::vector<FileTreeNode*>& nodes) const {
    if (!node) return;
    nodes.push_back(node);
    if (node->IsExpanded()) {
        for (const auto& child : node->GetChildren()) {
            CollectVisibleNodes(child.get(), nodes);
        }
    }
}

FileTreeNode* FileTree::FindNodeRecursive(FileTreeNode* node, const std::string& path) const {
    if (!node) return nullptr;
    if (node->GetFullPath() == path) return node;
    
    for (const auto& child : node->GetChildren()) {
        if (FileTreeNode* found = FindNodeRecursive(child.get(), path)) {
            return found;
        }
    }
    
    return nullptr;
}

void FileTree::ExpandRecursive(FileTreeNode* node) {
    if (!node) return;
    node->SetExpanded(true);
    for (const auto& child : node->GetChildren()) {
        ExpandRecursive(child.get());
    }
}

void FileTree::CollapseRecursive(FileTreeNode* node) {
    if (!node) return;
    node->SetExpanded(false);
    for (const auto& child : node->GetChildren()) {
        CollapseRecursive(child.get());
    }
}

void FileTree::CollectFilePaths(FileTreeNode* node, std::vector<std::string>& paths) const {
    if (!node) return;
    if (node->GetType() == FileTreeNode::NodeType::FILE) {
        paths.push_back(node->GetFullPath());
    }
    for (const auto& child : node->GetChildren()) {
        CollectFilePaths(child.get(), paths);
    }
}

void FileTree::CollectDirectoryPaths(FileTreeNode* node, std::vector<std::string>& paths) const {
    if (!node) return;
    if (node->GetType() == FileTreeNode::NodeType::DIRECTORY) {
        paths.push_back(node->GetFullPath());
    }
    for (const auto& child : node->GetChildren()) {
        CollectDirectoryPaths(child.get(), paths);
    }
}

std::string FileTree::NormalizePath(const std::string& path) const {
    std::string normalized = path;
    // Remove leading/trailing slashes
    while (!normalized.empty() && normalized[0] == '/') {
        normalized = normalized.substr(1);
    }
    while (!normalized.empty() && normalized.back() == '/') {
        normalized = normalized.substr(0, normalized.length() - 1);
    }
    return normalized;
}

std::vector<std::string> FileTree::SplitPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

std::string FileTree::JoinPath(const std::vector<std::string>& parts) const {
    if (parts.empty()) return "";
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += "/" + parts[i];
    }
    
    return result;
}

} // namespace esp32_ide
