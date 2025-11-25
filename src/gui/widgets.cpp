#include "gui/widgets.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace esp32_ide {
namespace gui {

// =============================================================================
// Widget Base Class
// =============================================================================

Widget::Widget(const std::string& id)
    : id_(id),
      text_(""),
      x_(0), y_(0),
      width_(100), height_(30),
      enabled_(true),
      visible_(true),
      focused_(false),
      tooltip_("") {
}

// =============================================================================
// Button Widget
// =============================================================================

Button::Button(const std::string& id, const std::string& label)
    : Widget(id),
      icon_(""),
      on_click_(nullptr) {
    text_ = label;
}

void Button::Click() {
    if (enabled_ && on_click_) {
        on_click_();
    }
}

std::string Button::Render() const {
    std::ostringstream oss;
    oss << "[";
    if (!icon_.empty()) {
        oss << icon_ << " ";
    }
    oss << text_ << "]";
    if (!enabled_) {
        oss << " (disabled)";
    }
    return oss.str();
}

bool Button::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    if (x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_) {
        Click();
        return true;
    }
    return false;
}

// =============================================================================
// Dropdown Widget
// =============================================================================

Dropdown::Dropdown(const std::string& id)
    : Widget(id),
      selected_index_(-1),
      is_open_(false),
      on_select_(nullptr) {
}

void Dropdown::AddItem(const std::string& item) {
    items_.push_back(item);
    if (selected_index_ < 0 && !items_.empty()) {
        selected_index_ = 0;
    }
}

void Dropdown::RemoveItem(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        items_.erase(items_.begin() + index);
        if (selected_index_ >= static_cast<int>(items_.size())) {
            selected_index_ = items_.empty() ? -1 : static_cast<int>(items_.size()) - 1;
        }
    }
}

void Dropdown::ClearItems() {
    items_.clear();
    selected_index_ = -1;
}

void Dropdown::SetSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        selected_index_ = index;
        if (on_select_) {
            on_select_(selected_index_, items_[selected_index_]);
        }
    }
}

std::string Dropdown::GetSelectedItem() const {
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(items_.size())) {
        return items_[selected_index_];
    }
    return "";
}

std::string Dropdown::Render() const {
    std::ostringstream oss;
    oss << "[" << GetSelectedItem() << " ▼]";
    if (is_open_) {
        oss << "\n";
        for (size_t i = 0; i < items_.size(); ++i) {
            oss << "  " << (static_cast<int>(i) == selected_index_ ? "▸ " : "  ");
            oss << items_[i] << "\n";
        }
    }
    return oss.str();
}

bool Dropdown::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    if (x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_) {
        is_open_ = !is_open_;
        return true;
    }
    return false;
}

// =============================================================================
// Slider Widget
// =============================================================================

Slider::Slider(const std::string& id, float min_val, float max_val)
    : Widget(id),
      min_value_(min_val),
      max_value_(max_val),
      value_(min_val),
      step_(1.0f),
      on_value_change_(nullptr) {
}

void Slider::SetRange(float min_val, float max_val) {
    min_value_ = min_val;
    max_value_ = max_val;
    value_ = std::max(min_value_, std::min(value_, max_value_));
}

void Slider::SetValue(float value) {
    float new_value = std::max(min_value_, std::min(value, max_value_));
    if (new_value != value_) {
        value_ = new_value;
        if (on_value_change_) {
            on_value_change_(value_);
        }
    }
}

std::string Slider::Render() const {
    std::ostringstream oss;
    float ratio = (value_ - min_value_) / (max_value_ - min_value_);
    int filled = static_cast<int>(ratio * 20);
    
    oss << "[";
    for (int i = 0; i < 20; ++i) {
        oss << (i < filled ? "█" : "░");
    }
    oss << "] " << std::fixed << std::setprecision(1) << value_;
    return oss.str();
}

bool Slider::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    if (x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_) {
        float ratio = static_cast<float>(x - x_) / width_;
        SetValue(min_value_ + ratio * (max_value_ - min_value_));
        return true;
    }
    return false;
}

// =============================================================================
// Checkbox Widget
// =============================================================================

Checkbox::Checkbox(const std::string& id, const std::string& label, bool checked)
    : Widget(id),
      checked_(checked),
      on_toggle_(nullptr) {
    text_ = label;
}

void Checkbox::SetChecked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        if (on_toggle_) {
            on_toggle_(checked_);
        }
    }
}

