#include "collaboration/collaboration.h"
#include <algorithm>

namespace esp32_ide {
namespace collaboration {

// CollaborationSession implementation

CollaborationSession::CollaborationSession(const std::string& session_id, const std::string& host_user_id)
    : session_id_(session_id), host_user_id_(host_user_id), current_revision_(0),
      is_active_(false), is_paused_(false) {
    created_at_ = std::chrono::system_clock::now();
    last_activity_ = created_at_;
}

bool CollaborationSession::AddUser(const User& user) {
    if (users_.find(user.id) != users_.end()) {
        return false; // User already in session
    }
    users_[user.id] = user;
    last_activity_ = std::chrono::system_clock::now();
    return true;
}

bool CollaborationSession::RemoveUser(const std::string& user_id) {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
        return false;
    }
    users_.erase(it);
    cursors_.erase(user_id);
    last_activity_ = std::chrono::system_clock::now();
    return true;
}

User* CollaborationSession::GetUser(const std::string& user_id) {
    auto it = users_.find(user_id);
    return (it != users_.end()) ? &it->second : nullptr;
}

std::vector<User> CollaborationSession::GetActiveUsers() const {
    std::vector<User> result;
    for (const auto& pair : users_) {
        result.push_back(pair.second);
    }
    return result;
}

void CollaborationSession::ApplyOperation(const DocumentOperation& op) {
    operations_.push_back(op);
    current_revision_++;
    last_activity_ = std::chrono::system_clock::now();
}

std::vector<DocumentOperation> CollaborationSession::GetOperations(int from_revision) const {
    std::vector<DocumentOperation> result;
    for (const auto& op : operations_) {
        if (op.revision >= from_revision) {
            result.push_back(op);
        }
    }
    return result;
}

void CollaborationSession::UpdateCursor(const CursorState& cursor) {
    cursors_[cursor.user_id] = cursor;
    last_activity_ = std::chrono::system_clock::now();
}

std::vector<CursorState> CollaborationSession::GetCursors() const {
    std::vector<CursorState> result;
    for (const auto& pair : cursors_) {
        result.push_back(pair.second);
    }
    return result;
}

CursorState CollaborationSession::GetUserCursor(const std::string& user_id) const {
    auto it = cursors_.find(user_id);
    if (it != cursors_.end()) {
        return it->second;
    }
    return CursorState();
}

void CollaborationSession::Start() {
    is_active_ = true;
    is_paused_ = false;
    last_activity_ = std::chrono::system_clock::now();
}

void CollaborationSession::Stop() {
    is_active_ = false;
    is_paused_ = false;
}

void CollaborationSession::Pause() {
    is_paused_ = true;
}

void CollaborationSession::Resume() {
    is_paused_ = false;
    last_activity_ = std::chrono::system_clock::now();
}

// CollaborationClient implementation

CollaborationClient::CollaborationClient(const std::string& user_id, const std::string& user_name)
    : user_id_(user_id), user_name_(user_name), user_color_("#FF0000"),
      is_connected_(false), local_revision_(0) {
}

bool CollaborationClient::Connect(const std::string& server_url) {
    server_url_ = server_url;
    // In a real implementation, this would establish a network connection
    is_connected_ = true;
    return true;
}

bool CollaborationClient::Disconnect() {
    if (!is_connected_) return false;
    
    if (!current_session_id_.empty()) {
        LeaveSession();
    }
    
    is_connected_ = false;
    return true;
}

std::string CollaborationClient::CreateSession() {
    if (!is_connected_) return "";
    
    // In a real implementation, this would communicate with server
    current_session_id_ = "session_" + user_id_;
    local_revision_ = 0;
    return current_session_id_;
}

bool CollaborationClient::JoinSession(const std::string& session_id) {
    if (!is_connected_) return false;
    
    current_session_id_ = session_id;
    local_revision_ = 0;
    
    if (user_joined_callback_) {
        User user;
        user.id = user_id_;
        user.name = user_name_;
        user.color = user_color_;
        user.is_host = false;
        user_joined_callback_(user);
    }
    
    return true;
}

bool CollaborationClient::LeaveSession() {
    if (current_session_id_.empty()) return false;
    
    if (user_left_callback_) {
        user_left_callback_(user_id_);
    }
    
    current_session_id_.clear();
    local_revision_ = 0;
    pending_operations_.clear();
    
    return true;
}

void CollaborationClient::SendOperation(const DocumentOperation& op) {
    if (!is_connected_ || current_session_id_.empty()) return;
    
    // Store in pending operations
    pending_operations_.push_back(op);
    
    // In a real implementation, send to server
}

std::vector<DocumentOperation> CollaborationClient::ReceiveOperations() {
    if (!is_connected_ || current_session_id_.empty()) {
        return {};
    }
    
    // In a real implementation, receive from server
    std::vector<DocumentOperation> ops;
    
    // Process received operations
    for (const auto& op : ops) {
        if (operation_callback_) {
            operation_callback_(op);
        }
        local_revision_ = std::max(local_revision_, op.revision);
    }
    
    return ops;
}

void CollaborationClient::SendCursorUpdate(const CursorState& cursor) {
    if (!is_connected_ || current_session_id_.empty()) return;
    
    // In a real implementation, send to server
}

} // namespace collaboration
} // namespace esp32_ide
