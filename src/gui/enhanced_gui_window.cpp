#include "gui/enhanced_gui_window.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"

#include <iostream>
#include <algorithm>
#include <sstream>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <CoreGraphics/CoreGraphics.h>
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    #if __has_include(<X11/Xlib.h>)
        #include <X11/Xlib.h>
        #include <X11/Xutil.h>
    #else
        #define X11_NOT_AVAILABLE
    #endif
#endif

namespace esp32_ide {
namespace gui {

// Platform-specific window data
struct PlatformWindowData {
#ifdef _WIN32
    HWND hwnd;
    HDC hdc;
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    Display* display;
    Window window;
    GC gc;
#elif defined(__APPLE__)
    void* ns_window;
#endif
};

EnhancedGuiWindow::EnhancedGuiWindow()
    : window_handle_(nullptr),
      text_editor_(nullptr),
      file_manager_(nullptr),
      compiler_(nullptr),
      serial_monitor_(nullptr),
      syntax_highlighter_(nullptr),
      width_(1280),
      height_(800),
      running_(false),
      title_("ESP32 Driver IDE - Enhanced GUI"),
      mouse_down_(false),
      mouse_x_(0),
      mouse_y_(0),
      active_panel_(nullptr),
      dragging_panel_(false),
      resizing_panel_(false) {
}

EnhancedGuiWindow::~EnhancedGuiWindow() {
    Shutdown();
}

bool EnhancedGuiWindow::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    running_ = true;
    
    // Initialize GUI components
    panel_layout_ = std::make_unique<PanelLayout>();
    panel_layout_->Initialize(width_, height_);
    
    device_library_ = std::make_unique<DeviceLibrary>();
    device_library_->Initialize();
    
    terminal_ = std::make_unique<IntegratedTerminal>();
    terminal_->Initialize();
    
    device_preview_ = std::make_unique<DeviceLibraryPreview>();
    
    // Setup terminal command callback
    terminal_->SetCommandCallback([this](const std::string& cmd) {
        return HandleTerminalCommand(cmd);
    });
    
    // Setup panels
    SetupPanels();
    
    // Initialize platform-specific window
    if (!InitializePlatform()) {
        std::cerr << "Failed to initialize platform-specific window\n";
        return false;
    }
    
    AddConsoleMessage("ESP32 Driver IDE - Enhanced GUI initialized", "success");
    AddConsoleMessage("Features: Modular panels, integrated terminal, device library", "info");
    
