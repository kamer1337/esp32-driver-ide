#include "gui/simple_gui_window.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"

#include <iostream>
#include <algorithm>
#include <cstring>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    // macOS Cocoa includes would go here
    #include <CoreGraphics/CoreGraphics.h>
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xos.h>
#endif

namespace esp32_ide {
namespace gui {

// Platform-specific window data structure
struct PlatformWindowData {
#ifdef _WIN32
    HWND hwnd;
    HDC hdc;
    HBRUSH background_brush;
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    Display* display;
    Window window;
    GC gc;
    int screen;
#elif defined(__APPLE__)
    // macOS window handle would go here
    void* ns_window;
#endif
};

SimpleGuiWindow::SimpleGuiWindow()
    : window_handle_(nullptr),
      text_editor_(nullptr),
      file_manager_(nullptr),
      compiler_(nullptr),
      serial_monitor_(nullptr),
      syntax_highlighter_(nullptr),
      width_(1024),
      height_(768),
      running_(false),
      title_("ESP32 Driver IDE") {
}

SimpleGuiWindow::~SimpleGuiWindow() {
    Shutdown();
}

bool SimpleGuiWindow::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    running_ = true;
    
    if (!InitializePlatform()) {
        std::cerr << "Failed to initialize platform-specific window" << std::endl;
        return false;
    }
    
    AddConsoleMessage("ESP32 Driver IDE initialized");
    AddConsoleMessage("Simple GUI Window ready");
    
    return true;
}

bool SimpleGuiWindow::InitializePlatform() {
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
    wc.lpszClassName = "ESP32IDEWindow";
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        return false;
    }
    
