#ifndef ESP32_IDE_WIDGETS_H
#define ESP32_IDE_WIDGETS_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace esp32_ide {
namespace gui {

/**
 * @brief Base class for all GUI widgets
 * 
 * Inspired by Arduino IDE's widget system, providing consistent
 * interface for all UI elements
 */
class Widget {
public:
    Widget(const std::string& id);
    virtual ~Widget() = default;
    
    // Identity
    const std::string& GetId() const { return id_; }
    const std::string& GetText() const { return text_; }
    void SetText(const std::string& text) { text_ = text; }
    
    // Geometry
    void SetPosition(int x, int y) { x_ = x; y_ = y; }
    void SetSize(int width, int height) { width_ = width; height_ = height; }
    int GetX() const { return x_; }
    int GetY() const { return y_; }
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    
    // State
    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }
    bool IsVisible() const { return visible_; }
    void SetVisible(bool visible) { visible_ = visible; }
    bool IsFocused() const { return focused_; }
    void SetFocused(bool focused) { focused_ = focused; }
    
    // Tooltip
    void SetTooltip(const std::string& tooltip) { tooltip_ = tooltip; }
    const std::string& GetTooltip() const { return tooltip_; }
    
    // Virtual methods for derived classes
    virtual std::string GetType() const = 0;
    virtual std::string Render() const = 0;
    virtual bool HandleClick(int x, int y) { return false; }
    virtual bool HandleKeyPress(int keycode) { return false; }
    
protected:
    std::string id_;
    std::string text_;
    int x_, y_;
    int width_, height_;
    bool enabled_;
    bool visible_;
    bool focused_;
    std::string tooltip_;
};

/**
 * @brief Button widget with click callback
 */
class Button : public Widget {
public:
    using ClickCallback = std::function<void()>;
    
    Button(const std::string& id, const std::string& label);
    ~Button() override = default;
    
    void SetLabel(const std::string& label) { text_ = label; }
    const std::string& GetLabel() const { return text_; }
    
    void SetIcon(const std::string& icon) { icon_ = icon; }
    const std::string& GetIcon() const { return icon_; }
    
    void SetOnClick(ClickCallback callback) { on_click_ = callback; }
    void Click();
    
    std::string GetType() const override { return "Button"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::string icon_;
    ClickCallback on_click_;
};

/**
 * @brief Dropdown/ComboBox widget for selection
 */
class Dropdown : public Widget {
public:
    using SelectCallback = std::function<void(int index, const std::string& value)>;
    
    Dropdown(const std::string& id);
    ~Dropdown() override = default;
    
    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();
    
    const std::vector<std::string>& GetItems() const { return items_; }
    
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return selected_index_; }
    std::string GetSelectedItem() const;
    
    void SetOnSelect(SelectCallback callback) { on_select_ = callback; }
    
    bool IsOpen() const { return is_open_; }
    void SetOpen(bool open) { is_open_ = open; }
    
    std::string GetType() const override { return "Dropdown"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::vector<std::string> items_;
    int selected_index_;
    bool is_open_;
    SelectCallback on_select_;
};

/**
 * @brief Slider widget for value selection
 */
class Slider : public Widget {
public:
    using ValueCallback = std::function<void(float value)>;
    
    Slider(const std::string& id, float min_val = 0.0f, float max_val = 100.0f);
    ~Slider() override = default;
    
    void SetRange(float min_val, float max_val);
    float GetMinValue() const { return min_value_; }
    float GetMaxValue() const { return max_value_; }
    
    void SetValue(float value);
    float GetValue() const { return value_; }
    
    void SetStep(float step) { step_ = step; }
    float GetStep() const { return step_; }
    
    void SetOnValueChange(ValueCallback callback) { on_value_change_ = callback; }
    
