#include "gui/panel_system.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace esp32_ide {
namespace gui {

// Panel implementation
Panel::Panel(const std::string& id, const std::string& title)
    : id_(id), title_(title), 
      dock_(PanelDock::CENTER), 
      state_(PanelState::VISIBLE),
      resizable_(true),
      moveable_(true),
      closeable_(true),
      min_width_(100),
      min_height_(100),
      max_width_(10000),
      max_height_(10000) {
}

// PanelLayout implementation
PanelLayout::PanelLayout()
    : window_width_(1024), window_height_(768),
      is_resizing_(false), resize_edge_(0),
      is_moving_(false), move_start_x_(0), move_start_y_(0) {
    
    // Default splitter positions (percentage)
    splitter_positions_[PanelDock::LEFT] = 20;    // 20% from left
    splitter_positions_[PanelDock::RIGHT] = 80;   // 80% from left
    splitter_positions_[PanelDock::TOP] = 20;     // 20% from top
    splitter_positions_[PanelDock::BOTTOM] = 80;  // 80% from top
}

void PanelLayout::Initialize(int window_width, int window_height) {
    window_width_ = window_width;
    window_height_ = window_height;
    ComputeLayout();
}

void PanelLayout::SetWindowSize(int width, int height) {
    window_width_ = width;
    window_height_ = height;
    ComputeLayout();
}

void PanelLayout::AddPanel(std::unique_ptr<Panel> panel) {
    if (panel) {
        std::string id = panel->GetId();
        panels_[id] = std::move(panel);
        ComputeLayout();
    }
}

void PanelLayout::RemovePanel(const std::string& panel_id) {
    panels_.erase(panel_id);
    ComputeLayout();
}

Panel* PanelLayout::GetPanel(const std::string& panel_id) {
    auto it = panels_.find(panel_id);
    return (it != panels_.end()) ? it->second.get() : nullptr;
}

const Panel* PanelLayout::GetPanel(const std::string& panel_id) const {
    auto it = panels_.find(panel_id);
    return (it != panels_.end()) ? it->second.get() : nullptr;
}

std::vector<Panel*> PanelLayout::GetAllPanels() {
    std::vector<Panel*> result;
    for (auto& pair : panels_) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::vector<const Panel*> PanelLayout::GetAllPanels() const {
    std::vector<const Panel*> result;
    for (const auto& pair : panels_) {
        result.push_back(pair.second.get());
    }
    return result;
}

void PanelLayout::ShowPanel(const std::string& panel_id) {
    Panel* panel = GetPanel(panel_id);
    if (panel) {
        panel->SetState(PanelState::VISIBLE);
        panel->OnShow();
        ComputeLayout();
    }
}

void PanelLayout::HidePanel(const std::string& panel_id) {
    Panel* panel = GetPanel(panel_id);
    if (panel) {
        panel->SetState(PanelState::HIDDEN);
        panel->OnHide();
        ComputeLayout();
    }
}

void PanelLayout::TogglePanel(const std::string& panel_id) {
    Panel* panel = GetPanel(panel_id);
    if (panel) {
        if (panel->IsVisible()) {
            HidePanel(panel_id);
        } else {
            ShowPanel(panel_id);
        }
    }
}

void PanelLayout::DockPanel(const std::string& panel_id, PanelDock dock) {
    Panel* panel = GetPanel(panel_id);
    if (panel) {
        panel->SetDock(dock);
        ComputeLayout();
    }
}

void PanelLayout::FloatPanel(const std::string& panel_id, int x, int y, int width, int height) {
    Panel* panel = GetPanel(panel_id);
    if (panel) {
        panel->SetDock(PanelDock::FLOATING);
        panel->SetBounds(Rectangle(x, y, width, height));
    }
}

void PanelLayout::ComputeLayout() {
    LayoutDockedPanels();
    LayoutFloatingPanels();
}

void PanelLayout::LayoutDockedPanels() {
    // Compute bounds for each docking region
    int left_width = (window_width_ * splitter_positions_[PanelDock::LEFT]) / 100;
    int right_width = window_width_ - (window_width_ * splitter_positions_[PanelDock::RIGHT]) / 100;
    int top_height = (window_height_ * splitter_positions_[PanelDock::TOP]) / 100;
    int bottom_height = window_height_ - (window_height_ * splitter_positions_[PanelDock::BOTTOM]) / 100;
    
    int center_x = left_width;
    int center_y = top_height;
    int center_width = window_width_ - left_width - right_width;
    int center_height = window_height_ - top_height - bottom_height;
    
    // Layout left panels
    auto left_panels = GetPanelsByDock(PanelDock::LEFT);
    if (!left_panels.empty()) {
        int panel_height = window_height_ / left_panels.size();
        for (size_t i = 0; i < left_panels.size(); ++i) {
            if (left_panels[i]->IsVisible()) {
                left_panels[i]->SetBounds(Rectangle(0, i * panel_height, left_width, panel_height));
            }
        }
    }
    
    // Layout right panels
    auto right_panels = GetPanelsByDock(PanelDock::RIGHT);
    if (!right_panels.empty()) {
        int right_x = window_width_ - right_width;
        int panel_height = window_height_ / right_panels.size();
        for (size_t i = 0; i < right_panels.size(); ++i) {
            if (right_panels[i]->IsVisible()) {
                right_panels[i]->SetBounds(Rectangle(right_x, i * panel_height, right_width, panel_height));
            }
        }
    }
    
    // Layout top panels
    auto top_panels = GetPanelsByDock(PanelDock::TOP);
    if (!top_panels.empty()) {
        int panel_width = center_width / top_panels.size();
        for (size_t i = 0; i < top_panels.size(); ++i) {
            if (top_panels[i]->IsVisible()) {
                top_panels[i]->SetBounds(Rectangle(center_x + i * panel_width, 0, panel_width, top_height));
            }
        }
    }
    
    // Layout bottom panels
    auto bottom_panels = GetPanelsByDock(PanelDock::BOTTOM);
    if (!bottom_panels.empty()) {
        int bottom_y = window_height_ - bottom_height;
        int panel_width = center_width / bottom_panels.size();
        for (size_t i = 0; i < bottom_panels.size(); ++i) {
            if (bottom_panels[i]->IsVisible()) {
                bottom_panels[i]->SetBounds(Rectangle(center_x + i * panel_width, bottom_y, panel_width, bottom_height));
            }
        }
    }
    
    // Layout center panels
    auto center_panels = GetPanelsByDock(PanelDock::CENTER);
    if (!center_panels.empty()) {
        // For now, just use the first center panel to fill the space
        if (center_panels[0]->IsVisible()) {
            center_panels[0]->SetBounds(Rectangle(center_x, center_y, center_width, center_height));
        }
    }
}

void PanelLayout::LayoutFloatingPanels() {
    // Floating panels keep their positions, just ensure they're within window bounds
    for (auto& pair : panels_) {
        Panel* panel = pair.second.get();
        if (panel->GetDock() == PanelDock::FLOATING && panel->IsVisible()) {
            Rectangle bounds = panel->GetBounds();
            
            // Clamp to window bounds
            if (bounds.x < 0) bounds.x = 0;
            if (bounds.y < 0) bounds.y = 0;
            if (bounds.x + bounds.width > window_width_) {
                bounds.x = window_width_ - bounds.width;
            }
            if (bounds.y + bounds.height > window_height_) {
                bounds.y = window_height_ - bounds.height;
            }
            
            panel->SetBounds(bounds);
        }
    }
}

Rectangle PanelLayout::ComputeDockBounds(PanelDock dock) const {
    int left_width = (window_width_ * GetSplitterPosition(PanelDock::LEFT)) / 100;
    int right_width = window_width_ - (window_width_ * GetSplitterPosition(PanelDock::RIGHT)) / 100;
    int top_height = (window_height_ * GetSplitterPosition(PanelDock::TOP)) / 100;
    int bottom_height = window_height_ - (window_height_ * GetSplitterPosition(PanelDock::BOTTOM)) / 100;
    
    switch (dock) {
        case PanelDock::LEFT:
            return Rectangle(0, 0, left_width, window_height_);
        case PanelDock::RIGHT:
            return Rectangle(window_width_ - right_width, 0, right_width, window_height_);
        case PanelDock::TOP:
            return Rectangle(left_width, 0, window_width_ - left_width - right_width, top_height);
        case PanelDock::BOTTOM:
            return Rectangle(left_width, window_height_ - bottom_height, 
                           window_width_ - left_width - right_width, bottom_height);
        case PanelDock::CENTER:
            return Rectangle(left_width, top_height, 
                           window_width_ - left_width - right_width, 
                           window_height_ - top_height - bottom_height);
        default:
            return Rectangle();
    }
}

std::vector<Panel*> PanelLayout::GetPanelsByDock(PanelDock dock) {
    std::vector<Panel*> result;
    for (auto& pair : panels_) {
        if (pair.second->GetDock() == dock) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

void PanelLayout::StartResize(const std::string& panel_id, int edge) {
    Panel* panel = GetPanel(panel_id);
    if (panel && panel->IsResizable()) {
        is_resizing_ = true;
        resizing_panel_id_ = panel_id;
        resize_edge_ = edge;
        original_bounds_ = panel->GetBounds();
    }
}

void PanelLayout::UpdateResize(int dx, int dy) {
    if (!is_resizing_) return;
    
    Panel* panel = GetPanel(resizing_panel_id_);
    if (!panel) return;
    
    Rectangle bounds = original_bounds_;
    
    // Apply resize based on edge
    if (resize_edge_ & 1) { // Left edge
        bounds.x += dx;
        bounds.width -= dx;
    }
    if (resize_edge_ & 2) { // Right edge
        bounds.width += dx;
    }
    if (resize_edge_ & 4) { // Top edge
        bounds.y += dy;
        bounds.height -= dy;
    }
    if (resize_edge_ & 8) { // Bottom edge
        bounds.height += dy;
    }
    
    // Apply size constraints
    if (bounds.width < panel->GetMinWidth()) bounds.width = panel->GetMinWidth();
    if (bounds.height < panel->GetMinHeight()) bounds.height = panel->GetMinHeight();
    if (bounds.width > panel->GetMaxWidth()) bounds.width = panel->GetMaxWidth();
    if (bounds.height > panel->GetMaxHeight()) bounds.height = panel->GetMaxHeight();
    
    panel->SetBounds(bounds);
    panel->OnResize(bounds.width, bounds.height);
}

void PanelLayout::EndResize() {
    is_resizing_ = false;
    resizing_panel_id_ = "";
}

void PanelLayout::StartMove(const std::string& panel_id) {
    Panel* panel = GetPanel(panel_id);
    if (panel && panel->IsMoveable()) {
        is_moving_ = true;
        moving_panel_id_ = panel_id;
    }
}

void PanelLayout::UpdateMove(int x, int y) {
    if (!is_moving_) return;
    
    Panel* panel = GetPanel(moving_panel_id_);
    if (!panel) return;
    
    panel->SetPosition(x, y);
    panel->OnMove(x, y);
}

void PanelLayout::EndMove() {
    is_moving_ = false;
    moving_panel_id_ = "";
}

Panel* PanelLayout::GetPanelAt(int x, int y) {
    // Check floating panels first (they're on top)
    for (auto& pair : panels_) {
        Panel* panel = pair.second.get();
        if (panel->GetDock() == PanelDock::FLOATING && panel->IsVisible()) {
            if (panel->GetBounds().Contains(x, y)) {
                return panel;
            }
        }
    }
    
    // Check docked panels
    for (auto& pair : panels_) {
        Panel* panel = pair.second.get();
        if (panel->GetDock() != PanelDock::FLOATING && panel->IsVisible()) {
            if (panel->GetBounds().Contains(x, y)) {
                return panel;
            }
        }
    }
    
    return nullptr;
}

bool PanelLayout::IsResizeHandle(const std::string& panel_id, int x, int y, int& edge) {
    Panel* panel = GetPanel(panel_id);
    if (!panel || !panel->IsResizable()) return false;
    
    Rectangle bounds = panel->GetBounds();
    const int handle_size = 5;
    
    edge = 0;
    if (x >= bounds.x - handle_size && x <= bounds.x + handle_size) edge |= 1; // Left
    if (x >= bounds.x + bounds.width - handle_size && x <= bounds.x + bounds.width + handle_size) edge |= 2; // Right
    if (y >= bounds.y - handle_size && y <= bounds.y + handle_size) edge |= 4; // Top
    if (y >= bounds.y + bounds.height - handle_size && y <= bounds.y + bounds.height + handle_size) edge |= 8; // Bottom
    
    return edge != 0;
}

void PanelLayout::SetSplitterPosition(PanelDock dock, int position) {
    splitter_positions_[dock] = position;
    ComputeLayout();
}

int PanelLayout::GetSplitterPosition(PanelDock dock) const {
    auto it = splitter_positions_.find(dock);
    return (it != splitter_positions_.end()) ? it->second : 50;
}

bool PanelLayout::SaveLayout(const std::string& filename) const {
    // TODO: Implement layout serialization
    return false;
}

bool PanelLayout::LoadLayout(const std::string& filename) {
    // TODO: Implement layout deserialization
    return false;
}

// EditorPanel implementation
EditorPanel::EditorPanel(const std::string& id) 
    : Panel(id, "Editor") {
}

std::string EditorPanel::GetContent() const {
    return content_;
}

// ConsolePanel implementation
ConsolePanel::ConsolePanel(const std::string& id)
    : Panel(id, "Console") {
}

void ConsolePanel::AddLine(const std::string& line) {
    lines_.push_back(line);
    if (lines_.size() > 1000) {
        lines_.erase(lines_.begin());
    }
}

void ConsolePanel::Clear() {
    lines_.clear();
}

std::string ConsolePanel::GetContent() const {
    std::ostringstream oss;
    for (const auto& line : lines_) {
        oss << line << "\n";
    }
    return oss.str();
}

// FileBrowserPanel implementation
FileBrowserPanel::FileBrowserPanel(const std::string& id)
    : Panel(id, "Files") {
}

std::string FileBrowserPanel::GetContent() const {
    std::ostringstream oss;
    oss << "Files:\n";
    for (const auto& file : files_) {
        oss << "  " << file << "\n";
    }
    return oss.str();
}

// DeviceLibraryPanel implementation
DeviceLibraryPanel::DeviceLibraryPanel(const std::string& id)
    : Panel(id, "Device Library") {
}

std::string DeviceLibraryPanel::GetContent() const {
    std::ostringstream oss;
    oss << "Devices:\n";
    for (const auto& device : devices_) {
        oss << "  " << device << "\n";
    }
    return oss.str();
}

// TerminalPanel implementation
TerminalPanel::TerminalPanel(const std::string& id)
    : Panel(id, "Terminal"), prompt_("$ ") {
}

void TerminalPanel::AddOutput(const std::string& output) {
    output_.push_back(output);
    if (output_.size() > 1000) {
        output_.erase(output_.begin());
    }
}

std::string TerminalPanel::GetContent() const {
    std::ostringstream oss;
    for (const auto& line : output_) {
        oss << line << "\n";
    }
    oss << prompt_;
    return oss.str();
}

// PreviewPanel implementation
PreviewPanel::PreviewPanel(const std::string& id)
    : Panel(id, "Preview") {
}

std::string PreviewPanel::GetContent() const {
    return preview_content_;
}

} // namespace gui
} // namespace esp32_ide
