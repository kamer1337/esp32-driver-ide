#ifndef ESP32_IDE_PANEL_SYSTEM_H
#define ESP32_IDE_PANEL_SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace esp32_ide {
namespace gui {

/**
 * @brief Panel position/docking location
 */
enum class PanelDock {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER,
    FLOATING
};

/**
 * @brief Panel state
 */
enum class PanelState {
    VISIBLE,
    HIDDEN,
    MINIMIZED,
    MAXIMIZED
};

/**
 * @brief Rectangle structure for panel bounds
 */
struct Rectangle {
    int x, y, width, height;
    
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}
    
    bool Contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
};

/**
 * @brief Base class for a panel
 */
class Panel {
public:
    Panel(const std::string& id, const std::string& title);
    virtual ~Panel() = default;
    
    // Identity
    const std::string& GetId() const { return id_; }
    const std::string& GetTitle() const { return title_; }
    void SetTitle(const std::string& title) { title_ = title; }
    
    // Geometry
    Rectangle GetBounds() const { return bounds_; }
    void SetBounds(const Rectangle& bounds) { bounds_ = bounds; }
    void SetPosition(int x, int y) { bounds_.x = x; bounds_.y = y; }
    void SetSize(int width, int height) { bounds_.width = width; bounds_.height = height; }
    
    // Dock state
    PanelDock GetDock() const { return dock_; }
    void SetDock(PanelDock dock) { dock_ = dock; }
    
    // Visibility
    PanelState GetState() const { return state_; }
    void SetState(PanelState state) { state_ = state; }
    bool IsVisible() const { return state_ == PanelState::VISIBLE || state_ == PanelState::MAXIMIZED; }
    
    // Properties
    void SetMinSize(int width, int height) { min_width_ = width; min_height_ = height; }
    void SetMaxSize(int width, int height) { max_width_ = width; max_height_ = height; }
    void SetResizable(bool resizable) { resizable_ = resizable; }
    void SetMoveable(bool moveable) { moveable_ = moveable; }
    void SetCloseable(bool closeable) { closeable_ = closeable; }
    
    bool IsResizable() const { return resizable_; }
    bool IsMoveable() const { return moveable_; }
    bool IsCloseable() const { return closeable_; }
    
    int GetMinWidth() const { return min_width_; }
    int GetMinHeight() const { return min_height_; }
    int GetMaxWidth() const { return max_width_; }
    int GetMaxHeight() const { return max_height_; }
    
    // Content rendering (to be implemented by derived classes)
    virtual std::string GetContent() const { return ""; }
    virtual void OnResize(int width, int height) {}
    virtual void OnMove(int x, int y) {}
    virtual void OnShow() {}
    virtual void OnHide() {}
    virtual void OnClose() {}
    
protected:
    std::string id_;
    std::string title_;
    Rectangle bounds_;
    PanelDock dock_;
    PanelState state_;
    
    bool resizable_;
    bool moveable_;
    bool closeable_;
    
    int min_width_;
    int min_height_;
    int max_width_;
    int max_height_;
};

/**
 * @brief Panel layout manager
 */
class PanelLayout {
public:
    PanelLayout();
    ~PanelLayout() = default;
    
    // Initialization
    void Initialize(int window_width, int window_height);
    void SetWindowSize(int width, int height);
    
    // Panel management
    void AddPanel(std::unique_ptr<Panel> panel);
    void RemovePanel(const std::string& panel_id);
    Panel* GetPanel(const std::string& panel_id);
    const Panel* GetPanel(const std::string& panel_id) const;
    std::vector<Panel*> GetAllPanels();
    std::vector<const Panel*> GetAllPanels() const;
    
    // Panel visibility
    void ShowPanel(const std::string& panel_id);
    void HidePanel(const std::string& panel_id);
    void TogglePanel(const std::string& panel_id);
    
    // Docking
    void DockPanel(const std::string& panel_id, PanelDock dock);
    void FloatPanel(const std::string& panel_id, int x, int y, int width, int height);
    
    // Layout computation
    void ComputeLayout();
    
    // Resize handling
    void StartResize(const std::string& panel_id, int edge);
    void UpdateResize(int dx, int dy);
    void EndResize();
    
    // Move handling
    void StartMove(const std::string& panel_id);
    void UpdateMove(int x, int y);
    void EndMove();
    
    // Hit testing
    Panel* GetPanelAt(int x, int y);
    bool IsResizeHandle(const std::string& panel_id, int x, int y, int& edge);
    
    // Save/Load layout
    bool SaveLayout(const std::string& filename) const;
    bool LoadLayout(const std::string& filename);
    
    // Splitter positions (for docked panels)
    void SetSplitterPosition(PanelDock dock, int position);
    int GetSplitterPosition(PanelDock dock) const;

private:
    std::map<std::string, std::unique_ptr<Panel>> panels_;
    int window_width_;
    int window_height_;
    
    // Resize state
    bool is_resizing_;
    std::string resizing_panel_id_;
    int resize_edge_;
    Rectangle original_bounds_;
    
    // Move state
    bool is_moving_;
    std::string moving_panel_id_;
    int move_start_x_;
    int move_start_y_;
    
    // Splitter positions (percentage of window size)
    std::map<PanelDock, int> splitter_positions_;
    
    // Layout helpers
    void LayoutDockedPanels();
    void LayoutFloatingPanels();
    Rectangle ComputeDockBounds(PanelDock dock) const;
    std::vector<Panel*> GetPanelsByDock(PanelDock dock);
};

/**
 * @brief Specific panel types
 */

// Editor panel
class EditorPanel : public Panel {
public:
    EditorPanel(const std::string& id);
    ~EditorPanel() override = default;
    
    void SetEditorContent(const std::string& content) { content_ = content; }
    std::string GetEditorContent() const { return content_; }
    std::string GetContent() const override;
    
private:
    std::string content_;
};

// Console panel
class ConsolePanel : public Panel {
public:
    ConsolePanel(const std::string& id);
    ~ConsolePanel() override = default;
    
    void AddLine(const std::string& line);
    void Clear();
    std::string GetContent() const override;
    
private:
    std::vector<std::string> lines_;
};

// File browser panel
class FileBrowserPanel : public Panel {
public:
    FileBrowserPanel(const std::string& id);
    ~FileBrowserPanel() override = default;
    
    void SetFiles(const std::vector<std::string>& files) { files_ = files; }
    std::string GetContent() const override;
    
private:
    std::vector<std::string> files_;
};

// Device library panel
class DeviceLibraryPanel : public Panel {
public:
    DeviceLibraryPanel(const std::string& id);
    ~DeviceLibraryPanel() override = default;
    
    void SetDeviceList(const std::vector<std::string>& devices) { devices_ = devices; }
    std::string GetContent() const override;
    
private:
    std::vector<std::string> devices_;
};

// Terminal panel (integrated terminal)
class TerminalPanel : public Panel {
public:
    TerminalPanel(const std::string& id);
    ~TerminalPanel() override = default;
    
    void AddOutput(const std::string& output);
    void SetPrompt(const std::string& prompt) { prompt_ = prompt; }
    std::string GetContent() const override;
    
private:
    std::vector<std::string> output_;
    std::string prompt_;
};

// Preview panel (for device preview)
class PreviewPanel : public Panel {
public:
    PreviewPanel(const std::string& id);
    ~PreviewPanel() override = default;
    
    void SetPreviewContent(const std::string& content) { preview_content_ = content; }
    std::string GetContent() const override;
    
private:
    std::string preview_content_;
};

} // namespace gui
} // namespace esp32_ide

#endif // ESP32_IDE_PANEL_SYSTEM_H