void Checkbox::Toggle() {
    SetChecked(!checked_);
}

std::string Checkbox::Render() const {
    std::ostringstream oss;
    oss << (checked_ ? "[✓]" : "[ ]") << " " << text_;
    return oss.str();
}

bool Checkbox::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    if (x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_) {
        Toggle();
        return true;
    }
    return false;
}

// =============================================================================
// TextInput Widget
// =============================================================================

TextInput::TextInput(const std::string& id, const std::string& placeholder)
    : Widget(id),
      placeholder_(placeholder),
      value_(""),
      max_length_(256),
      cursor_pos_(0),
      is_password_(false),
      on_text_change_(nullptr),
      on_submit_(nullptr) {
}

void TextInput::SetValue(const std::string& value) {
    value_ = value.substr(0, max_length_);
    cursor_pos_ = value_.length();
    if (on_text_change_) {
        on_text_change_(value_);
    }
}

std::string TextInput::Render() const {
    std::ostringstream oss;
    oss << "[";
    if (value_.empty()) {
        oss << placeholder_;
    } else if (is_password_) {
        oss << std::string(value_.length(), '*');
    } else {
        oss << value_;
    }
    if (focused_) {
        oss << "|";
    }
    oss << "]";
    return oss.str();
}

bool TextInput::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    if (x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_) {
        focused_ = true;
        return true;
    }
    focused_ = false;
    return false;
}

bool TextInput::HandleKeyPress(int keycode) {
    if (!enabled_ || !focused_) return false;
    
    // Handle basic key input (simplified)
    if (keycode >= 32 && keycode < 127 && value_.length() < max_length_) {
        value_.insert(cursor_pos_, 1, static_cast<char>(keycode));
        cursor_pos_++;
        if (on_text_change_) {
            on_text_change_(value_);
        }
        return true;
    }
    
    // Backspace
    if (keycode == 8 && cursor_pos_ > 0) {
        value_.erase(cursor_pos_ - 1, 1);
        cursor_pos_--;
        if (on_text_change_) {
            on_text_change_(value_);
        }
        return true;
    }
    
    // Enter
    if (keycode == 13 && on_submit_) {
        on_submit_(value_);
        return true;
    }
    
    return false;
}

// =============================================================================
// TabBar Widget
// =============================================================================

TabBar::TabBar(const std::string& id)
    : Widget(id),
      active_tab_id_(""),
      on_tab_change_(nullptr) {
}

void TabBar::AddTab(const std::string& tab_id, const std::string& label,
                   const std::string& icon, bool closeable) {
    Tab tab;
    tab.id = tab_id;
    tab.label = label;
    tab.icon = icon;
    tab.closeable = closeable;
    tabs_.push_back(tab);
    
    if (active_tab_id_.empty() && !tabs_.empty()) {
        active_tab_id_ = tabs_[0].id;
    }
}

void TabBar::RemoveTab(const std::string& tab_id) {
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
                          [&tab_id](const Tab& t) { return t.id == tab_id; });
    if (it != tabs_.end()) {
        tabs_.erase(it);
        if (active_tab_id_ == tab_id && !tabs_.empty()) {
            active_tab_id_ = tabs_[0].id;
        }
    }
}

void TabBar::ClearTabs() {
    tabs_.clear();
    active_tab_id_.clear();
}

void TabBar::SetActiveTab(const std::string& tab_id) {
    auto it = std::find_if(tabs_.begin(), tabs_.end(),
                          [&tab_id](const Tab& t) { return t.id == tab_id; });
    if (it != tabs_.end()) {
        active_tab_id_ = tab_id;
        if (on_tab_change_) {
            on_tab_change_(GetActiveTabIndex(), tab_id);
        }
    }
}

void TabBar::SetActiveTabByIndex(int index) {
    if (index >= 0 && index < static_cast<int>(tabs_.size())) {
        active_tab_id_ = tabs_[index].id;
        if (on_tab_change_) {
            on_tab_change_(index, active_tab_id_);
        }
    }
}

int TabBar::GetActiveTabIndex() const {
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].id == active_tab_id_) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string TabBar::Render() const {
    std::ostringstream oss;
    for (const auto& tab : tabs_) {
        if (tab.id == active_tab_id_) {
            oss << "[" << tab.label << "]";
        } else {
            oss << " " << tab.label << " ";
        }
        oss << " ";
    }
    return oss.str();
}

