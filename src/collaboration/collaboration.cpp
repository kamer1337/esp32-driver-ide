#include "collaboration/collaboration.h"
#include <algorithm>
#include <sstream>

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

// ============================================================================
// GitIntegration Implementation (Version 1.3.0)
// ============================================================================

GitIntegration::GitIntegration() : is_repo_open_(false), current_branch_("main") {}

bool GitIntegration::InitRepository(const std::string& path) {
    repo_path_ = path;
    is_repo_open_ = true;
    current_branch_ = "main";
    
    // In real implementation, would use libgit2 or similar
    // For now, simulate repository initialization
    return true;
}

bool GitIntegration::OpenRepository(const std::string& path) {
    repo_path_ = path;
    is_repo_open_ = true;
    
    // Simulate opening repository
    return true;
}

bool GitIntegration::CloseRepository() {
    if (!is_repo_open_) return false;
    
    is_repo_open_ = false;
    repo_path_.clear();
    staged_files_.clear();
    
    return true;
}

std::vector<GitIntegration::FileStatus> GitIntegration::GetStatus() {
    if (!is_repo_open_) return {};
    
    // In real implementation, would check actual git status
    // Simulated for demonstration
    return staged_files_;
}

bool GitIntegration::StageFile(const std::string& path) {
    if (!is_repo_open_) return false;
    
    FileStatus status;
    status.path = path;
    status.status = GitStatus::MODIFIED;
    status.additions = 0;
    status.deletions = 0;
    
    staged_files_.push_back(status);
    return true;
}

bool GitIntegration::UnstageFile(const std::string& path) {
    if (!is_repo_open_) return false;
    
    staged_files_.erase(
        std::remove_if(staged_files_.begin(), staged_files_.end(),
                      [&path](const FileStatus& s) { return s.path == path; }),
        staged_files_.end()
    );
    
    return true;
}

bool GitIntegration::StageAll() {
    if (!is_repo_open_) return false;
    
    // In real implementation, would stage all modified files
    return true;
}

bool GitIntegration::Commit(const std::string& message, const std::string& author) {
    if (!is_repo_open_ || staged_files_.empty()) return false;
    
    CommitInfo commit;
    commit.hash = "abc123def456";  // Simulated hash
    commit.author = author;
    commit.message = message;
    commit.timestamp = std::chrono::system_clock::now();
    
    for (const auto& file : staged_files_) {
        commit.files.push_back(file.path);
    }
    
    commit_history_.push_back(commit);
    staged_files_.clear();
    
    return true;
}

std::vector<GitIntegration::CommitInfo> GitIntegration::GetCommitHistory(int max_count) {
    std::vector<CommitInfo> result;
    int count = 0;
    
    for (auto it = commit_history_.rbegin(); 
         it != commit_history_.rend() && count < max_count; 
         ++it, ++count) {
        result.push_back(*it);
    }
    
    return result;
}

GitIntegration::CommitInfo GitIntegration::GetCommitInfo(const std::string& hash) {
    for (const auto& commit : commit_history_) {
        if (commit.hash == hash) {
            return commit;
        }
    }
    return CommitInfo();
}

std::vector<GitIntegration::BranchInfo> GitIntegration::GetBranches() {
    if (!is_repo_open_) return {};
    
    // Simulated branch list
    std::vector<BranchInfo> branches;
    
    BranchInfo main;
    main.name = "main";
    main.is_current = (current_branch_ == "main");
    main.last_commit = commit_history_.empty() ? "" : commit_history_.back().hash;
    branches.push_back(main);
    
    return branches;
}

bool GitIntegration::CreateBranch(const std::string& name) {
    if (!is_repo_open_) return false;
    // Simulated branch creation
    return true;
}

bool GitIntegration::SwitchBranch(const std::string& name) {
    if (!is_repo_open_) return false;
    current_branch_ = name;
    return true;
}

bool GitIntegration::DeleteBranch(const std::string& name) {
    if (!is_repo_open_ || name == current_branch_) return false;
    // Simulated branch deletion
    return true;
}

std::string GitIntegration::GetCurrentBranch() {
    return current_branch_;
}

std::string GitIntegration::GetDiff(const std::string& file_path) {
    if (!is_repo_open_) return "";
    
    // Simulated diff output
    return "diff --git a/" + file_path + " b/" + file_path + "\n"
           "--- a/" + file_path + "\n"
           "+++ b/" + file_path + "\n"
           "@@ -1,3 +1,4 @@\n"
           " // Existing code\n"
           "+// New line added\n"
           " void setup() {\n";
}

std::string GitIntegration::GetDiffBetweenCommits(const std::string& commit1, 
                                                   const std::string& commit2) {
    if (!is_repo_open_) return "";
    // Simulated diff between commits
    return "Diff between " + commit1 + " and " + commit2;
}