    platform_data->hwnd = CreateWindowEx(
        0,
        "ESP32IDEWindow",
        title_.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width_, height_,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (!platform_data->hwnd) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    
    platform_data->hdc = GetDC(platform_data->hwnd);
    platform_data->background_brush = CreateSolidBrush(RGB(43, 43, 43));
    
    return true;
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    // Linux X11 implementation
    platform_data->display = XOpenDisplay(NULL);
    if (!platform_data->display) {
        std::cerr << "Cannot open X display" << std::endl;
        return false;
    }
    
    platform_data->screen = DefaultScreen(platform_data->display);
    
    platform_data->window = XCreateSimpleWindow(
        platform_data->display,
        RootWindow(platform_data->display, platform_data->screen),
        0, 0, width_, height_, 1,
        BlackPixel(platform_data->display, platform_data->screen),
        WhitePixel(platform_data->display, platform_data->screen)
    );
    
    XStoreName(platform_data->display, platform_data->window, title_.c_str());
    
    XSelectInput(platform_data->display, platform_data->window, 
                 ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    
    platform_data->gc = XCreateGC(platform_data->display, platform_data->window, 0, NULL);
    
    XMapWindow(platform_data->display, platform_data->window);
    
    // Set background color to dark theme
    XSetForeground(platform_data->display, platform_data->gc, 0x2B2B2B);
    XFillRectangle(platform_data->display, platform_data->window, platform_data->gc,
                   0, 0, width_, height_);
    
    return true;
    
#elif defined(__APPLE__)
    // macOS Cocoa implementation
    std::cerr << "macOS implementation not yet available. Use terminal interface." << std::endl;
    return false;
    
#else
    std::cerr << "Simple GUI not available on this platform or X11 not installed." << std::endl;
    std::cerr << "Use terminal interface instead (build without -DBUILD_WITH_SIMPLE_GUI)" << std::endl;
    return false;
#endif
}

void SimpleGuiWindow::Run() {
    if (!window_handle_) {
        std::cerr << "Window not initialized" << std::endl;
        return;
    }
    
    while (running_) {
        ProcessEvents();
        Render();
    }
}

void SimpleGuiWindow::ProcessEvents() {
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    MSG msg;
    while (PeekMessage(&msg, platform_data->hwnd, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            running_ = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    while (XPending(platform_data->display)) {
        XEvent event;
        XNextEvent(platform_data->display, &event);
        
        switch (event.type) {
            case ClientMessage:
            case DestroyNotify:
                running_ = false;
                break;
            case ButtonPress:
                HandleMouseClick(event.xbutton.x, event.xbutton.y);
                break;
            case KeyPress: {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                HandleKeyPress(static_cast<int>(key));
                break;
            }
            case Expose:
                // Window needs redraw
                break;
        }
    }
#endif
}

void SimpleGuiWindow::Render() {
    ClearWindow(0x2B2B2B);
    
    // Render UI components
    RenderMenuBar();
    RenderToolbar();
    RenderEditor();
    RenderConsole();
    RenderFileList();
    
#ifdef _WIN32
    // Windows swap/present
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    // No explicit swap needed for GDI
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    XFlush(platform_data->display);
#endif
}

void SimpleGuiWindow::ClearWindow(uint32_t color) {
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    RECT rect = {0, 0, width_, height_};
    HBRUSH brush = CreateSolidBrush(RGB(
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF
    ));
    FillRect(platform_data->hdc, &rect, brush);
    DeleteObject(brush);
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    XSetForeground(platform_data->display, platform_data->gc, color);
    XFillRectangle(platform_data->display, platform_data->window, platform_data->gc,
                   0, 0, width_, height_);
#endif
}

void SimpleGuiWindow::DrawText(int x, int y, const std::string& text, uint32_t color) {
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    SetTextColor(platform_data->hdc, RGB(
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF
    ));
    SetBkMode(platform_data->hdc, TRANSPARENT);
    TextOutA(platform_data->hdc, x, y, text.c_str(), text.length());
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    XSetForeground(platform_data->display, platform_data->gc, color);
    XDrawString(platform_data->display, platform_data->window, platform_data->gc,
                x, y, text.c_str(), text.length());
#endif
}

void SimpleGuiWindow::DrawRect(int x, int y, int width, int height, uint32_t color) {
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    RECT rect = {x, y, x + width, y + height};
    HBRUSH brush = CreateSolidBrush(RGB(
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF
    ));
    FrameRect(platform_data->hdc, &rect, brush);
    DeleteObject(brush);
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    XSetForeground(platform_data->display, platform_data->gc, color);
    XDrawRectangle(platform_data->display, platform_data->window, platform_data->gc,
                   x, y, width, height);
#endif
}

void SimpleGuiWindow::DrawButton(int x, int y, int width, int height, const std::string& label) {
    // Draw button background
    DrawRect(x, y, width, height, 0x404040);
    
    // Draw button label (centered)
    int text_x = x + (width - label.length() * 7) / 2;
    int text_y = y + (height + 12) / 2;
    DrawText(text_x, text_y, label, 0xFFFFFF);
}

void SimpleGuiWindow::RenderMenuBar() {
    // Menu bar at top
    DrawRect(0, 0, width_, 25, 0x1E1E1E);
    DrawText(10, 18, "File  Edit  View  Tools  Help", 0xFFFFFF);
}

void SimpleGuiWindow::RenderToolbar() {
    // Toolbar below menu bar
    DrawRect(0, 25, width_, 50, 0x2B2B2B);
    
    DrawButton(10, 30, 80, 30, "Compile");
    DrawButton(100, 30, 80, 30, "Upload");
    DrawButton(190, 30, 80, 30, "Debug");
    
    DrawText(300, 50, "Port: /dev/ttyUSB0", 0xCCCCCC);
}

void SimpleGuiWindow::RenderFileList() {
    // File list on the left side
    int panel_x = 10;
    int panel_y = 80;
    int panel_width = 200;
    int panel_height = height_ - 180;
    
    DrawRect(panel_x, panel_y, panel_width, panel_height, 0x3C3C3C);
    DrawText(panel_x + 5, panel_y + 20, "Project Files:", 0xFFFFFF);
    
    int y_offset = panel_y + 40;
    for (size_t i = 0; i < file_list_.size() && i < 20; ++i) {
        DrawText(panel_x + 10, y_offset + (i * 18), file_list_[i], 0xCCCCCC);
    }
}

void SimpleGuiWindow::RenderEditor() {
    // Editor in the center
    int editor_x = 220;
    int editor_y = 80;
    int editor_width = width_ - 230;
    int editor_height = height_ - 280;
    
    DrawRect(editor_x, editor_y, editor_width, editor_height, 0x1E1E1E);
    DrawText(editor_x + 5, editor_y + 20, "Editor: " + current_file_, 0xFFFFFF);
    
    // Draw editor content (simplified)
    int y_offset = editor_y + 45;
    std::string content_sample = editor_content_.empty() ? 
        "void setup() {\n  // Your code here\n}\n\nvoid loop() {\n  // Your code here\n}" :
        editor_content_;
    
    // Split content into lines and display
    size_t pos = 0;
    int line = 0;
    while (pos < content_sample.length() && line < 20) {
        size_t newline_pos = content_sample.find('\n', pos);
        if (newline_pos == std::string::npos) {
            newline_pos = content_sample.length();
        }
        
        std::string line_text = content_sample.substr(pos, newline_pos - pos);
        DrawText(editor_x + 10, y_offset + (line * 18), line_text, 0xCCCCCC);
        
        pos = newline_pos + 1;
        line++;
    }
}

void SimpleGuiWindow::RenderConsole() {
    // Console at the bottom
    int console_y = height_ - 200;
    int console_height = 195;
    
    DrawRect(0, console_y, width_, console_height, 0x1A1A1A);
    DrawText(10, console_y + 20, "Console Output:", 0xFFFFFF);
    
    // Display recent console messages
    int y_offset = console_y + 40;
    size_t start_idx = console_messages_.size() > 8 ? console_messages_.size() - 8 : 0;
    for (size_t i = start_idx; i < console_messages_.size(); ++i) {
        DrawText(10, y_offset + ((i - start_idx) * 18), console_messages_[i], 0x88FF88);
    }
}

void SimpleGuiWindow::HandleMouseClick(int x, int y) {
    // Handle button clicks in toolbar
    if (y >= 30 && y <= 60) {
        if (x >= 10 && x <= 90) {
            // Compile button
            CompileCode();
        } else if (x >= 100 && x <= 180) {
            // Upload button
            UploadCode();
        } else if (x >= 190 && x <= 270) {
            // Debug button
            AddConsoleMessage("Debug clicked");
        }
    }
}

void SimpleGuiWindow::HandleKeyPress(int keycode) {
    // Handle keyboard input
    // Basic key handling - could be extended
    if (keycode == 'q' || keycode == 'Q') {
        // Quit on Q key
        // running_ = false;  // Commented out for safety
    }
}

void SimpleGuiWindow::Shutdown() {
    if (!window_handle_) {
        return;
    }
    
    ShutdownPlatform();
    delete static_cast<PlatformWindowData*>(window_handle_);
    window_handle_ = nullptr;
    running_ = false;
}

void SimpleGuiWindow::ShutdownPlatform() {
    auto* platform_data = static_cast<PlatformWindowData*>(window_handle_);
    
#ifdef _WIN32
    if (platform_data->hdc) {
        ReleaseDC(platform_data->hwnd, platform_data->hdc);
    }
    if (platform_data->background_brush) {
        DeleteObject(platform_data->background_brush);
    }
    if (platform_data->hwnd) {
        DestroyWindow(platform_data->hwnd);
    }
    
#elif defined(__linux__) && !defined(X11_NOT_AVAILABLE)
    if (platform_data->gc) {
        XFreeGC(platform_data->display, platform_data->gc);
    }
    if (platform_data->window) {
        XDestroyWindow(platform_data->display, platform_data->window);
    }
    if (platform_data->display) {
        XCloseDisplay(platform_data->display);
    }
#endif
}

void SimpleGuiWindow::SetTextEditor(TextEditor* editor) {
    text_editor_ = editor;
    if (editor && !editor->GetText().empty()) {
        editor_content_ = editor->GetText();
    }
}

void SimpleGuiWindow::SetFileManager(FileManager* file_manager) {
    file_manager_ = file_manager;
    if (file_manager) {
        file_list_ = file_manager->GetFileList();
    }
}

void SimpleGuiWindow::SetCompiler(ESP32Compiler* compiler) {
    compiler_ = compiler;
}

void SimpleGuiWindow::SetSerialMonitor(SerialMonitor* serial_monitor) {
    serial_monitor_ = serial_monitor;
}

void SimpleGuiWindow::SetSyntaxHighlighter(SyntaxHighlighter* highlighter) {
    syntax_highlighter_ = highlighter;
}

void SimpleGuiWindow::CompileCode() {
    AddConsoleMessage("=== Compilation Started ===");
    
    if (!compiler_) {
        AddConsoleMessage("Error: Compiler not initialized");
        return;
    }
    
    std::string code = editor_content_.empty() && text_editor_ ? 
                      text_editor_->GetText() : editor_content_;
    
    if (code.empty()) {
        AddConsoleMessage("Error: No code to compile");
        return;
    }
    
    auto result = compiler_->Compile(code, compiler_->GetBoard());
    
    if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
        AddConsoleMessage("Compilation successful!");
    } else {
        AddConsoleMessage("Compilation failed!");
        for (const auto& error : result.errors) {
            AddConsoleMessage("  " + error);
        }
    }
    
    AddConsoleMessage("=== Compilation Finished ===");
}

void SimpleGuiWindow::UploadCode() {
    AddConsoleMessage("=== Upload Started ===");
    
    if (!compiler_) {
        AddConsoleMessage("Error: Compiler not initialized");
        return;
    }
    
    compiler_->Upload(compiler_->GetBoard());
    AddConsoleMessage("Upload complete!");
    AddConsoleMessage("=== Upload Finished ===");
}

void SimpleGuiWindow::SaveFile() {
    if (current_file_.empty()) {
        AddConsoleMessage("No file open to save");
        return;
    }
    
    if (file_manager_) {
        file_manager_->SetFileContent(current_file_, editor_content_);
        file_manager_->SaveFile(current_file_);
        AddConsoleMessage("Saved file: " + current_file_);
    }
}

void SimpleGuiWindow::LoadFile(const std::string& filename) {
    current_file_ = filename;
    
    if (file_manager_ && file_manager_->FileExists(filename)) {
        editor_content_ = file_manager_->GetFileContent(filename);
        if (text_editor_) {
            text_editor_->SetText(editor_content_);
        }
        AddConsoleMessage("Loaded file: " + filename);
    } else {
        editor_content_ = "// New file: " + filename;
        AddConsoleMessage("Created new file: " + filename);
    }
}

void SimpleGuiWindow::AddConsoleMessage(const std::string& message) {
    console_messages_.push_back(message);
    
    // Keep only last 50 messages
    if (console_messages_.size() > 50) {
        console_messages_.erase(console_messages_.begin());
    }
}

} // namespace gui
} // namespace esp32_ide