bool TabBar::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified click handling - would need proper tab bounds calculation
    return false;
}

// =============================================================================
// MenuBar Widget (Arduino IDE style)
// =============================================================================

MenuBar::MenuBar(const std::string& id)
    : Widget(id),
      active_menu_id_(""),
      menu_open_(false) {
}

void MenuBar::AddMenu(const std::string& menu_id, const std::string& label) {
    Menu menu;
    menu.id = menu_id;
    menu.label = label;
    menus_.push_back(menu);
}

void MenuBar::AddMenuItem(const std::string& menu_id, const std::string& item_id,
                         const std::string& label, const std::string& shortcut,
                         MenuItemCallback callback) {
    for (auto& menu : menus_) {
        if (menu.id == menu_id) {
            MenuItem item;
            item.id = item_id;
            item.label = label;
            item.shortcut = shortcut;
            item.enabled = true;
            item.separator = false;
            item.callback = callback;
            menu.items.push_back(item);
            break;
        }
    }
}

void MenuBar::AddSeparator(const std::string& menu_id) {
    for (auto& menu : menus_) {
        if (menu.id == menu_id) {
            MenuItem separator;
            separator.separator = true;
            separator.enabled = true;
            menu.items.push_back(separator);
            break;
        }
    }
}

void MenuBar::SetActiveMenu(const std::string& menu_id) {
    active_menu_id_ = menu_id;
    menu_open_ = !menu_id.empty();
}

void MenuBar::ExecuteMenuItem(const std::string& menu_id, const std::string& item_id) {
    for (const auto& menu : menus_) {
        if (menu.id == menu_id) {
            for (const auto& item : menu.items) {
                if (item.id == item_id && item.enabled && item.callback) {
                    item.callback();
                    break;
                }
            }
            break;
        }
    }
    menu_open_ = false;
    active_menu_id_.clear();
}

std::string MenuBar::Render() const {
    std::ostringstream oss;
    
    // Render menu bar
    for (const auto& menu : menus_) {
        if (menu.id == active_menu_id_) {
            oss << "[" << menu.label << "] ";
        } else {
            oss << " " << menu.label << "  ";
        }
    }
    oss << "\n";
    
    // Render open menu
    if (menu_open_ && !active_menu_id_.empty()) {
        for (const auto& menu : menus_) {
            if (menu.id == active_menu_id_) {
                oss << "┌────────────────────────┐\n";
                for (const auto& item : menu.items) {
                    if (item.separator) {
                        oss << "├────────────────────────┤\n";
                    } else {
                        oss << "│ " << std::left << std::setw(15) << item.label;
                        oss << std::setw(8) << item.shortcut << " │\n";
                    }
                }
                oss << "└────────────────────────┘\n";
                break;
            }
        }
    }
    
    return oss.str();
}

bool MenuBar::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified - would need proper menu bounds calculation
    return false;
}

// =============================================================================
// Toolbar Widget (Arduino IDE style)
// =============================================================================

Toolbar::Toolbar(const std::string& id)
    : Widget(id) {
}

void Toolbar::AddButton(const std::string& item_id, const std::string& icon,
                       const std::string& label, const std::string& tooltip,
                       ActionCallback callback) {
    ToolbarItem item;
    item.id = item_id;
    item.icon = icon;
    item.label = label;
    item.tooltip = tooltip;
    item.enabled = true;
    item.separator = false;
    item.callback = callback;
    items_.push_back(item);
}

void Toolbar::AddSeparator() {
    ToolbarItem separator;
    separator.separator = true;
    separator.enabled = true;
    items_.push_back(separator);
}

void Toolbar::RemoveItem(const std::string& item_id) {
    items_.erase(std::remove_if(items_.begin(), items_.end(),
                                [&item_id](const ToolbarItem& i) { return i.id == item_id; }),
                 items_.end());
}

void Toolbar::SetItemEnabled(const std::string& item_id, bool enabled) {
    for (auto& item : items_) {
        if (item.id == item_id) {
            item.enabled = enabled;
            break;
        }
    }
}

void Toolbar::ExecuteAction(const std::string& item_id) {
    for (const auto& item : items_) {
        if (item.id == item_id && item.enabled && item.callback) {
            item.callback();
            break;
        }
    }
}

