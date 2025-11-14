#ifndef COLLABORATION_H
#define COLLABORATION_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

namespace esp32_ide {

/**
 * @brief Real-time collaboration system for ESP32 IDE
 * 
 * Enables multiple users to work on the same code simultaneously
 * with operational transformation for conflict resolution
 */
class CollaborationManager {
public:
    struct User {
        std::string id;
        std::string name;
        std::string color;  // For cursor/selection highlighting
        size_t cursor_position;
        bool is_active;
        long long last_seen;
    };
    
    struct EditOperation {
        enum class Type {
            INSERT,
            DELETE,
            REPLACE
        };
        
        Type type;
        size_t position;
        std::string content;
        std::string user_id;
        long long timestamp;
        int version;
    };
    
    struct Cursor {
        std::string user_id;
        size_t position;
        size_t selection_start;
        size_t selection_end;
        long long timestamp;
    };
    
    CollaborationManager();
    ~CollaborationManager();
    
    // Session management
    bool CreateSession(const std::string& session_id, const std::string& initial_content);
    bool JoinSession(const std::string& session_id, const std::string& user_id, const std::string& user_name);
    bool LeaveSession(const std::string& user_id);
    void CloseSession();
    
    // User management
    std::vector<User> GetActiveUsers() const;
    User GetUser(const std::string& user_id) const;
    void UpdateUserCursor(const std::string& user_id, size_t position);
    
    // Edit operations
    bool ApplyLocalEdit(const EditOperation& operation);
    bool ApplyRemoteEdit(const EditOperation& operation);
    std::vector<EditOperation> GetPendingOperations() const;
    void ClearPendingOperations();
    
    // Operational transformation
    EditOperation TransformOperation(const EditOperation& op1, const EditOperation& op2) const;
    std::string GetTransformedContent() const { return content_; }
    
    // Cursor tracking
    void BroadcastCursor(const Cursor& cursor);
    std::vector<Cursor> GetOtherCursors() const;
    
    // Conflict resolution
    bool HasConflicts() const { return has_conflicts_; }
    std::string GetConflictDescription() const { return conflict_description_; }
    void ResolveConflict();
    
    // Connection status
    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    };
    
    ConnectionStatus GetConnectionStatus() const { return connection_status_; }
    void SetConnectionStatus(ConnectionStatus status) { connection_status_ = status; }
    
    // Callbacks
    using ContentChangeCallback = std::function<void(const std::string& new_content)>;
    using UserJoinedCallback = std::function<void(const User& user)>;
    using UserLeftCallback = std::function<void(const std::string& user_id)>;
    using CursorUpdateCallback = std::function<void(const Cursor& cursor)>;
    
    void SetContentChangeCallback(ContentChangeCallback callback) { content_change_callback_ = callback; }
    void SetUserJoinedCallback(UserJoinedCallback callback) { user_joined_callback_ = callback; }
    void SetUserLeftCallback(UserLeftCallback callback) { user_left_callback_ = callback; }
    void SetCursorUpdateCallback(CursorUpdateCallback callback) { cursor_update_callback_ = callback; }
    
private:
    // Session state
    std::string session_id_;
    std::string content_;
    int content_version_;
    std::map<std::string, User> users_;
    std::vector<EditOperation> pending_operations_;
    std::vector<Cursor> cursors_;
    
    // Conflict tracking
    bool has_conflicts_;
    std::string conflict_description_;
    
    // Connection
    ConnectionStatus connection_status_;
    
    // Callbacks
    ContentChangeCallback content_change_callback_;
    UserJoinedCallback user_joined_callback_;
    UserLeftCallback user_left_callback_;
    CursorUpdateCallback cursor_update_callback_;
    
    // Helper methods
    void ApplyOperationToContent(const EditOperation& operation);
    size_t TransformPosition(size_t position, const EditOperation& operation) const;
    void NotifyContentChange();
    void UpdateUserActivity(const std::string& user_id);
    long long GetCurrentTimestamp() const;
};

} // namespace esp32_ide

#endif // COLLABORATION_H
