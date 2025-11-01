#include "editor/text_editor.h"
#include <algorithm>
#include <sstream>

namespace esp32_ide {

TextEditor::TextEditor() {
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
    const size_t MAX_UNDO = 100;
    if (undo_stack_.size() > MAX_UNDO) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

void TextEditor::NotifyChange() {
    if (change_callback_) {
        change_callback_();
    }
}

} // namespace esp32_ide