bool GitIntegration::AddRemote(const std::string& name, const std::string& url) {
    if (!is_repo_open_) return false;
    // Simulated remote addition
    return true;
}

bool GitIntegration::RemoveRemote(const std::string& name) {
    if (!is_repo_open_) return false;
    // Simulated remote removal
    return true;
}

bool GitIntegration::Push(const std::string& remote, const std::string& branch) {
    if (!is_repo_open_) return false;
    // Simulated push operation
    return true;
}

bool GitIntegration::Pull(const std::string& remote, const std::string& branch) {
    if (!is_repo_open_) return false;
    // Simulated pull operation
    return true;
}

bool GitIntegration::Fetch(const std::string& remote) {
    if (!is_repo_open_) return false;
    // Simulated fetch operation
    return true;
}

// ============================================================================
// CodeReviewSystem Implementation (Version 1.3.0)
// ============================================================================

CodeReviewSystem::CodeReviewSystem() : next_review_id_(1), next_comment_id_(1) {}

std::string CodeReviewSystem::CreateReview(const std::string& title, 
                                           const std::string& description,
                                           const std::string& author) {
    std::string id = GenerateId("review");
    
    CodeReview review;
    review.id = id;
    review.title = title;
    review.description = description;
    review.author = author;
    review.status = ReviewStatus::PENDING;
    review.created_at = std::chrono::system_clock::now();
    review.updated_at = review.created_at;
    
    reviews_[id] = review;
    return id;
}

bool CodeReviewSystem::AddReviewer(const std::string& review_id, const std::string& reviewer) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return false;
    
    it->second.reviewers.push_back(reviewer);
    it->second.updated_at = std::chrono::system_clock::now();
    return true;
}

bool CodeReviewSystem::UpdateReviewStatus(const std::string& review_id, ReviewStatus status) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return false;
    
    it->second.status = status;
    it->second.updated_at = std::chrono::system_clock::now();
    return true;
}

CodeReviewSystem::CodeReview CodeReviewSystem::GetReview(const std::string& review_id) {
    auto it = reviews_.find(review_id);
    if (it != reviews_.end()) {
        return it->second;
    }
    return CodeReview();
}

