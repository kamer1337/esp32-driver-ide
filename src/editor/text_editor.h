#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <string>
#include <vector>
#include <functional>

namespace esp32_ide {

/**
 * @brief Text editor component for managing code content
 * 
 * Provides text editing functionality with undo/redo,
 * cursor management, and content manipulation.
 */
class TextEditor {
public:
    TextEditor();
    ~TextEditor();

    // Content management
    void SetText(const std::string& text);
    std::string GetText() const;
    void InsertText(const std::string& text, size_t position);
    void DeleteText(size_t start, size_t length);
    void Clear();

    // Cursor operations
    void SetCursorPosition(size_t position);
    size_t GetCursorPosition() const;
    
    // Line operations
    size_t GetLineCount() const;
    std::string GetLine(size_t line_number) const;
    size_t GetCurrentLine() const;
    
    // Selection
    void SetSelection(size_t start, size_t end);
    std::string GetSelectedText() const;
    bool HasSelection() const;
    
    // Undo/Redo
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    
    // Change notifications
    using ChangeCallback = std::function<void()>;
    void SetChangeCallback(ChangeCallback callback);
    
    // Search
    std::vector<size_t> FindText(const std::string& query) const;
    bool ReplaceText(const std::string& search, const std::string& replace);
    
    // Breakpoint support
    void ToggleBreakpoint(size_t line_number);
    void AddBreakpoint(size_t line_number);
    void RemoveBreakpoint(size_t line_number);
    void ClearAllBreakpoints();
    std::vector<size_t> GetBreakpoints() const;
    bool HasBreakpoint(size_t line_number) const;
    
    // Code completion
    struct CompletionItem {
        std::string text;
        std::string description;
        std::string insert_text;
        int priority;
    };
    std::vector<CompletionItem> GetCompletionsAtCursor() const;
    
    // Tab groups and split views
    enum class SplitOrientation {
        NONE,
        HORIZONTAL,
        VERTICAL
    };
    
    struct EditorTab {
        std::string filename;
        std::string content;
        size_t cursor_position;
        bool is_modified;
        int group_id;
    };
    
    struct TabGroup {
        int id;
        std::vector<int> tab_indices;
        int active_tab_index;
    };
    
    // Tab management
    int CreateTab(const std::string& filename = "untitled.ino");
    bool CloseTab(int tab_id);
    bool SwitchToTab(int tab_id);
    int GetActiveTabId() const { return active_tab_id_; }
    EditorTab* GetTab(int tab_id);
    const EditorTab* GetTab(int tab_id) const;
    std::vector<EditorTab> GetAllTabs() const;
    
    // Tab groups
    int CreateTabGroup();
    bool MoveTabToGroup(int tab_id, int group_id);
    bool CloseTabGroup(int group_id);
    TabGroup* GetTabGroup(int group_id);
    std::vector<TabGroup> GetAllTabGroups() const;
    
    // Split view
    void SetSplitOrientation(SplitOrientation orientation) { split_orientation_ = orientation; }
    SplitOrientation GetSplitOrientation() const { return split_orientation_; }
    
private:
    // Configuration constants
    static constexpr size_t MAX_CONTENT_SIZE = 10 * 1024 * 1024; // 10MB limit
    static constexpr size_t MAX_UNDO_STACK_SIZE = 100;
    
    struct EditorState {
        std::string content;
        size_t cursor_position;
        size_t selection_start;
        size_t selection_end;
    };
    
    EditorState current_state_;
    std::vector<EditorState> undo_stack_;
    std::vector<EditorState> redo_stack_;
    ChangeCallback change_callback_;
    std::vector<size_t> breakpoints_;
    
    // Tab and group management
    std::vector<EditorTab> tabs_;
    std::vector<TabGroup> tab_groups_;
    int active_tab_id_;
    int next_tab_id_;
    int next_group_id_;
    SplitOrientation split_orientation_;
    
    void SaveState();
    void NotifyChange();
};

} // namespace esp32_ide

#endif // TEXT_EDITOR_H
