#include "editor/tab_manager.h"
#include "editor/text_editor.h"
#include <algorithm>
#include <fstream>

namespace esp32_ide {

// EditorTab implementation

EditorTab::EditorTab(const std::string& file_path, TextEditor* editor)
    : file_path_(file_path), editor_(editor), is_modified_(false), 
      is_active_(false), cursor_position_(0), scroll_position_(0) {
    
    // Extract filename from path for title
    size_t last_slash = file_path.find_last_of("/\\");
    title_ = (last_slash != std::string::npos) ? file_path.substr(last_slash + 1) : file_path;
}

void EditorTab::SetFilePath(const std::string& path) {
    file_path_ = path;
    size_t last_slash = path.find_last_of("/\\");
    title_ = (last_slash != std::string::npos) ? path.substr(last_slash + 1) : path;
}

void EditorTab::SaveState() {
    if (editor_) {
        cursor_position_ = editor_->GetCursorPosition();
        // scroll_position would be saved here if we tracked it
    }
}

void EditorTab::RestoreState() {
    if (editor_) {
        editor_->SetCursorPosition(cursor_position_);
        // scroll_position would be restored here
    }
}

// SplitPane implementation

SplitPane::SplitPane(int id)
    : id_(id), active_tab_index_(-1), is_active_(false), width_(800), height_(600) {
}

EditorTab* SplitPane::GetActiveTab() const {
    if (active_tab_index_ >= 0 && active_tab_index_ < static_cast<int>(tabs_.size())) {
        return tabs_[active_tab_index_].get();
    }
    return nullptr;
}

EditorTab* SplitPane::AddTab(const std::string& file_path, TextEditor* editor) {
    // Check if tab already exists
    for (const auto& tab : tabs_) {
        if (tab->GetFilePath() == file_path) {
            return tab.get();
        }
    }

    auto tab = std::make_unique<EditorTab>(file_path, editor);
    EditorTab* ptr = tab.get();
    tabs_.push_back(std::move(tab));
    
    // Activate the new tab
    active_tab_index_ = static_cast<int>(tabs_.size()) - 1;
    ptr->SetActive(true);
    
    return ptr;
}

bool SplitPane::RemoveTab(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size())) {
        return false;
    }

    tabs_.erase(tabs_.begin() + index);
    
    // Adjust active tab index
    if (tabs_.empty()) {
        active_tab_index_ = -1;
    } else if (active_tab_index_ >= static_cast<int>(tabs_.size())) {
        active_tab_index_ = static_cast<int>(tabs_.size()) - 1;
    }
    
    return true;
}

bool SplitPane::RemoveTab(const std::string& file_path) {
    int index = FindTabIndex(file_path);
    if (index >= 0) {
        return RemoveTab(index);
    }
    return false;
}

void SplitPane::ActivateTab(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size())) {
        return;
    }

    // Deactivate current tab
    if (active_tab_index_ >= 0 && active_tab_index_ < static_cast<int>(tabs_.size())) {
        tabs_[active_tab_index_]->SetActive(false);
        tabs_[active_tab_index_]->SaveState();
    }

    // Activate new tab
    active_tab_index_ = index;
    tabs_[index]->SetActive(true);
    tabs_[index]->RestoreState();
}

void SplitPane::ActivateTab(const std::string& file_path) {
    int index = FindTabIndex(file_path);
    if (index >= 0) {
        ActivateTab(index);
    }
}

EditorTab* SplitPane::FindTab(const std::string& file_path) const {
    for (const auto& tab : tabs_) {
        if (tab->GetFilePath() == file_path) {
            return tab.get();
        }
    }
    return nullptr;
}

int SplitPane::FindTabIndex(const std::string& file_path) const {
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i]->GetFilePath() == file_path) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void SplitPane::NextTab() {
    if (tabs_.empty()) return;
    int next = (active_tab_index_ + 1) % static_cast<int>(tabs_.size());
    ActivateTab(next);
}

void SplitPane::PreviousTab() {
    if (tabs_.empty()) return;
    int prev = (active_tab_index_ - 1 + static_cast<int>(tabs_.size())) % static_cast<int>(tabs_.size());
    ActivateTab(prev);
}

void SplitPane::MoveTab(int from_index, int to_index) {
    if (from_index < 0 || from_index >= static_cast<int>(tabs_.size()) ||
        to_index < 0 || to_index >= static_cast<int>(tabs_.size())) {
        return;
    }

    if (from_index == to_index) return;

    auto tab = std::move(tabs_[from_index]);
    tabs_.erase(tabs_.begin() + from_index);
    tabs_.insert(tabs_.begin() + to_index, std::move(tab));

    // Adjust active tab index
    if (active_tab_index_ == from_index) {
        active_tab_index_ = to_index;
    } else if (from_index < active_tab_index_ && to_index >= active_tab_index_) {
        active_tab_index_--;
    } else if (from_index > active_tab_index_ && to_index <= active_tab_index_) {
        active_tab_index_++;
    }
}

// TabManager implementation