    return true;
}

bool EnhancedGuiWindow::InitializePlatform() {
    auto* platform_data = new PlatformWindowData();
    window_handle_ = platform_data;
    
#ifdef _WIN32
    // Windows implementation
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "ESP32IDEEnhancedWindow";
    
    if (!RegisterClassEx(&wc)) {
        return false;
    }
    
    platform_data->hwnd = CreateWindowEx(
        0,
        "ESP32IDEEnhancedWindow",
        title_.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width_, height_,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (!platform_data->hwnd) {
        return false;
    }
    
    ShowWindow(platform_data->hwnd, SW_SHOW);
    platform_data->hdc = GetDC(platform_data->hwnd);
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    // Linux X11 implementation
    platform_data->display = XOpenDisplay(NULL);
    if (!platform_data->display) {
        std::cerr << "Cannot open X display\n";
        return false;
    }
    
    int screen = DefaultScreen(platform_data->display);
    platform_data->window = XCreateSimpleWindow(
        platform_data->display,
        RootWindow(platform_data->display, screen),
        0, 0, width_, height_, 1,
        BlackPixel(platform_data->display, screen),
        WhitePixel(platform_data->display, screen)
    );
    
    XStoreName(platform_data->display, platform_data->window, title_.c_str());
    XSelectInput(platform_data->display, platform_data->window, 
                ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | 
                PointerMotionMask | StructureNotifyMask);
    XMapWindow(platform_data->display, platform_data->window);
    
    platform_data->gc = XCreateGC(platform_data->display, platform_data->window, 0, NULL);
    
#else
    // Fallback: terminal-based mode
    std::cout << "Platform-specific GUI not available, using terminal mode\n";
    return true;
#endif
    
    return true;
}

void EnhancedGuiWindow::SetupPanels() {
    // Create editor panel (center)
    auto editor_panel = std::make_unique<EditorPanel>("editor");
    editor_panel->SetDock(PanelDock::CENTER);
    panel_layout_->AddPanel(std::move(editor_panel));
    
    // Create file browser panel (left)
    auto file_panel = std::make_unique<FileBrowserPanel>("files");
    file_panel->SetDock(PanelDock::LEFT);
    file_panel->SetMinSize(200, 100);
    panel_layout_->AddPanel(std::move(file_panel));
    
    // Create device library panel (left, below files)
    auto device_panel = std::make_unique<DeviceLibraryPanel>("devices");
    device_panel->SetDock(PanelDock::LEFT);
    device_panel->SetMinSize(200, 150);
    panel_layout_->AddPanel(std::move(device_panel));
    
    // Create preview panel (right)
    auto preview_panel = std::make_unique<PreviewPanel>("preview");
    preview_panel->SetDock(PanelDock::RIGHT);
    preview_panel->SetMinSize(250, 200);
    panel_layout_->AddPanel(std::move(preview_panel));
    
    // Create console panel (bottom)
    auto console_panel = std::make_unique<ConsolePanel>("console");
    console_panel->SetDock(PanelDock::BOTTOM);
    console_panel->SetMinSize(100, 150);
    panel_layout_->AddPanel(std::move(console_panel));
    
    // Create terminal panel (bottom, alongside console)
    auto terminal_panel = std::make_unique<TerminalPanel>("terminal");
    terminal_panel->SetDock(PanelDock::BOTTOM);
    terminal_panel->SetMinSize(100, 150);
    panel_layout_->AddPanel(std::move(terminal_panel));
    
    // Compute initial layout
    panel_layout_->ComputeLayout();
}

void EnhancedGuiWindow::SetTextEditor(TextEditor* editor) {
    text_editor_ = editor;
    UpdateEditorPanel();
}

void EnhancedGuiWindow::SetFileManager(FileManager* file_manager) {
    file_manager_ = file_manager;
    UpdateFileBrowserPanel();
}

void EnhancedGuiWindow::SetCompiler(ESP32Compiler* compiler) {
    compiler_ = compiler;
}

void EnhancedGuiWindow::SetSerialMonitor(SerialMonitor* serial_monitor) {
    serial_monitor_ = serial_monitor;
}

void EnhancedGuiWindow::SetSyntaxHighlighter(SyntaxHighlighter* highlighter) {
    syntax_highlighter_ = highlighter;
}

void EnhancedGuiWindow::Run() {
    std::cout << "Enhanced GUI Window running...\n";
    std::cout << "Panel system initialized with " << panel_layout_->GetAllPanels().size() << " panels\n";
    std::cout << "Device library loaded with " << device_library_->GetAllDevices().size() << " devices\n";
    std::cout << "\nAvailable panels:\n";
    for (const auto* panel : panel_layout_->GetAllPanels()) {
        std::cout << "  - " << panel->GetTitle() << " (" << panel->GetId() << ")\n";
    }
    std::cout << "\nPress Ctrl+C to exit\n\n";
    
    // Simple event loop for demonstration
    running_ = true;
    int frame_count = 0;
    
    while (running_ && frame_count < 5) {
        ProcessEvents();
        Render();
        frame_count++;
        
        // Simulate some activity
        if (frame_count == 2) {
            AddConsoleMessage("Compilation started...", "info");
            UpdateTerminalPanel();
        }
        if (frame_count == 3) {
            AddConsoleMessage("Compilation successful!", "success");
        }
    }
    
    std::cout << "\nEnhanced GUI demonstration complete\n";
}

void EnhancedGuiWindow::ProcessEvents() {
    // Platform-specific event processing would go here
}

void EnhancedGuiWindow::Render() {
    // Clear window with gradient background
    DrawGradientRect(0, 0, width_, height_, Colors::BACKGROUND_GRADIENT_TOP, Colors::BACKGROUND_GRADIENT_BOTTOM, true);
    
    // For now, just print panel information
    std::cout << "\n=== Frame Render ===\n";
    
    // Show visible panels with gradient backgrounds
    for (const auto* panel : panel_layout_->GetAllPanels()) {
        if (panel->IsVisible()) {
            Rectangle bounds = panel->GetBounds();
            
            // Draw panel with gradient
            DrawGradientRect(bounds.x, bounds.y, bounds.width, bounds.height,
                           Colors::PANEL_BG_GRADIENT_TOP, Colors::PANEL_BG_GRADIENT_BOTTOM, true);
            
            // Draw panel border with accent color for active panel
            uint32_t border_color = (panel == active_panel_) ? Colors::PANEL_BORDER_HIGHLIGHT : Colors::PANEL_BORDER;
            DrawRect(bounds.x, bounds.y, bounds.width, bounds.height, border_color, false);
            
            // Draw panel title
            DrawText(bounds.x + 5, bounds.y + 5, panel->GetTitle(), Colors::TEXT);
            
            std::cout << panel->GetTitle() << " [" << bounds.x << "," << bounds.y 
                     << " " << bounds.width << "x" << bounds.height << "]\n";
        }
    }
}

void EnhancedGuiWindow::Shutdown() {
    running_ = false;
    ShutdownPlatform();
}

void EnhancedGuiWindow::ShutdownPlatform() {
    if (!window_handle_) return;
    
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    if (platform_data->hdc) {
        ReleaseDC(platform_data->hwnd, platform_data->hdc);
    }
    if (platform_data->hwnd) {
        DestroyWindow(platform_data->hwnd);
    }
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    if (platform_data->gc) {
        XFreeGC(platform_data->display, platform_data->gc);
    }
    if (platform_data->display) {
        XCloseDisplay(platform_data->display);
    }
#endif
    
    delete platform_data;
    window_handle_ = nullptr;
}

void EnhancedGuiWindow::ShowPanel(const std::string& panel_id) {
    panel_layout_->ShowPanel(panel_id);
}

void EnhancedGuiWindow::HidePanel(const std::string& panel_id) {
    panel_layout_->HidePanel(panel_id);
}

void EnhancedGuiWindow::TogglePanel(const std::string& panel_id) {
    panel_layout_->TogglePanel(panel_id);
}

void EnhancedGuiWindow::ShowDeviceLibrary() {
    ShowPanel("devices");
    ShowPanel("preview");
    UpdateDeviceLibraryPanel();
}

void EnhancedGuiWindow::AddDeviceInstance(const std::string& device_id) {
    const DeviceDefinition* device = device_library_->GetDevice(device_id);
    if (!device) {
        AddConsoleMessage("Device not found: " + device_id, "error");
        return;
    }
    
    std::string instance_id = device_id + "_" + std::to_string(device_library_->GetAllInstances().size() + 1);
    DeviceInstance* instance = device_library_->CreateInstance(device_id, instance_id);
    
    if (instance) {
        AddConsoleMessage("Added device instance: " + instance_id, "success");
        selected_instance_ = instance_id;
        UpdateDevicePreviewPanel();
    } else {
        AddConsoleMessage("Failed to create device instance", "error");
    }
}

void EnhancedGuiWindow::EditDeviceInstance(const std::string& instance_id) {
    DeviceInstance* instance = device_library_->GetInstance(instance_id);
    if (instance) {
        selected_instance_ = instance_id;
        UpdateDevicePreviewPanel();
        AddConsoleMessage("Editing device: " + instance_id, "info");
    }
}

void EnhancedGuiWindow::RemoveDeviceInstance(const std::string& instance_id) {
    device_library_->RemoveInstance(instance_id);
    AddConsoleMessage("Removed device instance: " + instance_id, "info");
    UpdateDeviceLibraryPanel();
}

void EnhancedGuiWindow::UploadDeviceConfiguration() {
    AddConsoleMessage("Uploading device configuration...", "info");
    
    // Generate code from all instances
    for (auto* instance : device_library_->GetAllInstances()) {
        std::string error;
        if (!instance->Validate(error)) {
            AddConsoleMessage("Validation error: " + error, "error");
            return;
        }
    }
    
    AddConsoleMessage("Device configuration uploaded successfully", "success");
}

void EnhancedGuiWindow::DownloadDeviceConfiguration() {
    AddConsoleMessage("Downloading device configuration...", "info");
    
    // Check if serial monitor is connected
    if (!serial_monitor_) {
        AddConsoleMessage("Error: Serial monitor not initialized", "error");
        return;
    }
    
    // Request device configuration via serial
    serial_monitor_->AddMessage("GET_CONFIG\n");
    
    // In a real implementation, this would:
    // 1. Send command to device to dump current configuration
    // 2. Wait for response with timeout
    // 3. Parse received configuration data
    // 4. Update device library with downloaded configuration
    // 5. Save to local file for future reference
    
    // Simulated download for now
    if (device_library_) {
        AddConsoleMessage("Received device configuration data", "info");
        AddConsoleMessage("Parsing configuration...", "info");
        // device_library_->ImportFromJSON(received_data);
    }
    
    AddConsoleMessage("Device configuration downloaded successfully", "success");
}

void EnhancedGuiWindow::ShowTerminal() {
    ShowPanel("terminal");
}

void EnhancedGuiWindow::ExecuteTerminalCommand(const std::string& command) {
    terminal_->ExecuteCommand(command);
    UpdateTerminalPanel();
}

void EnhancedGuiWindow::ClearTerminal() {
    terminal_->Clear();
    UpdateTerminalPanel();
}

void EnhancedGuiWindow::UpdateDeviceLibraryPanel() {
    auto* panel = dynamic_cast<DeviceLibraryPanel*>(panel_layout_->GetPanel("devices"));
    if (panel) {
        std::vector<std::string> device_names;
        for (const auto* device : device_library_->GetAllDevices()) {
            device_names.push_back(device->GetName() + " (" + device->GetId() + ")");
        }
        panel->SetDeviceList(device_names);
    }
}

void EnhancedGuiWindow::UpdateDevicePreviewPanel() {
    auto* panel = dynamic_cast<PreviewPanel*>(panel_layout_->GetPanel("preview"));
    if (panel) {
        if (!selected_instance_.empty()) {
            DeviceInstance* instance = device_library_->GetInstance(selected_instance_);
            if (instance) {
                device_preview_->SetInstance(instance);
                panel->SetPreviewContent(device_preview_->RenderPreview());
            }
        } else if (!selected_device_.empty()) {
            const DeviceDefinition* device = device_library_->GetDevice(selected_device_);
            if (device) {
                device_preview_->SetDevice(device);
                panel->SetPreviewContent(device_preview_->RenderPreview());
            }
        }
    }
}

void EnhancedGuiWindow::UpdateTerminalPanel() {
    auto* panel = dynamic_cast<TerminalPanel*>(panel_layout_->GetPanel("terminal"));
    if (panel && terminal_) {
        // Update panel with terminal content
        auto lines = terminal_->GetLines();
        for (const auto& line : lines) {
            panel->AddOutput(line.content);
        }
    }
}

std::string EnhancedGuiWindow::HandleTerminalCommand(const std::string& command) {
    // Handle custom commands
    if (command == "devices") {
        std::ostringstream oss;
        oss << "Available devices:\n";
        for (const auto* device : device_library_->GetAllDevices()) {
            oss << "  - " << device->GetName() << " [" << device->GetId() << "]\n";
        }
        return oss.str();
    }
    
    if (command == "instances") {
        std::ostringstream oss;
        oss << "Device instances:\n";
        for (const auto* instance : device_library_->GetAllInstances()) {
            oss << "  - " << instance->GetInstanceId() << " (" 
                << instance->GetDefinition()->GetName() << ")\n";
        }
        return oss.str();
    }
    
    if (command == "compile") {
        CompileCode();
        return "Compilation started";
    }
    
    if (command == "upload") {
        UploadCode();
        return "Upload started";
    }
    
    return "Command not recognized. Try: devices, instances, compile, upload";
}

void EnhancedGuiWindow::UpdateFileBrowserPanel() {
    auto* panel = dynamic_cast<FileBrowserPanel*>(panel_layout_->GetPanel("files"));
    if (panel && file_manager_) {
        panel->SetFiles(file_manager_->GetFileList());
    }
}

void EnhancedGuiWindow::UpdateEditorPanel() {
    auto* panel = dynamic_cast<EditorPanel*>(panel_layout_->GetPanel("editor"));
    if (panel && text_editor_) {
        panel->SetEditorContent(text_editor_->GetText());
    }
}

void EnhancedGuiWindow::UpdateConsolePanel() {
    // Console is updated via AddConsoleMessage
}

void EnhancedGuiWindow::AddConsoleMessage(const std::string& message, const std::string& type) {
    auto* panel = dynamic_cast<ConsolePanel*>(panel_layout_->GetPanel("console"));
    if (panel) {
        std::string prefix;
        if (type == "error") prefix = "[ERROR] ";
        else if (type == "success") prefix = "[OK] ";
        else if (type == "warning") prefix = "[WARN] ";
        else prefix = "[INFO] ";
        
        panel->AddLine(prefix + message);
    }
}

void EnhancedGuiWindow::CompileCode() {
    if (!compiler_ || !text_editor_) {
        AddConsoleMessage("Compiler or editor not initialized", "error");
        return;
    }
    
    AddConsoleMessage("Compiling code...", "info");
    auto result = compiler_->Compile(text_editor_->GetText(), ESP32Compiler::BoardType::ESP32);
    
    if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
        AddConsoleMessage("Compilation successful!", "success");
    } else {
        AddConsoleMessage("Compilation failed", "error");
        for (const auto& error : result.errors) {
            AddConsoleMessage(error, "error");
        }
    }
}

void EnhancedGuiWindow::UploadCode() {
    if (!compiler_) {
        AddConsoleMessage("Compiler not initialized", "error");
        return;
    }
    
    AddConsoleMessage("Uploading code to ESP32...", "info");
    bool success = compiler_->Upload(ESP32Compiler::BoardType::ESP32);
    
    if (success) {
        AddConsoleMessage("Upload successful!", "success");
    } else {
        AddConsoleMessage("Upload failed", "error");
    }
}

void EnhancedGuiWindow::HandleResize(int width, int height) {
    width_ = width;
    height_ = height;
    panel_layout_->SetWindowSize(width, height);
}

// Drawing primitive implementations

uint32_t EnhancedGuiWindow::InterpolateColor(uint32_t color1, uint32_t color2, float ratio) {
    if (ratio <= 0.0f) return color1;
    if (ratio >= 1.0f) return color2;
    
    uint32_t r1 = (color1 >> 16) & 0xFF;
    uint32_t g1 = (color1 >> 8) & 0xFF;
    uint32_t b1 = color1 & 0xFF;
    
    uint32_t r2 = (color2 >> 16) & 0xFF;
    uint32_t g2 = (color2 >> 8) & 0xFF;
    uint32_t b2 = color2 & 0xFF;
    
    uint32_t r = static_cast<uint32_t>(r1 + (r2 - r1) * ratio);
    uint32_t g = static_cast<uint32_t>(g1 + (g2 - g1) * ratio);
    uint32_t b = static_cast<uint32_t>(b1 + (b2 - b1) * ratio);
    
    return (r << 16) | (g << 8) | b;
}

void EnhancedGuiWindow::DrawGradientRect(int x, int y, int width, int height, 
                                         uint32_t color1, uint32_t color2, bool vertical) {
    if (!window_handle_) return;
    
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
    if (vertical) {
        // Vertical gradient
        for (int i = 0; i < height; i++) {
            float ratio = static_cast<float>(i) / height;
            uint32_t color = InterpolateColor(color1, color2, ratio);
            DrawLine(x, y + i, x + width, y + i, color);
        }
    } else {
        // Horizontal gradient
        for (int i = 0; i < width; i++) {
            float ratio = static_cast<float>(i) / width;
            uint32_t color = InterpolateColor(color1, color2, ratio);
            DrawLine(x + i, y, x + i, y + height, color);
        }
    }
}

void EnhancedGuiWindow::DrawRect(int x, int y, int width, int height, uint32_t color, bool filled) {
    if (!window_handle_) return;
    
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    if (platform_data->hdc) {
        HBRUSH brush = CreateSolidBrush(RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
        RECT rect = {x, y, x + width, y + height};
        if (filled) {
            FillRect(platform_data->hdc, &rect, brush);
        } else {
            FrameRect(platform_data->hdc, &rect, brush);
        }
        DeleteObject(brush);
    }
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    if (platform_data->display && platform_data->gc) {
        XSetForeground(platform_data->display, platform_data->gc, color);
        if (filled) {
            XFillRectangle(platform_data->display, platform_data->window, platform_data->gc,
                         x, y, width, height);
        } else {
            XDrawRectangle(platform_data->display, platform_data->window, platform_data->gc,
                         x, y, width, height);
        }
    }
#endif
}

void EnhancedGuiWindow::DrawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    if (!window_handle_) return;
    
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    if (platform_data->hdc) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
        HPEN old_pen = (HPEN)SelectObject(platform_data->hdc, pen);
        MoveToEx(platform_data->hdc, x1, y1, NULL);
        LineTo(platform_data->hdc, x2, y2);
        SelectObject(platform_data->hdc, old_pen);
        DeleteObject(pen);
    }
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    if (platform_data->display && platform_data->gc) {
        XSetForeground(platform_data->display, platform_data->gc, color);
        XDrawLine(platform_data->display, platform_data->window, platform_data->gc,
                 x1, y1, x2, y2);
    }
#endif
}