std::vector<CodeReviewSystem::CodeReview> CodeReviewSystem::GetAllReviews() {
    std::vector<CodeReview> result;
    for (const auto& pair : reviews_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<CodeReviewSystem::CodeReview> CodeReviewSystem::GetReviewsByAuthor(
    const std::string& author) {
    std::vector<CodeReview> result;
    for (const auto& pair : reviews_) {
        if (pair.second.author == author) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::string CodeReviewSystem::AddComment(const std::string& review_id, 
                                        const std::string& author,
                                        const std::string& file_path, 
                                        int line_number,
                                        CommentType type, 
                                        const std::string& content) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return "";
    
    std::string comment_id = GenerateId("comment");
    
    ReviewComment comment;
    comment.id = comment_id;
    comment.author = author;
    comment.file_path = file_path;
    comment.line_number = line_number;
    comment.type = type;
    comment.content = content;
    comment.timestamp = std::chrono::system_clock::now();
    
    comments_[comment_id] = comment;
    it->second.comments.push_back(comment_id);
    it->second.updated_at = std::chrono::system_clock::now();
    
    return comment_id;
}

bool CodeReviewSystem::AddReply(const std::string& comment_id, const std::string& reply) {
    auto it = comments_.find(comment_id);
    if (it == comments_.end()) return false;
    
    it->second.replies.push_back(reply);
    return true;
}

CodeReviewSystem::ReviewComment CodeReviewSystem::GetComment(const std::string& comment_id) {
    auto it = comments_.find(comment_id);
    if (it != comments_.end()) {
        return it->second;
    }
    return ReviewComment();
}

std::vector<CodeReviewSystem::ReviewComment> CodeReviewSystem::GetCommentsForReview(
    const std::string& review_id) {
    std::vector<ReviewComment> result;
    
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return result;
    
    for (const auto& comment_id : it->second.comments) {
        auto comment_it = comments_.find(comment_id);
        if (comment_it != comments_.end()) {
            result.push_back(comment_it->second);
        }
    }
    
    return result;
}

std::vector<CodeReviewSystem::ReviewComment> CodeReviewSystem::GetCommentsForFile(
    const std::string& review_id, const std::string& file_path) {
    std::vector<ReviewComment> all_comments = GetCommentsForReview(review_id);
    std::vector<ReviewComment> result;
    
    for (const auto& comment : all_comments) {
        if (comment.file_path == file_path) {
            result.push_back(comment);
        }
    }
    
    return result;
}

bool CodeReviewSystem::ResolveComment(const std::string& comment_id) {
    auto it = comments_.find(comment_id);
    if (it == comments_.end()) return false;
    
    // Mark comment as resolved (could add a 'resolved' field to ReviewComment)
    return true;
}

void CodeReviewSystem::TrackChange(const std::string& file_path, int line_number,
                                   const std::string& change_type, 
                                   const std::string& old_content,
                                   const std::string& new_content, 
                                   const std::string& author) {
    ChangeTracker change;
    change.file_path = file_path;
    change.line_number = line_number;
    change.change_type = change_type;
    change.old_content = old_content;
    change.new_content = new_content;
    change.author = author;
    change.timestamp = std::chrono::system_clock::now();
    
    change_history_.push_back(change);
}

std::vector<CodeReviewSystem::ChangeTracker> CodeReviewSystem::GetChanges(
    const std::string& file_path) {
    std::vector<ChangeTracker> result;
    
    for (const auto& change : change_history_) {
        if (change.file_path == file_path) {
            result.push_back(change);
        }
    }
    
    return result;
}

std::vector<CodeReviewSystem::ChangeTracker> CodeReviewSystem::GetRecentChanges(int max_count) {
    std::vector<ChangeTracker> result;
    
    int start = std::max(0, static_cast<int>(change_history_.size()) - max_count);
    for (int i = start; i < static_cast<int>(change_history_.size()); ++i) {
        result.push_back(change_history_[i]);
    }
    
    return result;
}

void CodeReviewSystem::ClearChangeHistory() {
    change_history_.clear();
}

bool CodeReviewSystem::StartReview(const std::string& review_id) {
    return UpdateReviewStatus(review_id, ReviewStatus::PENDING);
}

bool CodeReviewSystem::ApproveReview(const std::string& review_id, const std::string& reviewer) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return false;
    
    // Check if reviewer is in the reviewers list
    auto& reviewers = it->second.reviewers;
    if (std::find(reviewers.begin(), reviewers.end(), reviewer) == reviewers.end()) {
        return false;
    }
    
    return UpdateReviewStatus(review_id, ReviewStatus::APPROVED);
}

bool CodeReviewSystem::RequestChanges(const std::string& review_id, 
                                      const std::string& reviewer,
                                      const std::string& reason) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return false;
    
    // Add a comment with the reason
    AddComment(review_id, reviewer, "", 0, CommentType::ISSUE, reason);
    
    return UpdateReviewStatus(review_id, ReviewStatus::CHANGES_REQUESTED);
}

std::string CodeReviewSystem::GenerateReviewReport(const std::string& review_id) {
    auto it = reviews_.find(review_id);
    if (it == reviews_.end()) return "";
    
    const auto& review = it->second;
    std::ostringstream report;
    
    report << "====================================\n";
    report << "Code Review Report\n";
    report << "====================================\n\n";
    
    report << "Title: " << review.title << "\n";
    report << "Author: " << review.author << "\n";
    report << "Status: ";
    
    switch (review.status) {
        case ReviewStatus::PENDING: report << "Pending\n"; break;
        case ReviewStatus::APPROVED: report << "Approved\n"; break;
        case ReviewStatus::CHANGES_REQUESTED: report << "Changes Requested\n"; break;
        case ReviewStatus::COMMENTED: report << "Commented\n"; break;
    }
    
    report << "\nReviewers: ";
    for (size_t i = 0; i < review.reviewers.size(); ++i) {
        if (i > 0) report << ", ";
        report << review.reviewers[i];
    }
    report << "\n\n";
    
    report << "Comments: " << review.comments.size() << "\n";
    report << "Changed Files: " << review.changed_files.size() << "\n\n";
    
    if (!review.comments.empty()) {
        report << "Comments:\n";
        report << "---------\n";
        
        for (const auto& comment_id : review.comments) {
            auto comment_it = comments_.find(comment_id);
            if (comment_it != comments_.end()) {
                const auto& comment = comment_it->second;
                report << "\n[";
                switch (comment.type) {
                    case CommentType::GENERAL: report << "General"; break;
                    case CommentType::SUGGESTION: report << "Suggestion"; break;
                    case CommentType::ISSUE: report << "Issue"; break;
                    case CommentType::QUESTION: report << "Question"; break;
                }
                report << "] " << comment.author << " - ";
                
                if (!comment.file_path.empty()) {
                    report << comment.file_path << ":" << comment.line_number << "\n";
                } else {
                    report << "General comment\n";
                }
                
                report << "  " << comment.content << "\n";
                
                if (!comment.replies.empty()) {
                    report << "  Replies: " << comment.replies.size() << "\n";
                }
            }
        }
    }
    
    return report.str();
}

std::string CodeReviewSystem::GenerateId(const std::string& prefix) {
    if (prefix == "review") {
        return prefix + "_" + std::to_string(next_review_id_++);
    } else if (prefix == "comment") {
        return prefix + "_" + std::to_string(next_comment_id_++);
    }
    return "";
}

} // namespace collaboration
} // namespace esp32_ide