std::string Toolbar::Render() const {
    std::ostringstream oss;
    oss << "┃ ";
    for (const auto& item : items_) {
        if (item.separator) {
            oss << "│ ";
        } else {
            if (item.enabled) {
                oss << "[" << item.icon << "] ";
            } else {
                oss << "(" << item.icon << ") ";
            }
        }
    }
    oss << "┃";
    return oss.str();
}

bool Toolbar::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified - would need proper button bounds calculation
    return false;
}

// =============================================================================
// StatusBar Widget
// =============================================================================

StatusBar::StatusBar(const std::string& id)
    : Widget(id) {
}

void StatusBar::AddSection(const std::string& section_id, const std::string& text, int width) {
    StatusSection section;
    section.id = section_id;
    section.text = text;
    section.width = width;
    sections_.push_back(section);
}

void StatusBar::SetSectionText(const std::string& section_id, const std::string& text) {
    for (auto& section : sections_) {
        if (section.id == section_id) {
            section.text = text;
            break;
        }
    }
}

std::string StatusBar::GetSectionText(const std::string& section_id) const {
    for (const auto& section : sections_) {
        if (section.id == section_id) {
            return section.text;
        }
    }
    return "";
}

void StatusBar::SetStatusText(const std::string& text) {
    SetSectionText("status", text);
}

void StatusBar::SetCursorPosition(int line, int column) {
    std::ostringstream oss;
    oss << "Ln " << line << ", Col " << column;
    SetSectionText("cursor", oss.str());
}

void StatusBar::SetFileInfo(const std::string& filename, bool modified) {
    std::ostringstream oss;
    oss << filename;
    if (modified) {
        oss << " *";
    }
    SetSectionText("file", oss.str());
}

void StatusBar::SetBoardInfo(const std::string& board, const std::string& port) {
    std::ostringstream oss;
    oss << board << " on " << port;
    SetSectionText("board", oss.str());
}

std::string StatusBar::Render() const {
    std::ostringstream oss;
    oss << "──";
    for (const auto& section : sections_) {
        oss << "│ " << section.text << " ";
    }
    oss << "──";
    return oss.str();
}

// =============================================================================
// SplitView Widget
// =============================================================================

SplitView::SplitView(const std::string& id, Orientation orientation)
    : Widget(id),
      orientation_(orientation),
      split_ratio_(0.5f),
      min_ratio_(0.1f),
      max_ratio_(0.9f),
      dragging_(false),
      on_split_change_(nullptr) {
}

void SplitView::SetSplitRatio(float ratio) {
    float new_ratio = std::max(min_ratio_, std::min(ratio, max_ratio_));
    if (new_ratio != split_ratio_) {
        split_ratio_ = new_ratio;
        if (on_split_change_) {
            on_split_change_(split_ratio_);
        }
    }
}

std::string SplitView::Render() const {
    std::ostringstream oss;
    if (orientation_ == Orientation::HORIZONTAL) {
        oss << "[Panel 1]│[Panel 2] (" << static_cast<int>(split_ratio_ * 100) << "%)";
    } else {
        oss << "[Panel 1]\n────────\n[Panel 2] (" << static_cast<int>(split_ratio_ * 100) << "%)";
    }
    return oss.str();
}

bool SplitView::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified - would need proper splitter bounds calculation
    return false;
}

// =============================================================================
// Label Widget
// =============================================================================

Label::Label(const std::string& id, const std::string& text)
    : Widget(id),
      alignment_(Alignment::LEFT),
      color_(0xCCCCCC) {
    text_ = text;
}

std::string Label::Render() const {
    return text_;
}

// =============================================================================
// ProgressBar Widget
// =============================================================================

ProgressBar::ProgressBar(const std::string& id)
    : Widget(id),
      progress_(0.0f),
      indeterminate_(false),
      show_text_(true) {
}

void ProgressBar::SetProgress(float progress) {
    progress_ = std::max(0.0f, std::min(progress, 1.0f));
}

std::string ProgressBar::Render() const {
    std::ostringstream oss;
    int filled = static_cast<int>(progress_ * 30);
    
    oss << "[";
    if (indeterminate_) {
        for (int i = 0; i < 30; ++i) {
            oss << (i % 4 == 0 ? "▓" : "░");
        }
    } else {
        for (int i = 0; i < 30; ++i) {
            oss << (i < filled ? "█" : "░");
        }
    }
    oss << "]";
    
    if (show_text_ && !indeterminate_) {
        oss << " " << static_cast<int>(progress_ * 100) << "%";
    }
    
    return oss.str();
}

