#ifndef COLLABORATION_H
#define COLLABORATION_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace esp32_ide {
namespace collaboration {

/**
 * @brief User information in a collaborative session
 */
struct User {
    std::string id;
    std::string name;
    std::string color;          // Color for cursor/selection display
    bool is_host;
    std::chrono::system_clock::time_point last_activity;
};

/**
 * @brief Document change operation for operational transformation
 */
struct DocumentOperation {
    enum class Type {
        INSERT,
        DELETE,
        REPLACE
    };

    Type type;
    int position;
    int length;
    std::string content;
    std::string user_id;
    int revision;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Cursor position and selection for a user
 */
struct CursorState {
    std::string user_id;
    int position;
    int selection_start;
    int selection_end;
    std::string file_path;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Collaborative editing session
 */
class CollaborationSession {
public:
    CollaborationSession(const std::string& session_id, const std::string& host_user_id);
    ~CollaborationSession() = default;

    // Getters
    const std::string& GetSessionId() const { return session_id_; }
    const std::string& GetHostUserId() const { return host_user_id_; }
    int GetRevision() const { return current_revision_; }
    bool IsActive() const { return is_active_; }

    // User management
    bool AddUser(const User& user);
    bool RemoveUser(const std::string& user_id);
    User* GetUser(const std::string& user_id);
    std::vector<User> GetActiveUsers() const;
    size_t GetUserCount() const { return users_.size(); }

    // Document operations
    void ApplyOperation(const DocumentOperation& op);
    std::vector<DocumentOperation> GetOperations(int from_revision) const;
    int GetLatestRevision() const { return current_revision_; }

    // Cursor tracking
    void UpdateCursor(const CursorState& cursor);
    std::vector<CursorState> GetCursors() const;
    CursorState GetUserCursor(const std::string& user_id) const;

    // Session control
    void Start();
    void Stop();
    void Pause();
    void Resume();

private:
    std::string session_id_;
    std::string host_user_id_;
    int current_revision_;
    bool is_active_;
    bool is_paused_;

    std::map<std::string, User> users_;
    std::vector<DocumentOperation> operations_;
    std::map<std::string, CursorState> cursors_;

    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point last_activity_;
};

/**
 * @brief Collaboration client for connecting to sessions
 */
class CollaborationClient {
public:
    CollaborationClient(const std::string& user_id, const std::string& user_name);
    ~CollaborationClient() = default;

    // Connection
    bool Connect(const std::string& server_url);
    bool Disconnect();
    bool IsConnected() const { return is_connected_; }

    // Session management
    std::string CreateSession();
    bool JoinSession(const std::string& session_id);
    bool LeaveSession();
    std::string GetCurrentSessionId() const { return current_session_id_; }

    // Document operations
    void SendOperation(const DocumentOperation& op);
    std::vector<DocumentOperation> ReceiveOperations();
    void SendCursorUpdate(const CursorState& cursor);

    // User information
    const std::string& GetUserId() const { return user_id_; }
    const std::string& GetUserName() const { return user_name_; }
    void SetUserColor(const std::string& color) { user_color_ = color; }

    // Callbacks
    using OperationCallback = std::function<void(const DocumentOperation&)>;
    using CursorUpdateCallback = std::function<void(const CursorState&)>;
    using UserJoinedCallback = std::function<void(const User&)>;
    using UserLeftCallback = std::function<void(const std::string& user_id)>;

    void SetOperationCallback(OperationCallback callback) { operation_callback_ = callback; }
    void SetCursorUpdateCallback(CursorUpdateCallback callback) { cursor_callback_ = callback; }
    void SetUserJoinedCallback(UserJoinedCallback callback) { user_joined_callback_ = callback; }
    void SetUserLeftCallback(UserLeftCallback callback) { user_left_callback_ = callback; }

private:
    std::string user_id_;
    std::string user_name_;
    std::string user_color_;
    std::string server_url_;
    std::string current_session_id_;
    bool is_connected_;

    OperationCallback operation_callback_;
    CursorUpdateCallback cursor_callback_;
    UserJoinedCallback user_joined_callback_;
    UserLeftCallback user_left_callback_;

    int local_revision_;
    std::vector<DocumentOperation> pending_operations_;
};

} // namespace collaboration
} // namespace esp32_ide

#endif // COLLABORATION_H