    std::string GetType() const override { return "Slider"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    float min_value_;
    float max_value_;
    float value_;
    float step_;
    ValueCallback on_value_change_;
};

/**
 * @brief Checkbox widget for boolean values
 */
class Checkbox : public Widget {
public:
    using ToggleCallback = std::function<void(bool checked)>;
    
    Checkbox(const std::string& id, const std::string& label, bool checked = false);
    ~Checkbox() override = default;
    
    bool IsChecked() const { return checked_; }
    void SetChecked(bool checked);
    void Toggle();
    
    void SetOnToggle(ToggleCallback callback) { on_toggle_ = callback; }
    
    std::string GetType() const override { return "Checkbox"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    bool checked_;
    ToggleCallback on_toggle_;
};

/**
 * @brief TextInput widget for text entry
 */
class TextInput : public Widget {
public:
    using TextChangeCallback = std::function<void(const std::string& text)>;
    using SubmitCallback = std::function<void(const std::string& text)>;
    
    TextInput(const std::string& id, const std::string& placeholder = "");
    ~TextInput() override = default;
    
    void SetPlaceholder(const std::string& placeholder) { placeholder_ = placeholder; }
    const std::string& GetPlaceholder() const { return placeholder_; }
    
    void SetValue(const std::string& value);
    const std::string& GetValue() const { return value_; }
    
    void SetMaxLength(size_t max_length) { max_length_ = max_length; }
    size_t GetMaxLength() const { return max_length_; }
    
    void SetPassword(bool password) { is_password_ = password; }
    bool IsPassword() const { return is_password_; }
    
    void SetOnTextChange(TextChangeCallback callback) { on_text_change_ = callback; }
    void SetOnSubmit(SubmitCallback callback) { on_submit_ = callback; }
    
    void Clear() { value_.clear(); cursor_pos_ = 0; }
    
    std::string GetType() const override { return "TextInput"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    bool HandleKeyPress(int keycode) override;
    
private:
    std::string placeholder_;
    std::string value_;
    size_t max_length_;
    size_t cursor_pos_;
    bool is_password_;
    TextChangeCallback on_text_change_;
    SubmitCallback on_submit_;
};

/**
 * @brief TabBar widget for multi-tab navigation
 */
class TabBar : public Widget {
public:
    using TabChangeCallback = std::function<void(int index, const std::string& tab_id)>;
    
    struct Tab {
        std::string id;
        std::string label;
        std::string icon;
        bool closeable;
    };
    
    TabBar(const std::string& id);
    ~TabBar() override = default;
    
    void AddTab(const std::string& tab_id, const std::string& label, 
                const std::string& icon = "", bool closeable = true);
    void RemoveTab(const std::string& tab_id);
    void ClearTabs();
    
    const std::vector<Tab>& GetTabs() const { return tabs_; }
    
    void SetActiveTab(const std::string& tab_id);
    void SetActiveTabByIndex(int index);
    const std::string& GetActiveTabId() const { return active_tab_id_; }
    int GetActiveTabIndex() const;
    
    void SetOnTabChange(TabChangeCallback callback) { on_tab_change_ = callback; }
    
    std::string GetType() const override { return "TabBar"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::vector<Tab> tabs_;
    std::string active_tab_id_;
    TabChangeCallback on_tab_change_;
};

/**
 * @brief MenuBar widget with dropdown menus (Arduino IDE style)
 */
class MenuBar : public Widget {
public:
    using MenuItemCallback = std::function<void()>;
    
    struct MenuItem {
        std::string id;
        std::string label;
        std::string shortcut;
        bool enabled;
        bool separator;
        std::vector<MenuItem> submenu;
        MenuItemCallback callback;
    };
    
    struct Menu {
        std::string id;
        std::string label;
        std::vector<MenuItem> items;
    };
    
    MenuBar(const std::string& id);
    ~MenuBar() override = default;
    
    void AddMenu(const std::string& menu_id, const std::string& label);
    void AddMenuItem(const std::string& menu_id, const std::string& item_id,
                    const std::string& label, const std::string& shortcut = "",
                    MenuItemCallback callback = nullptr);
    void AddSeparator(const std::string& menu_id);
    