// =============================================================================
// TreeView Widget
// =============================================================================

TreeView::TreeView(const std::string& id)
    : Widget(id),
      selected_node_id_(""),
      on_select_(nullptr),
      on_expand_(nullptr) {
}

void TreeView::SetRoot(const TreeNode& root) {
    root_ = root;
}

void TreeView::AddNode(const std::string& parent_id, const TreeNode& node) {
    TreeNode* parent = FindNode(parent_id);
    if (parent) {
        parent->children.push_back(node);
    }
}

void TreeView::RemoveNode(const std::string& node_id) {
    // Simplified - would need recursive removal
}

TreeView::TreeNode* TreeView::FindNode(const std::string& node_id) {
    return FindNodeRecursive(root_, node_id);
}

TreeView::TreeNode* TreeView::FindNodeRecursive(TreeNode& node, const std::string& node_id) {
    if (node.id == node_id) {
        return &node;
    }
    for (auto& child : node.children) {
        TreeNode* found = FindNodeRecursive(child, node_id);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

void TreeView::SetSelectedNode(const std::string& node_id) {
    selected_node_id_ = node_id;
    if (on_select_) {
        on_select_(node_id);
    }
}

void TreeView::ExpandNode(const std::string& node_id) {
    TreeNode* node = FindNode(node_id);
    if (node && !node->expanded) {
        node->expanded = true;
        if (on_expand_) {
            on_expand_(node_id, true);
        }
    }
}

void TreeView::CollapseNode(const std::string& node_id) {
    TreeNode* node = FindNode(node_id);
    if (node && node->expanded) {
        node->expanded = false;
        if (on_expand_) {
            on_expand_(node_id, false);
        }
    }
}

void TreeView::ToggleNode(const std::string& node_id) {
    TreeNode* node = FindNode(node_id);
    if (node) {
        node->expanded = !node->expanded;
        if (on_expand_) {
            on_expand_(node_id, node->expanded);
        }
    }
}

std::string TreeView::Render() const {
    return RenderNode(root_, 0);
}

std::string TreeView::RenderNode(const TreeNode& node, int depth) const {
    std::ostringstream oss;
    std::string indent(depth * 2, ' ');
    
    if (!node.id.empty()) {
        oss << indent;
        if (!node.children.empty()) {
            oss << (node.expanded ? "▼ " : "▸ ");
        } else {
            oss << "  ";
        }
        oss << (node.id == selected_node_id_ ? "[" : " ");
        oss << node.icon << node.label;
        oss << (node.id == selected_node_id_ ? "]" : " ");
        oss << "\n";
    }
    
    if (node.expanded) {
        for (const auto& child : node.children) {
            oss << RenderNode(child, depth + 1);
        }
    }
    
    return oss.str();
}

bool TreeView::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified - would need proper node bounds calculation
    return false;
}

// =============================================================================
// ListView Widget
// =============================================================================

ListView::ListView(const std::string& id)
    : Widget(id),
      selected_index_(-1),
      multi_select_(false),
      on_select_(nullptr) {
}

void ListView::AddItem(const std::string& item_id, const std::string& text,
                      const std::string& icon) {
    ListItem item;
    item.id = item_id;
    item.text = text;
    item.icon = icon;
    item.selected = false;
    items_.push_back(item);
}

void ListView::RemoveItem(const std::string& item_id) {
    items_.erase(std::remove_if(items_.begin(), items_.end(),
                                [&item_id](const ListItem& i) { return i.id == item_id; }),
                 items_.end());
}

void ListView::ClearItems() {
    items_.clear();
    selected_index_ = -1;
}

void ListView::SetSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        selected_index_ = index;
        if (on_select_) {
            on_select_(index, items_[index].text);
        }
    }
}

std::string ListView::GetSelectedItemId() const {
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(items_.size())) {
        return items_[selected_index_].id;
    }
    return "";
}

std::string ListView::Render() const {
    std::ostringstream oss;
    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        if (static_cast<int>(i) == selected_index_) {
            oss << "▸ [" << item.text << "]\n";
        } else {
            oss << "   " << item.text << "\n";
        }
    }
    return oss.str();
}

bool ListView::HandleClick(int x, int y) {
    if (!enabled_ || !visible_) return false;
    // Simplified - would need proper item bounds calculation
    return false;
}

} // namespace gui
} // namespace esp32_ide
