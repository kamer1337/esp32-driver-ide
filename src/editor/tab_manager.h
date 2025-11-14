#ifndef TAB_MANAGER_H
#define TAB_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace esp32_ide {

// Forward declaration
class TextEditor;

/**
 * @brief Tab representing an open file in the editor
 */
class EditorTab {
public:
    EditorTab(const std::string& file_path, TextEditor* editor);
    ~EditorTab() = default;

    // Getters
    const std::string& GetFilePath() const { return file_path_; }
    const std::string& GetTitle() const { return title_; }
    TextEditor* GetEditor() const { return editor_; }
    bool IsModified() const { return is_modified_; }
    bool IsActive() const { return is_active_; }
    int GetCursorPosition() const { return cursor_position_; }
    int GetScrollPosition() const { return scroll_position_; }

    // Setters
    void SetFilePath(const std::string& path);
    void SetTitle(const std::string& title) { title_ = title; }
    void SetModified(bool modified) { is_modified_ = modified; }
    void SetActive(bool active) { is_active_ = active; }
    void SetCursorPosition(int position) { cursor_position_ = position; }
    void SetScrollPosition(int position) { scroll_position_ = position; }

    // Content operations
    void SaveState();
    void RestoreState();

private:
    std::string file_path_;
    std::string title_;
    TextEditor* editor_;
    bool is_modified_;
    bool is_active_;
    int cursor_position_;
    int scroll_position_;
};

/**
 * @brief Split view pane containing tabs
 */
class SplitPane {
public:
    enum class Orientation {
        HORIZONTAL,
        VERTICAL
    };

    SplitPane(int id);
    ~SplitPane() = default;

    // Getters
    int GetId() const { return id_; }
    const std::vector<std::unique_ptr<EditorTab>>& GetTabs() const { return tabs_; }
    int GetActiveTabIndex() const { return active_tab_index_; }
    EditorTab* GetActiveTab() const;
    size_t GetTabCount() const { return tabs_.size(); }
    bool IsActive() const { return is_active_; }

    // Tab management
    EditorTab* AddTab(const std::string& file_path, TextEditor* editor);
    bool RemoveTab(int index);
    bool RemoveTab(const std::string& file_path);
    void ActivateTab(int index);
    void ActivateTab(const std::string& file_path);
    EditorTab* FindTab(const std::string& file_path) const;
    int FindTabIndex(const std::string& file_path) const;

    // Navigation
    void NextTab();
    void PreviousTab();
    void MoveTab(int from_index, int to_index);

    // Pane state
    void SetActive(bool active) { is_active_ = active; }
    void SetSize(int width, int height) { width_ = width; height_ = height; }
    void GetSize(int& width, int& height) const { width = width_; height = height_; }

private:
    int id_;
    std::vector<std::unique_ptr<EditorTab>> tabs_;
    int active_tab_index_;
    bool is_active_;
    int width_;
    int height_;
};

/**
 * @brief Tab manager with split view support
 */
class TabManager {
public:
    TabManager();
    ~TabManager() = default;

    // Initialization
    bool Initialize();
    void Shutdown();

    // Tab operations
    EditorTab* OpenFile(const std::string& file_path, TextEditor* editor);
    bool CloseFile(const std::string& file_path);
    bool CloseTab(int pane_id, int tab_index);
    void CloseAllTabs();
    void CloseAllTabsExcept(const std::string& file_path);

    // Tab navigation
    void NextTab();
    void PreviousTab();
    void ActivateTab(int pane_id, int tab_index);
    void ActivateTab(const std::string& file_path);

    // Tab queries
    EditorTab* GetActiveTab() const;
    EditorTab* FindTab(const std::string& file_path) const;
    bool HasTab(const std::string& file_path) const;
    std::vector<std::string> GetOpenFiles() const;
    int GetTabCount() const;

    // Split view operations
    int SplitHorizontal();
    int SplitVertical();
    bool CloseSplit(int pane_id);
    void CloseAllSplits();
    bool MoveToPaneTab(const std::string& file_path, int target_pane_id);

    // Pane management
    int GetActivePaneId() const { return active_pane_id_; }
    SplitPane* GetActivePane() const;
    SplitPane* GetPane(int pane_id) const;
    int GetPaneCount() const { return static_cast<int>(panes_.size()); }
    void NextPane();
    void PreviousPane();
    void ActivatePane(int pane_id);

    // Layout
    enum class LayoutMode {
        SINGLE,
        HORIZONTAL_SPLIT,
        VERTICAL_SPLIT,
        GRID_2X2,
        CUSTOM
    };
    
    void SetLayout(LayoutMode mode);
    LayoutMode GetLayout() const { return layout_mode_; }

    // Callbacks
    using TabChangeCallback = std::function<void(const std::string& file_path)>;
    using TabCloseCallback = std::function<void(const std::string& file_path)>;
    
    void SetTabChangeCallback(TabChangeCallback callback) { tab_change_callback_ = callback; }
    void SetTabCloseCallback(TabCloseCallback callback) { tab_close_callback_ = callback; }

    // Save/restore session
    bool SaveSession(const std::string& filename) const;
    bool LoadSession(const std::string& filename);

private:
    std::vector<std::unique_ptr<SplitPane>> panes_;
    int active_pane_id_;
    int next_pane_id_;
    LayoutMode layout_mode_;
    
    TabChangeCallback tab_change_callback_;
    TabCloseCallback tab_close_callback_;

    // Helper methods
    SplitPane* CreatePane();
    int FindPaneIndex(int pane_id) const;
    void NotifyTabChange(const std::string& file_path);
    void NotifyTabClose(const std::string& file_path);
};

} // namespace esp32_ide

#endif // TAB_MANAGER_H