TabManager::TabManager()
    : active_pane_id_(0), next_pane_id_(0), layout_mode_(LayoutMode::SINGLE) {
}

bool TabManager::Initialize() {
    // Create initial pane
    CreatePane();
    return true;
}

void TabManager::Shutdown() {
    panes_.clear();
}

EditorTab* TabManager::OpenFile(const std::string& file_path, TextEditor* editor) {
    // Check if file is already open in any pane
    EditorTab* existing = FindTab(file_path);
    if (existing) {
        ActivateTab(file_path);
        return existing;
    }

    // Add to active pane
    SplitPane* pane = GetActivePane();
    if (!pane) {
        CreatePane();
        pane = GetActivePane();
    }

    EditorTab* tab = pane->AddTab(file_path, editor);
    NotifyTabChange(file_path);
    return tab;
}

bool TabManager::CloseFile(const std::string& file_path) {
    for (auto& pane : panes_) {
        if (pane->RemoveTab(file_path)) {
            NotifyTabClose(file_path);
            return true;
        }
    }
    return false;
}

bool TabManager::CloseTab(int pane_id, int tab_index) {
    SplitPane* pane = GetPane(pane_id);
    if (pane) {
        EditorTab* tab = pane->GetTabs()[tab_index].get();
        std::string file_path = tab->GetFilePath();
        if (pane->RemoveTab(tab_index)) {
            NotifyTabClose(file_path);
            return true;
        }
    }
    return false;
}

void TabManager::CloseAllTabs() {
    for (auto& pane : panes_) {
        while (pane->GetTabCount() > 0) {
            EditorTab* tab = pane->GetTabs()[0].get();
            std::string file_path = tab->GetFilePath();
            pane->RemoveTab(0);
            NotifyTabClose(file_path);
        }
    }
}

void TabManager::CloseAllTabsExcept(const std::string& file_path) {
    for (auto& pane : panes_) {
        std::vector<std::string> to_close;
        for (const auto& tab : pane->GetTabs()) {
            if (tab->GetFilePath() != file_path) {
                to_close.push_back(tab->GetFilePath());
            }
        }
        for (const auto& path : to_close) {
            pane->RemoveTab(path);
            NotifyTabClose(path);
        }
    }
}

void TabManager::NextTab() {
    SplitPane* pane = GetActivePane();
    if (pane) {
        pane->NextTab();
        EditorTab* tab = pane->GetActiveTab();
        if (tab) {
            NotifyTabChange(tab->GetFilePath());
        }
    }
}

void TabManager::PreviousTab() {
    SplitPane* pane = GetActivePane();
    if (pane) {
        pane->PreviousTab();
        EditorTab* tab = pane->GetActiveTab();
        if (tab) {
            NotifyTabChange(tab->GetFilePath());
        }
    }
}

void TabManager::ActivateTab(int pane_id, int tab_index) {
    SplitPane* pane = GetPane(pane_id);
    if (pane) {
        pane->ActivateTab(tab_index);
        EditorTab* tab = pane->GetActiveTab();
        if (tab) {
            active_pane_id_ = pane_id;
            NotifyTabChange(tab->GetFilePath());
        }
    }
}

void TabManager::ActivateTab(const std::string& file_path) {
    for (auto& pane : panes_) {
        int index = pane->FindTabIndex(file_path);
        if (index >= 0) {
            active_pane_id_ = pane->GetId();
            pane->ActivateTab(index);
            NotifyTabChange(file_path);
            return;
        }
    }
}

EditorTab* TabManager::GetActiveTab() const {
    SplitPane* pane = GetActivePane();
    return pane ? pane->GetActiveTab() : nullptr;
}

EditorTab* TabManager::FindTab(const std::string& file_path) const {
    for (const auto& pane : panes_) {
        EditorTab* tab = pane->FindTab(file_path);
        if (tab) return tab;
    }
    return nullptr;
}

bool TabManager::HasTab(const std::string& file_path) const {
    return FindTab(file_path) != nullptr;
}

std::vector<std::string> TabManager::GetOpenFiles() const {
    std::vector<std::string> files;
    for (const auto& pane : panes_) {
        for (const auto& tab : pane->GetTabs()) {
            files.push_back(tab->GetFilePath());
        }
    }
    return files;
}

int TabManager::GetTabCount() const {
    int count = 0;
    for (const auto& pane : panes_) {
        count += static_cast<int>(pane->GetTabCount());
    }
    return count;
}

int TabManager::SplitHorizontal() {
    SplitPane* new_pane = CreatePane();
    layout_mode_ = LayoutMode::HORIZONTAL_SPLIT;
    return new_pane->GetId();
}

int TabManager::SplitVertical() {
    SplitPane* new_pane = CreatePane();
    layout_mode_ = LayoutMode::VERTICAL_SPLIT;
    return new_pane->GetId();
}

