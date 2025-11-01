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
    
private:
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
    
    void SaveState();
    void NotifyChange();
};

} // namespace esp32_ide

#endif // TEXT_EDITOR_H
