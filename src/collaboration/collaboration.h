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

/**
 * @brief Git integration for version control (Version 1.3.0)
 */
class GitIntegration {
public:
    enum class GitStatus {
        UNMODIFIED,
        MODIFIED,
        ADDED,
        DELETED,
        RENAMED,
        UNTRACKED
    };
    
    struct FileStatus {
        std::string path;
        GitStatus status;
        int additions;
        int deletions;
    };
    
    struct CommitInfo {
        std::string hash;
        std::string author;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::vector<std::string> files;
    };
    
    struct BranchInfo {
        std::string name;
        bool is_current;
        std::string last_commit;
    };
    
    GitIntegration();
    ~GitIntegration() = default;
    
    // Repository management
    bool InitRepository(const std::string& path);
    bool OpenRepository(const std::string& path);
    bool CloseRepository();
    bool IsRepositoryOpen() const { return is_repo_open_; }
    std::string GetRepositoryPath() const { return repo_path_; }
    
    // File operations
    std::vector<FileStatus> GetStatus();
    bool StageFile(const std::string& path);
    bool UnstageFile(const std::string& path);
    bool StageAll();
    
    // Commit operations
    bool Commit(const std::string& message, const std::string& author);
    std::vector<CommitInfo> GetCommitHistory(int max_count = 50);
    CommitInfo GetCommitInfo(const std::string& hash);
    
    // Branch operations
    std::vector<BranchInfo> GetBranches();
    bool CreateBranch(const std::string& name);
    bool SwitchBranch(const std::string& name);
    bool DeleteBranch(const std::string& name);
    std::string GetCurrentBranch();
    
    // Diff operations
    std::string GetDiff(const std::string& file_path);
    std::string GetDiffBetweenCommits(const std::string& commit1, const std::string& commit2);
    
    // Remote operations
    bool AddRemote(const std::string& name, const std::string& url);
    bool RemoveRemote(const std::string& name);
    bool Push(const std::string& remote, const std::string& branch);
    bool Pull(const std::string& remote, const std::string& branch);
    bool Fetch(const std::string& remote);
    
private:
    bool is_repo_open_;
    std::string repo_path_;
    std::string current_branch_;
    std::vector<FileStatus> staged_files_;
    std::vector<CommitInfo> commit_history_;
};

/**
 * @brief Code review tools (Version 1.3.0)
 */
class CodeReviewSystem {
public:
    enum class CommentType {
        GENERAL,
        SUGGESTION,
        ISSUE,
        QUESTION
    };
    
    enum class ReviewStatus {
        PENDING,
        APPROVED,
        CHANGES_REQUESTED,
        COMMENTED
    };
    
    struct ReviewComment {
        std::string id;
        std::string author;
        std::string file_path;
        int line_number;
        CommentType type;
        std::string content;
        std::chrono::system_clock::time_point timestamp;
        std::vector<std::string> replies;  // Reply IDs
    };
    
    struct CodeReview {
        std::string id;
        std::string title;
        std::string description;
        std::string author;
        ReviewStatus status;
        std::vector<std::string> reviewers;
        std::vector<std::string> comments;  // Comment IDs
        std::vector<std::string> changed_files;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point updated_at;
    };
    
    struct ChangeTracker {
        std::string file_path;
        int line_number;
        std::string change_type;  // "added", "modified", "deleted"
        std::string old_content;
        std::string new_content;
        std::string author;
        std::chrono::system_clock::time_point timestamp;
    };
    
    CodeReviewSystem();
    ~CodeReviewSystem() = default;
    
    // Review management
    std::string CreateReview(const std::string& title, const std::string& description,
                            const std::string& author);
    bool AddReviewer(const std::string& review_id, const std::string& reviewer);
    bool UpdateReviewStatus(const std::string& review_id, ReviewStatus status);
    CodeReview GetReview(const std::string& review_id);
    std::vector<CodeReview> GetAllReviews();
    std::vector<CodeReview> GetReviewsByAuthor(const std::string& author);
    
    // Comment management
    std::string AddComment(const std::string& review_id, const std::string& author,
                          const std::string& file_path, int line_number,
                          CommentType type, const std::string& content);
    bool AddReply(const std::string& comment_id, const std::string& reply);
    ReviewComment GetComment(const std::string& comment_id);
    std::vector<ReviewComment> GetCommentsForReview(const std::string& review_id);
    std::vector<ReviewComment> GetCommentsForFile(const std::string& review_id,
                                                  const std::string& file_path);
    bool ResolveComment(const std::string& comment_id);
    
    // Change tracking
    void TrackChange(const std::string& file_path, int line_number,
                    const std::string& change_type, const std::string& old_content,
                    const std::string& new_content, const std::string& author);
    std::vector<ChangeTracker> GetChanges(const std::string& file_path);
    std::vector<ChangeTracker> GetRecentChanges(int max_count = 50);
    void ClearChangeHistory();
    
    // Workflow
    bool StartReview(const std::string& review_id);
    bool ApproveReview(const std::string& review_id, const std::string& reviewer);
    bool RequestChanges(const std::string& review_id, const std::string& reviewer,
                       const std::string& reason);
    std::string GenerateReviewReport(const std::string& review_id);
    
private:
    std::map<std::string, CodeReview> reviews_;
    std::map<std::string, ReviewComment> comments_;
    std::vector<ChangeTracker> change_history_;
    int next_review_id_;
    int next_comment_id_;
    
    std::string GenerateId(const std::string& prefix);
};

} // namespace collaboration
} // namespace esp32_ide

#endif // COLLABORATION_H