    const std::vector<Menu>& GetMenus() const { return menus_; }
    
    void SetActiveMenu(const std::string& menu_id);
    const std::string& GetActiveMenuId() const { return active_menu_id_; }
    
    void ExecuteMenuItem(const std::string& menu_id, const std::string& item_id);
    
    std::string GetType() const override { return "MenuBar"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::vector<Menu> menus_;
    std::string active_menu_id_;
    bool menu_open_;
};

/**
 * @brief Toolbar widget with action buttons (Arduino IDE style)
 */
class Toolbar : public Widget {
public:
    using ActionCallback = std::function<void()>;
    
    struct ToolbarItem {
        std::string id;
        std::string icon;
        std::string label;
        std::string tooltip;
        bool enabled;
        bool separator;
        ActionCallback callback;
    };
    
    Toolbar(const std::string& id);
    ~Toolbar() override = default;
    
    void AddButton(const std::string& item_id, const std::string& icon,
                  const std::string& label, const std::string& tooltip,
                  ActionCallback callback = nullptr);
    void AddSeparator();
    void RemoveItem(const std::string& item_id);
    
    void SetItemEnabled(const std::string& item_id, bool enabled);
    
    const std::vector<ToolbarItem>& GetItems() const { return items_; }
    
    void ExecuteAction(const std::string& item_id);
    
    std::string GetType() const override { return "Toolbar"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::vector<ToolbarItem> items_;
};

/**
 * @brief StatusBar widget for status display
 */
class StatusBar : public Widget {
public:
    struct StatusSection {
        std::string id;
        std::string text;
        int width;  // -1 for flexible
    };
    
    StatusBar(const std::string& id);
    ~StatusBar() override = default;
    
    void AddSection(const std::string& section_id, const std::string& text, int width = -1);
    void SetSectionText(const std::string& section_id, const std::string& text);
    std::string GetSectionText(const std::string& section_id) const;
    
    const std::vector<StatusSection>& GetSections() const { return sections_; }
    
    // Convenience methods
    void SetStatusText(const std::string& text);  // Set main status
    void SetCursorPosition(int line, int column);
    void SetFileInfo(const std::string& filename, bool modified);
    void SetBoardInfo(const std::string& board, const std::string& port);
    
    std::string GetType() const override { return "StatusBar"; }
    std::string Render() const override;
    
private:
    std::vector<StatusSection> sections_;
};

/**
 * @brief SplitView widget for resizable panels
 */
class SplitView : public Widget {
public:
    enum class Orientation {
        HORIZONTAL,
        VERTICAL
    };
    
    using SplitChangeCallback = std::function<void(float ratio)>;
    
    SplitView(const std::string& id, Orientation orientation = Orientation::HORIZONTAL);
    ~SplitView() override = default;
    
    Orientation GetOrientation() const { return orientation_; }
    void SetOrientation(Orientation orientation) { orientation_ = orientation; }
    
    void SetSplitRatio(float ratio);  // 0.0 to 1.0
    float GetSplitRatio() const { return split_ratio_; }
    
    void SetMinRatio(float min_ratio) { min_ratio_ = min_ratio; }
    void SetMaxRatio(float max_ratio) { max_ratio_ = max_ratio; }
    
    void SetOnSplitChange(SplitChangeCallback callback) { on_split_change_ = callback; }
    
    std::string GetType() const override { return "SplitView"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    Orientation orientation_;
    float split_ratio_;
    float min_ratio_;
    float max_ratio_;
    bool dragging_;
    SplitChangeCallback on_split_change_;
};

/**
 * @brief Label widget for static text display
 */
class Label : public Widget {
public:
    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT
    };
    
    Label(const std::string& id, const std::string& text);
    ~Label() override = default;
    
