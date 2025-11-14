#include "editor/collaboration.h"
#include <algorithm>

namespace esp32_ide {

CollaborationManager::CollaborationManager() 
    : content_version_(0), has_conflicts_(false), connection_status_(ConnectionStatus::DISCONNECTED) {
}

CollaborationManager::~CollaborationManager() = default;

bool CollaborationManager::CreateSession(const std::string& session_id, const std::string& initial_content) {
    session_id_ = session_id;
    content_ = initial_content;
    content_version_ = 0;
    users_.clear();
    pending_operations_.clear();
    cursors_.clear();
    connection_status_ = ConnectionStatus::CONNECTED;
    return true;
}

bool CollaborationManager::JoinSession(const std::string& session_id, const std::string& user_id, const std::string& user_name) {
    if (session_id != session_id_) {
        return false;
    }
    
    User user;
    user.id = user_id;
    user.name = user_name;
    user.color = "#" + std::to_string(std::hash<std::string>{}(user_id) % 0xFFFFFF);
    user.cursor_position = 0;
    user.is_active = true;
    user.last_seen = GetCurrentTimestamp();
    
    users_[user_id] = user;
    
    if (user_joined_callback_) {
        user_joined_callback_(user);
    }
    
    return true;
}

bool CollaborationManager::LeaveSession(const std::string& user_id) {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
        return false;
    }
    
    users_.erase(it);
    
    // Remove user's cursors
    cursors_.erase(
        std::remove_if(cursors_.begin(), cursors_.end(),
                      [&user_id](const Cursor& c) { return c.user_id == user_id; }),
        cursors_.end()
    );
    
    if (user_left_callback_) {
        user_left_callback_(user_id);
    }
    
    return true;
}

void CollaborationManager::CloseSession() {
    session_id_ = "";
    users_.clear();
    pending_operations_.clear();
    cursors_.clear();
    connection_status_ = ConnectionStatus::DISCONNECTED;
}

std::vector<CollaborationManager::User> CollaborationManager::GetActiveUsers() const {
    std::vector<User> active_users;
    for (const auto& pair : users_) {
        if (pair.second.is_active) {
            active_users.push_back(pair.second);
        }
    }
    return active_users;
}

CollaborationManager::User CollaborationManager::GetUser(const std::string& user_id) const {
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        return it->second;
    }
    return User();
}

void CollaborationManager::UpdateUserCursor(const std::string& user_id, size_t position) {
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        it->second.cursor_position = position;
        UpdateUserActivity(user_id);
    }
}

bool CollaborationManager::ApplyLocalEdit(const EditOperation& operation) {
    // Add to pending operations
    pending_operations_.push_back(operation);
    
    // Apply to local content
    ApplyOperationToContent(operation);
    content_version_++;
    
    NotifyContentChange();
    return true;
}

bool CollaborationManager::ApplyRemoteEdit(const EditOperation& operation) {
    // Transform against pending operations
    EditOperation transformed_op = operation;
    
    for (const auto& pending_op : pending_operations_) {
        transformed_op = TransformOperation(transformed_op, pending_op);
    }
    
    // Apply to content
    ApplyOperationToContent(transformed_op);
    content_version_++;
    
    NotifyContentChange();
    return true;
}

std::vector<CollaborationManager::EditOperation> CollaborationManager::GetPendingOperations() const {
    return pending_operations_;
}

void CollaborationManager::ClearPendingOperations() {
    pending_operations_.clear();
}

CollaborationManager::EditOperation CollaborationManager::TransformOperation(
    const EditOperation& op1, 
    const EditOperation& op2
) const {
    EditOperation transformed = op1;
    
    // Operational transformation logic
    if (op1.type == EditOperation::Type::INSERT && op2.type == EditOperation::Type::INSERT) {
        if (op2.position <= op1.position) {
            transformed.position += op2.content.length();
        }
    }
    else if (op1.type == EditOperation::Type::DELETE && op2.type == EditOperation::Type::INSERT) {
        if (op2.position <= op1.position) {
            transformed.position += op2.content.length();
        }
    }
    else if (op1.type == EditOperation::Type::INSERT && op2.type == EditOperation::Type::DELETE) {
        if (op2.position < op1.position) {
            transformed.position -= std::min(transformed.position - op2.position, op2.content.length());
        }
    }
    else if (op1.type == EditOperation::Type::DELETE && op2.type == EditOperation::Type::DELETE) {
        if (op2.position < op1.position) {
            transformed.position -= std::min(transformed.position - op2.position, op2.content.length());
        }
    }
    
    return transformed;
}

void CollaborationManager::BroadcastCursor(const Cursor& cursor) {
    // Update or add cursor
    auto it = std::find_if(cursors_.begin(), cursors_.end(),
                          [&cursor](const Cursor& c) { return c.user_id == cursor.user_id; });
    
    if (it != cursors_.end()) {
        *it = cursor;
    } else {
        cursors_.push_back(cursor);
    }
    
    UpdateUserActivity(cursor.user_id);
    
    if (cursor_update_callback_) {
        cursor_update_callback_(cursor);
    }
}

std::vector<CollaborationManager::Cursor> CollaborationManager::GetOtherCursors() const {
    return cursors_;
}

void CollaborationManager::ResolveConflict() {
    has_conflicts_ = false;
    conflict_description_ = "";
}

void CollaborationManager::ApplyOperationToContent(const EditOperation& operation) {
    switch (operation.type) {
        case EditOperation::Type::INSERT:
            if (operation.position <= content_.length()) {
                content_.insert(operation.position, operation.content);
            }
            break;
            
        case EditOperation::Type::DELETE:
            if (operation.position < content_.length()) {
                size_t delete_len = std::min(operation.content.length(), 
                                            content_.length() - operation.position);
                content_.erase(operation.position, delete_len);
            }
            break;
            
        case EditOperation::Type::REPLACE:
            if (operation.position < content_.length()) {
                size_t replace_len = std::min(operation.content.length(), 
                                             content_.length() - operation.position);
                content_.replace(operation.position, replace_len, operation.content);
            }
            break;
    }
}

size_t CollaborationManager::TransformPosition(size_t position, const EditOperation& operation) const {
    if (operation.type == EditOperation::Type::INSERT) {
        if (operation.position <= position) {
            return position + operation.content.length();
        }
    }
    else if (operation.type == EditOperation::Type::DELETE) {
        if (operation.position < position) {
            size_t delete_len = operation.content.length();
            if (position <= operation.position + delete_len) {
                return operation.position;
            } else {
                return position - delete_len;
            }
        }
    }
    return position;
}

void CollaborationManager::NotifyContentChange() {
    if (content_change_callback_) {
        content_change_callback_(content_);
    }
}

void CollaborationManager::UpdateUserActivity(const std::string& user_id) {
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        it->second.last_seen = GetCurrentTimestamp();
        it->second.is_active = true;
    }
}

long long CollaborationManager::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

} // namespace esp32_ide