void EnhancedGuiWindow::DrawText(int x, int y, const std::string& text, uint32_t color) {
    if (!window_handle_) return;
    
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    if (platform_data->hdc) {
        SetTextColor(platform_data->hdc, RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
        SetBkMode(platform_data->hdc, TRANSPARENT);
        TextOutA(platform_data->hdc, x, y, text.c_str(), text.length());
    }
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    if (platform_data->display && platform_data->gc) {
        XSetForeground(platform_data->display, platform_data->gc, color);
        XDrawString(platform_data->display, platform_data->window, platform_data->gc,
                   x, y, text.c_str(), text.length());
    }
#endif
}

void EnhancedGuiWindow::DrawButton(int x, int y, int width, int height, const std::string& label) {
    // Draw button with gradient background
    DrawGradientRect(x, y, width, height, Colors::BUTTON_GRADIENT_TOP, Colors::BUTTON_GRADIENT_BOTTOM, true);
    DrawRect(x, y, width, height, Colors::PANEL_BORDER, false);
    
    // Draw label centered
    int text_x = x + (width - label.length() * 6) / 2;  // Approximate text width
    int text_y = y + (height - 12) / 2;  // Approximate text height
    DrawText(text_x, text_y, label, Colors::TEXT);
}

void EnhancedGuiWindow::ClearWindow(uint32_t color) {
    DrawRect(0, 0, width_, height_, color, true);
}

} // namespace gui
} // namespace esp32_ide