    void SetAlignment(Alignment alignment) { alignment_ = alignment; }
    Alignment GetAlignment() const { return alignment_; }
    
    void SetColor(uint32_t color) { color_ = color; }
    uint32_t GetColor() const { return color_; }
    
    std::string GetType() const override { return "Label"; }
    std::string Render() const override;
    
private:
    Alignment alignment_;
    uint32_t color_;
};

/**
 * @brief ProgressBar widget for progress display
 */
class ProgressBar : public Widget {
public:
    ProgressBar(const std::string& id);
    ~ProgressBar() override = default;
    
    void SetProgress(float progress);  // 0.0 to 1.0
    float GetProgress() const { return progress_; }
    
    void SetIndeterminate(bool indeterminate) { indeterminate_ = indeterminate; }
    bool IsIndeterminate() const { return indeterminate_; }
    
    void SetShowText(bool show) { show_text_ = show; }
    bool GetShowText() const { return show_text_; }
    
    std::string GetType() const override { return "ProgressBar"; }
    std::string Render() const override;
    
private:
    float progress_;
    bool indeterminate_;
    bool show_text_;
};

/**
 * @brief TreeView widget for hierarchical data display
 */
class TreeView : public Widget {
public:
    using SelectCallback = std::function<void(const std::string& node_id)>;
    using ExpandCallback = std::function<void(const std::string& node_id, bool expanded)>;
    
    struct TreeNode {
        std::string id;
        std::string label;
        std::string icon;
        bool expanded;
        bool selectable;
        std::vector<TreeNode> children;
    };
    
    TreeView(const std::string& id);
    ~TreeView() override = default;
    
    void SetRoot(const TreeNode& root);
    const TreeNode& GetRoot() const { return root_; }
    
    void AddNode(const std::string& parent_id, const TreeNode& node);
    void RemoveNode(const std::string& node_id);
    TreeNode* FindNode(const std::string& node_id);
    
    void SetSelectedNode(const std::string& node_id);
    const std::string& GetSelectedNodeId() const { return selected_node_id_; }
    
    void ExpandNode(const std::string& node_id);
    void CollapseNode(const std::string& node_id);
    void ToggleNode(const std::string& node_id);
    
    void SetOnSelect(SelectCallback callback) { on_select_ = callback; }
    void SetOnExpand(ExpandCallback callback) { on_expand_ = callback; }
    
    std::string GetType() const override { return "TreeView"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    TreeNode root_;
    std::string selected_node_id_;
    SelectCallback on_select_;
    ExpandCallback on_expand_;
    
    TreeNode* FindNodeRecursive(TreeNode& node, const std::string& node_id);
    std::string RenderNode(const TreeNode& node, int depth) const;
};

/**
 * @brief ListView widget for list display with selection
 */
class ListView : public Widget {
public:
    using SelectCallback = std::function<void(int index, const std::string& item)>;
    
    struct ListItem {
        std::string id;
        std::string text;
        std::string icon;
        bool selected;
    };
    
    ListView(const std::string& id);
    ~ListView() override = default;
    
    void AddItem(const std::string& item_id, const std::string& text, 
                const std::string& icon = "");
    void RemoveItem(const std::string& item_id);
    void ClearItems();
    
    const std::vector<ListItem>& GetItems() const { return items_; }
    
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return selected_index_; }
    std::string GetSelectedItemId() const;
    
    void SetMultiSelect(bool multi) { multi_select_ = multi; }
    bool IsMultiSelect() const { return multi_select_; }
    
    void SetOnSelect(SelectCallback callback) { on_select_ = callback; }
    
    std::string GetType() const override { return "ListView"; }
    std::string Render() const override;
    bool HandleClick(int x, int y) override;
    
private:
    std::vector<ListItem> items_;
    int selected_index_;
    bool multi_select_;
    SelectCallback on_select_;
};

} // namespace gui
} // namespace esp32_ide

#endif // ESP32_IDE_WIDGETS_H
