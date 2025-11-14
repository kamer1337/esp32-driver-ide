#include "editor/text_editor.h"
#include <algorithm>
#include <sstream>

namespace esp32_ide {

TextEditor::TextEditor() 
    : active_tab_id_(-1), next_tab_id_(0), next_group_id_(0), split_orientation_(SplitOrientation::NONE) {
    current_state_.content = "";
    current_state_.cursor_position = 0;
    current_state_.selection_start = 0;
    current_state_.selection_end = 0;
}

TextEditor::~TextEditor() = default;

void TextEditor::SetText(const std::string& text) {
    SaveState();
    current_state_.content = text;
    current_state_.cursor_position = text.length();
    current_state_.selection_start = 0;
    current_state_.selection_end = 0;
    NotifyChange();
}

std::string TextEditor::GetText() const {
    return current_state_.content;
}

void TextEditor::InsertText(const std::string& text, size_t position) {
    // Validate input to prevent excessive memory usage
    if (current_state_.content.length() + text.length() > MAX_CONTENT_SIZE) {
        // Log or notify about the error - in a real implementation, this would
        // trigger a user notification or error callback
        // For now, silently reject to maintain existing API compatibility
        return;
    }
    
    SaveState();
    if (position > current_state_.content.length()) {
        position = current_state_.content.length();
    }
    current_state_.content.insert(position, text);
    current_state_.cursor_position = position + text.length();
    NotifyChange();
}

void TextEditor::DeleteText(size_t start, size_t length) {
    SaveState();
    if (start < current_state_.content.length()) {
        current_state_.content.erase(start, length);
        current_state_.cursor_position = start;
    }
    NotifyChange();
}

void TextEditor::Clear() {
    SaveState();
    current_state_.content.clear();
    current_state_.cursor_position = 0;
    current_state_.selection_start = 0;
    current_state_.selection_end = 0;
    NotifyChange();
}

void TextEditor::SetCursorPosition(size_t position) {
    if (position <= current_state_.content.length()) {
        current_state_.cursor_position = position;
    }
}

size_t TextEditor::GetCursorPosition() const {
    return current_state_.cursor_position;
}

size_t TextEditor::GetLineCount() const {
    if (current_state_.content.empty()) return 1;
    return std::count(current_state_.content.begin(), current_state_.content.end(), '\n') + 1;
}

std::string TextEditor::GetLine(size_t line_number) const {
    std::istringstream stream(current_state_.content);
    std::string line;
    size_t current = 0;
    
    while (std::getline(stream, line)) {
        if (current == line_number) {
            return line;
        }
        current++;
    }
    return "";
}

size_t TextEditor::GetCurrentLine() const {
    return std::count(current_state_.content.begin(), 
                     current_state_.content.begin() + current_state_.cursor_position, '\n');
}

void TextEditor::SetSelection(size_t start, size_t end) {
    current_state_.selection_start = std::min(start, current_state_.content.length());
    current_state_.selection_end = std::min(end, current_state_.content.length());
}

std::string TextEditor::GetSelectedText() const {
    if (!HasSelection()) return "";
    size_t start = std::min(current_state_.selection_start, current_state_.selection_end);
    size_t end = std::max(current_state_.selection_start, current_state_.selection_end);
    return current_state_.content.substr(start, end - start);
}

bool TextEditor::HasSelection() const {
    return current_state_.selection_start != current_state_.selection_end;
}

void TextEditor::Undo() {
    if (CanUndo()) {
        redo_stack_.push_back(current_state_);
        current_state_ = undo_stack_.back();
        undo_stack_.pop_back();
        NotifyChange();
    }
}

void TextEditor::Redo() {
    if (CanRedo()) {
        undo_stack_.push_back(current_state_);
        current_state_ = redo_stack_.back();
        redo_stack_.pop_back();
        NotifyChange();
    }
}

bool TextEditor::CanUndo() const {
    return !undo_stack_.empty();
}

bool TextEditor::CanRedo() const {
    return !redo_stack_.empty();
}

void TextEditor::SetChangeCallback(ChangeCallback callback) {
    change_callback_ = callback;
}

std::vector<size_t> TextEditor::FindText(const std::string& query) const {
    std::vector<size_t> positions;
    if (query.empty()) return positions;
    
    size_t pos = 0;
    while ((pos = current_state_.content.find(query, pos)) != std::string::npos) {
        positions.push_back(pos);
        pos += query.length();
    }
    return positions;
}

bool TextEditor::ReplaceText(const std::string& search, const std::string& replace) {
    SaveState();
    size_t pos = current_state_.content.find(search);
    if (pos != std::string::npos) {
        current_state_.content.replace(pos, search.length(), replace);
        NotifyChange();
        return true;
    }
    return false;
}

void TextEditor::SaveState() {
    undo_stack_.push_back(current_state_);
    redo_stack_.clear();
    
    // Limit undo stack size
    if (undo_stack_.size() > MAX_UNDO_STACK_SIZE) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

void TextEditor::NotifyChange() {
    if (change_callback_) {
        change_callback_();
    }
}

// Breakpoint support implementation
void TextEditor::ToggleBreakpoint(size_t line_number) {
    if (HasBreakpoint(line_number)) {
        RemoveBreakpoint(line_number);
    } else {
        AddBreakpoint(line_number);
    }
}

void TextEditor::AddBreakpoint(size_t line_number) {
    if (!HasBreakpoint(line_number)) {
        breakpoints_.push_back(line_number);
        std::sort(breakpoints_.begin(), breakpoints_.end());
    }
}

void TextEditor::RemoveBreakpoint(size_t line_number) {
    breakpoints_.erase(
        std::remove(breakpoints_.begin(), breakpoints_.end(), line_number),
        breakpoints_.end()
    );
}

void TextEditor::ClearAllBreakpoints() {
    breakpoints_.clear();
}

std::vector<size_t> TextEditor::GetBreakpoints() const {
    return breakpoints_;
}

bool TextEditor::HasBreakpoint(size_t line_number) const {
    return std::find(breakpoints_.begin(), breakpoints_.end(), line_number) != breakpoints_.end();
}

// Code completion implementation
std::vector<TextEditor::CompletionItem> TextEditor::GetCompletionsAtCursor() const {
    std::vector<CompletionItem> completions;
    
    // Get current line text
    size_t current_line = GetCurrentLine();
    // Fix: Line numbers are 0-indexed, so line 0 is valid. Check should be >= not >
    if (current_line >= GetLineCount()) {
        return completions;
    }
    
    std::string line = GetLine(current_line);
    std::string lower_line = line;
    std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
    
    // Arduino/ESP32 common completions
    if (lower_line.find("pin") != std::string::npos || lower_line.find("gpio") != std::string::npos) {
        completions.push_back({"pinMode", "Configure pin mode", "pinMode(pin, OUTPUT);", 95});
        completions.push_back({"digitalWrite", "Write digital value", "digitalWrite(pin, HIGH);", 90});
        completions.push_back({"digitalRead", "Read digital value", "digitalRead(pin)", 90});
        completions.push_back({"analogRead", "Read analog value", "analogRead(pin)", 85});
    }
    
    if (lower_line.find("serial") != std::string::npos) {
        completions.push_back({"Serial.begin", "Initialize serial", "Serial.begin(115200);", 95});
        completions.push_back({"Serial.println", "Print with newline", "Serial.println();", 90});
        completions.push_back({"Serial.print", "Print without newline", "Serial.print();", 88});
    }
    
    if (lower_line.find("wifi") != std::string::npos) {
        completions.push_back({"WiFi.begin", "Connect to WiFi", "WiFi.begin(ssid, password);", 95});
        completions.push_back({"WiFi.status", "Get WiFi status", "WiFi.status()", 90});
        completions.push_back({"WiFi.localIP", "Get IP address", "WiFi.localIP()", 85});
    }
    
    if (lower_line.find("delay") != std::string::npos || lower_line.find("time") != std::string::npos) {
        completions.push_back({"delay", "Blocking delay", "delay(1000);", 90});
        completions.push_back({"millis", "Get milliseconds", "millis()", 95});
        completions.push_back({"micros", "Get microseconds", "micros()", 85});
    }
    
    // Sort by priority
    std::sort(completions.begin(), completions.end(),
              [](const CompletionItem& a, const CompletionItem& b) {
                  return a.priority > b.priority;
              });
    
    return completions;
}

// Tab management
int TextEditor::CreateTab(const std::string& filename) {
    EditorTab tab;
    tab.filename = filename;
    tab.content = "";
    tab.cursor_position = 0;
    tab.is_modified = false;
    tab.group_id = -1;
    
    int tab_id = next_tab_id_++;
    tabs_.push_back(tab);
    active_tab_id_ = tab_id;
    
    return tab_id;
}

bool TextEditor::CloseTab(int tab_id) {
    if (tab_id < 0 || tab_id >= static_cast<int>(tabs_.size())) {
        return false;
    }
    
    // Remove from any group
    for (auto& group : tab_groups_) {
        auto it = std::find(group.tab_indices.begin(), group.tab_indices.end(), tab_id);
        if (it != group.tab_indices.end()) {
            group.tab_indices.erase(it);
        }
    }
    
    // Don't actually remove to preserve indices, just mark as closed
    tabs_[tab_id].filename = "";
    
    // Switch to another tab if this was active
    if (active_tab_id_ == tab_id) {
        active_tab_id_ = -1;
        for (size_t i = 0; i < tabs_.size(); ++i) {
            if (!tabs_[i].filename.empty()) {
                active_tab_id_ = static_cast<int>(i);
                break;
            }
        }
    }
    
    return true;
}

bool TextEditor::SwitchToTab(int tab_id) {
    if (tab_id < 0 || tab_id >= static_cast<int>(tabs_.size())) {
        return false;
    }
    
    if (tabs_[tab_id].filename.empty()) {
        return false; // Tab is closed
    }
    
    active_tab_id_ = tab_id;
    
    // Update current state from tab
    current_state_.content = tabs_[tab_id].content;
    current_state_.cursor_position = tabs_[tab_id].cursor_position;
    
    return true;
}

TextEditor::EditorTab* TextEditor::GetTab(int tab_id) {
    if (tab_id < 0 || tab_id >= static_cast<int>(tabs_.size())) {
        return nullptr;
    }
    return &tabs_[tab_id];
}

const TextEditor::EditorTab* TextEditor::GetTab(int tab_id) const {
    if (tab_id < 0 || tab_id >= static_cast<int>(tabs_.size())) {
        return nullptr;
    }
    return &tabs_[tab_id];
}

std::vector<TextEditor::EditorTab> TextEditor::GetAllTabs() const {
    std::vector<EditorTab> result;
    for (const auto& tab : tabs_) {
        if (!tab.filename.empty()) {
            result.push_back(tab);
        }
    }
    return result;
}

// Tab groups
int TextEditor::CreateTabGroup() {
    TabGroup group;
    group.id = next_group_id_++;
    group.active_tab_index = -1;
    
    tab_groups_.push_back(group);
    return group.id;
}

bool TextEditor::MoveTabToGroup(int tab_id, int group_id) {
    if (tab_id < 0 || tab_id >= static_cast<int>(tabs_.size())) {
        return false;
    }
    
    // Find the group
    TabGroup* target_group = nullptr;
    for (auto& group : tab_groups_) {
        if (group.id == group_id) {
            target_group = &group;
            break;
        }
    }
    
    if (!target_group) {
        return false;
    }
    
    // Remove from current group
    for (auto& group : tab_groups_) {
        auto it = std::find(group.tab_indices.begin(), group.tab_indices.end(), tab_id);
        if (it != group.tab_indices.end()) {
            group.tab_indices.erase(it);
        }
    }
    
    // Add to new group
    target_group->tab_indices.push_back(tab_id);
    tabs_[tab_id].group_id = group_id;
    
    return true;
}

bool TextEditor::CloseTabGroup(int group_id) {
    auto it = std::find_if(tab_groups_.begin(), tab_groups_.end(),
                          [group_id](const TabGroup& g) { return g.id == group_id; });
    
    if (it == tab_groups_.end()) {
        return false;
    }
    
    // Close all tabs in the group
    for (int tab_id : it->tab_indices) {
        CloseTab(tab_id);
    }
    
    tab_groups_.erase(it);
    return true;
}

TextEditor::TabGroup* TextEditor::GetTabGroup(int group_id) {
    for (auto& group : tab_groups_) {
        if (group.id == group_id) {
            return &group;
        }
    }
    return nullptr;
}

std::vector<TextEditor::TabGroup> TextEditor::GetAllTabGroups() const {
    return tab_groups_;
}

} // namespace esp32_ide