bool TabManager::CloseSplit(int pane_id) {
    if (panes_.size() <= 1) return false;

    int index = FindPaneIndex(pane_id);
    if (index < 0) return false;

    panes_.erase(panes_.begin() + index);
    
    if (active_pane_id_ == pane_id && !panes_.empty()) {
        active_pane_id_ = panes_[0]->GetId();
    }

    if (panes_.size() == 1) {
        layout_mode_ = LayoutMode::SINGLE;
    }

    return true;
}

void TabManager::CloseAllSplits() {
    while (panes_.size() > 1) {
        panes_.pop_back();
    }
    if (!panes_.empty()) {
        active_pane_id_ = panes_[0]->GetId();
    }
    layout_mode_ = LayoutMode::SINGLE;
}

bool TabManager::MoveToPaneTab(const std::string& file_path, int target_pane_id) {
    // Find the tab
    SplitPane* source_pane = nullptr;
    EditorTab* tab = nullptr;
    
    for (auto& pane : panes_) {
        tab = pane->FindTab(file_path);
        if (tab) {
            source_pane = pane.get();
            break;
        }
    }
    
    if (!tab || !source_pane) return false;
    
    SplitPane* target_pane = GetPane(target_pane_id);
    if (!target_pane || source_pane == target_pane) return false;

    // Create new tab in target pane
    target_pane->AddTab(file_path, tab->GetEditor());
    
    // Remove from source pane
    source_pane->RemoveTab(file_path);
    
    return true;
}

SplitPane* TabManager::GetActivePane() const {
    return GetPane(active_pane_id_);
}

SplitPane* TabManager::GetPane(int pane_id) const {
    for (const auto& pane : panes_) {
        if (pane->GetId() == pane_id) {
            return pane.get();
        }
    }
    return nullptr;
}

void TabManager::NextPane() {
    if (panes_.size() <= 1) return;
    
    int current_index = FindPaneIndex(active_pane_id_);
    int next_index = (current_index + 1) % static_cast<int>(panes_.size());
    active_pane_id_ = panes_[next_index]->GetId();
}

void TabManager::PreviousPane() {
    if (panes_.size() <= 1) return;
    
    int current_index = FindPaneIndex(active_pane_id_);
    int prev_index = (current_index - 1 + static_cast<int>(panes_.size())) % static_cast<int>(panes_.size());
    active_pane_id_ = panes_[prev_index]->GetId();
}

void TabManager::ActivatePane(int pane_id) {
    if (GetPane(pane_id)) {
        active_pane_id_ = pane_id;
    }
}

void TabManager::SetLayout(LayoutMode mode) {
    layout_mode_ = mode;
    
    switch (mode) {
        case LayoutMode::SINGLE:
            while (panes_.size() > 1) {
                panes_.pop_back();
            }
            break;
        case LayoutMode::HORIZONTAL_SPLIT:
        case LayoutMode::VERTICAL_SPLIT:
            while (panes_.size() > 2) {
                panes_.pop_back();
            }
            if (panes_.size() < 2) {
                CreatePane();
            }
            break;
        case LayoutMode::GRID_2X2:
            while (panes_.size() > 4) {
                panes_.pop_back();
            }
            while (panes_.size() < 4) {
                CreatePane();
            }
            break;
        case LayoutMode::CUSTOM:
            // Don't change pane count
            break;
    }
}

bool TabManager::SaveSession(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    // Save layout mode
    file << static_cast<int>(layout_mode_) << "\n";
    
    // Save panes and tabs
    file << panes_.size() << "\n";
    for (const auto& pane : panes_) {
        file << pane->GetId() << " " << pane->GetTabCount() << "\n";
        for (const auto& tab : pane->GetTabs()) {
            file << tab->GetFilePath() << "\n";
        }
    }
    
    return true;
}

bool TabManager::LoadSession(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    // Load layout mode
    int layout;
    file >> layout;
    layout_mode_ = static_cast<LayoutMode>(layout);
    
    // Clear existing panes
    panes_.clear();
    
    // Load panes and tabs
    size_t pane_count;
    file >> pane_count;
    
    for (size_t i = 0; i < pane_count; ++i) {
        int pane_id;
        size_t tab_count;
        file >> pane_id >> tab_count;
        file.ignore(); // Skip newline
        
        // Note: Can't fully restore tabs without TextEditor instances
        // This is a simplified version
    }
    
    return true;
}

// Private helper methods

SplitPane* TabManager::CreatePane() {
    auto pane = std::make_unique<SplitPane>(next_pane_id_++);
    SplitPane* ptr = pane.get();
    panes_.push_back(std::move(pane));
    
    if (panes_.size() == 1) {
        active_pane_id_ = ptr->GetId();
        ptr->SetActive(true);
    }
    
    return ptr;
}

int TabManager::FindPaneIndex(int pane_id) const {
    for (size_t i = 0; i < panes_.size(); ++i) {
        if (panes_[i]->GetId() == pane_id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void TabManager::NotifyTabChange(const std::string& file_path) {
    if (tab_change_callback_) {
        tab_change_callback_(file_path);
    }
}

void TabManager::NotifyTabClose(const std::string& file_path) {
    if (tab_close_callback_) {
        tab_close_callback_(file_path);
    }
}

} // namespace esp32_ide
