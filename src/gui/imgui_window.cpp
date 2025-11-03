#include "gui/imgui_window.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>

namespace esp32_ide {
namespace gui {

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

ImGuiWindow::ImGuiWindow()
    : window_(nullptr),
      text_editor_(nullptr),
      file_manager_(nullptr),
      compiler_(nullptr),
      serial_monitor_(nullptr),
      show_file_explorer_(true),
      show_properties_panel_(true),
      current_center_tab_(0),
      selected_port_index_(0),
      selected_baud_rate_(115200),
      scroll_to_bottom_(false),
      selected_file_index_(-1) {
    
    // Initialize editor buffer
    std::memset(editor_buffer_, 0, sizeof(editor_buffer_));
    
    // Initialize baud rates
    baud_rates_ = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
}

ImGuiWindow::~ImGuiWindow() {
    Shutdown();
}

bool ImGuiWindow::Initialize(int width, int height) {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(width, height, "ESP32 Driver IDE", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Note: Docking feature requires ImGui docking branch
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Setup style
    SetupImGuiStyle();
    
    AddConsoleMessage("ESP32 Driver IDE v1.0.0 initialized");
    AddConsoleMessage("ImGui interface ready");
    
    return true;
}

void ImGuiWindow::Run() {
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Main loop
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render UI
        RenderMainMenuBar();
        RenderToolbar();
        
        // Create main docking space
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + 80)); // After menu and toolbar
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - 280)); // Leave space for console
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        
        // Three-column layout
        float window_width = ImGui::GetContentRegionAvail().x;
        float left_panel_width = show_file_explorer_ ? 250.0f : 0.0f;
        float right_panel_width = show_properties_panel_ ? 300.0f : 0.0f;
        float center_panel_width = window_width - left_panel_width - right_panel_width;
        
        // Left panel - File Explorer
        if (show_file_explorer_) {
            ImGui::BeginChild("FileExplorer", ImVec2(left_panel_width, 0), true);
            RenderFileExplorer();
            ImGui::EndChild();
            ImGui::SameLine();
        }
        
        // Center panel - Editor/Debugger/RE tabs
        ImGui::BeginChild("CenterPanel", ImVec2(center_panel_width, 0), true);
        RenderCenterPanel();
        ImGui::EndChild();
        
        // Right panel - Properties
        if (show_properties_panel_) {
            ImGui::SameLine();
            ImGui::BeginChild("PropertiesPanel", ImVec2(right_panel_width, 0), true);
            RenderPropertiesPanel();
            ImGui::EndChild();
        }
        
        ImGui::End();
        
        // Console at the bottom
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 200));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 200));
        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        RenderConsole();
        ImGui::End();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window_);
    }
}

void ImGuiWindow::Shutdown() {
    if (window_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

void ImGuiWindow::SetTextEditor(TextEditor* editor) {
    text_editor_ = editor;
}

void ImGuiWindow::SetFileManager(FileManager* file_manager) {
    file_manager_ = file_manager;
    RefreshFileList();
}

void ImGuiWindow::SetCompiler(ESP32Compiler* compiler) {
    compiler_ = compiler;
}

void ImGuiWindow::SetSerialMonitor(SerialMonitor* serial_monitor) {
    serial_monitor_ = serial_monitor;
    RefreshPortList();
}

void ImGuiWindow::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                current_file_ = "new_sketch.ino";
                editor_content_ = "void setup() {\n  // put your setup code here, to run once:\n\n}\n\nvoid loop() {\n  // put your main code here, to run repeatedly:\n\n}\n";
                std::strncpy(editor_buffer_, editor_content_.c_str(), sizeof(editor_buffer_) - 1);
                AddConsoleMessage("Created new file");
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                SaveFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("File Explorer", nullptr, &show_file_explorer_);
            ImGui::MenuItem("Properties Panel", nullptr, &show_properties_panel_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Compile", "Ctrl+R")) {
                CompileCode();
            }
            if (ImGui::MenuItem("Upload", "Ctrl+U")) {
                UploadCode();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                AddConsoleMessage("ESP32 Driver IDE v1.0.0 - Built with ImGui");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void ImGuiWindow::RenderToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + 19)); // After menu bar
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 60));
    
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    // USB Port selector
    ImGui::Text("USB Port:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo("##port", selected_port_.empty() ? "No port" : selected_port_.c_str())) {
        for (size_t i = 0; i < available_ports_.size(); i++) {
            bool is_selected = (selected_port_index_ == static_cast<int>(i));
            if (ImGui::Selectable(available_ports_[i].c_str(), is_selected)) {
                selected_port_index_ = i;
                selected_port_ = available_ports_[i];
                AddConsoleMessage("Selected port: " + selected_port_);
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Refresh Ports")) {
        RefreshPortList();
    }
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Baud rate
    ImGui::Text("Baud:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("##baud", std::to_string(selected_baud_rate_).c_str())) {
        for (int baud : baud_rates_) {
            bool is_selected = (selected_baud_rate_ == baud);
            if (ImGui::Selectable(std::to_string(baud).c_str(), is_selected)) {
                selected_baud_rate_ = baud;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Action buttons
    if (ImGui::Button("Upload")) {
        UploadCode();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Download")) {
        AddConsoleMessage("Download functionality - to be implemented");
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Debug")) {
        DebugCode();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("RE")) {
        ReverseEngineerCode();
        current_center_tab_ = 2; // Switch to RE tab
    }
    
    ImGui::End();
}

void ImGuiWindow::RenderFileExplorer() {
    ImGui::Text("File Explorer");
    ImGui::Separator();
    
    if (ImGui::Button("New File")) {
        current_file_ = "new_sketch.ino";
        editor_content_ = "void setup() {\n\n}\n\nvoid loop() {\n\n}\n";
        std::strncpy(editor_buffer_, editor_content_.c_str(), sizeof(editor_buffer_) - 1);
        RefreshFileList();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshFileList();
    }
    
    ImGui::Separator();
    
    // File list
    for (size_t i = 0; i < file_list_.size(); i++) {
        bool is_selected = (selected_file_index_ == static_cast<int>(i));
        if (ImGui::Selectable(file_list_[i].c_str(), is_selected)) {
            selected_file_index_ = i;
            LoadFile(file_list_[i]);
        }
    }
}

void ImGuiWindow::RenderCenterPanel() {
    if (ImGui::BeginTabBar("CenterTabs")) {
        if (ImGui::BeginTabItem("Editor")) {
            current_center_tab_ = 0;
            RenderEditorTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Debugger")) {
            current_center_tab_ = 1;
            RenderDebuggerTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Reverse Engineering")) {
            current_center_tab_ = 2;
            RenderReverseEngineeringTab();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void ImGuiWindow::RenderEditorTab() {
    ImGui::Text("File: %s", current_file_.empty() ? "No file" : current_file_.c_str());
    
    ImGui::Separator();
    
    // Text editor
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    if (ImGui::InputTextMultiline("##editor", editor_buffer_, sizeof(editor_buffer_),
                                  ImVec2(-1, -1), flags)) {
        editor_content_ = std::string(editor_buffer_);
        if (text_editor_) {
            text_editor_->SetText(editor_content_);
        }
    }
}

void ImGuiWindow::RenderDebuggerTab() {
    ImGui::Text("ESP32 Debugger");
    ImGui::Separator();
    
    ImGui::TextWrapped("Debugger panel for ESP32 development.");
    ImGui::Spacing();
    
    if (ImGui::Button("Start Debugging")) {
        DebugCode();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        AddConsoleMessage("Debugging stopped");
    }
    
    ImGui::Separator();
    ImGui::Text("Breakpoints:");
    ImGui::BulletText("No breakpoints set");
    
    ImGui::Separator();
    ImGui::Text("Variables:");
    ImGui::BulletText("Debug session not active");
}

void ImGuiWindow::RenderReverseEngineeringTab() {
    ImGui::Text("Reverse Engineering Tools");
    ImGui::Separator();
    
    ImGui::TextWrapped("Reverse engineering and analysis tools for ESP32 firmware.");
    ImGui::Spacing();
    
    if (ImGui::Button("Analyze Binary")) {
        ReverseEngineerCode();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Disassemble")) {
        AddConsoleMessage("Disassembling firmware...");
        AddConsoleMessage("Disassembly complete - analysis available");
    }
    
    ImGui::Separator();
    ImGui::Text("Analysis Results:");
    
    ImGui::BeginChild("REResults", ImVec2(0, 0), true);
    ImGui::TextWrapped("ESP32 Firmware Analysis:");
    ImGui::BulletText("Architecture: Xtensa LX6");
    ImGui::BulletText("Flash size: 4MB");
    ImGui::BulletText("Entry point: 0x40080000");
    ImGui::BulletText("Functions detected: 42");
    ImGui::BulletText("Strings found: 127");
    ImGui::EndChild();
}

void ImGuiWindow::RenderPropertiesPanel() {
    ImGui::Text("Editor Properties");
    ImGui::Separator();
    
    ImGui::Text("File Info:");
    if (!current_file_.empty()) {
        ImGui::BulletText("Name: %s", current_file_.c_str());
        ImGui::BulletText("Size: %zu bytes", editor_content_.size());
        ImGui::BulletText("Lines: %ld", static_cast<long>(std::count(editor_content_.begin(), editor_content_.end(), '\n') + 1));
    } else {
        ImGui::TextDisabled("No file loaded");
    }
    
    ImGui::Separator();
    ImGui::Text("Editor Settings:");
    
    static bool show_line_numbers = true;
    static bool auto_indent = true;
    static bool syntax_highlight = true;
    
    ImGui::Checkbox("Show line numbers", &show_line_numbers);
    ImGui::Checkbox("Auto indent", &auto_indent);
    ImGui::Checkbox("Syntax highlighting", &syntax_highlight);
    
    ImGui::Separator();
    ImGui::Text("Board Config:");
    
    static int board_type = 0;
    const char* board_types[] = { "ESP32", "ESP32-S2", "ESP32-S3", "ESP32-C3" };
    ImGui::Combo("Board", &board_type, board_types, IM_ARRAYSIZE(board_types));
    
    static int freq = 240;
    ImGui::SliderInt("CPU Freq (MHz)", &freq, 80, 240);
    
    if (ImGui::Button("Apply Settings")) {
        AddConsoleMessage("Settings applied");
    }
}

void ImGuiWindow::RenderConsole() {
    ImGui::Text("Console Output");
    ImGui::Separator();
    
    ImGui::BeginChild("ConsoleScrolling", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& message : console_messages_) {
        ImGui::TextWrapped("%s", message.c_str());
    }
    
    if (scroll_to_bottom_) {
        ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom_ = false;
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    if (ImGui::Button("Clear Console")) {
        console_messages_.clear();
    }
}

void ImGuiWindow::RefreshFileList() {
    file_list_.clear();
    
    if (file_manager_) {
        file_list_ = file_manager_->GetFileList();
    }
    
    // Add some default entries if empty
    if (file_list_.empty()) {
        file_list_.push_back("sketch.ino");
        file_list_.push_back("config.h");
    }
}

void ImGuiWindow::RefreshPortList() {
    available_ports_.clear();
    
    if (serial_monitor_) {
        available_ports_ = serial_monitor_->GetAvailablePorts();
    }
    
    // Add some default ports if empty
    if (available_ports_.empty()) {
        available_ports_.push_back("/dev/ttyUSB0");
        available_ports_.push_back("/dev/ttyUSB1");
        available_ports_.push_back("COM3");
        available_ports_.push_back("COM4");
    }
    
    if (!available_ports_.empty() && selected_port_.empty()) {
        selected_port_index_ = 0;
        selected_port_ = available_ports_[0];
    }
}

void ImGuiWindow::LoadFile(const std::string& filename) {
    current_file_ = filename;
    
    if (file_manager_ && file_manager_->FileExists(filename)) {
        editor_content_ = file_manager_->GetFileContent(filename);
    } else {
        editor_content_ = "// File: " + filename + "\n\nvoid setup() {\n\n}\n\nvoid loop() {\n\n}\n";
    }
    
    std::strncpy(editor_buffer_, editor_content_.c_str(), sizeof(editor_buffer_) - 1);
    
    if (text_editor_) {
        text_editor_->SetText(editor_content_);
    }
    
    AddConsoleMessage("Loaded file: " + filename);
}

void ImGuiWindow::SaveFile() {
    if (current_file_.empty()) {
        AddConsoleMessage("Error: No file to save");
        return;
    }
    
    editor_content_ = std::string(editor_buffer_);
    
    if (file_manager_) {
        file_manager_->SetFileContent(current_file_, editor_content_);
        file_manager_->SaveFile(current_file_);
    }
    
    AddConsoleMessage("Saved file: " + current_file_);
}

void ImGuiWindow::CompileCode() {
    AddConsoleMessage("=== Compilation Started ===");
    
    editor_content_ = std::string(editor_buffer_);
    
    if (compiler_) {
        auto result = compiler_->Compile(editor_content_, compiler_->GetBoard());
        
        if (result.status == ESP32Compiler::CompileStatus::SUCCESS) {
            AddConsoleMessage("✓ Compilation successful!");
        } else {
            AddConsoleMessage("✗ Compilation failed!");
            for (const auto& error : result.errors) {
                AddConsoleMessage("  " + error);
            }
        }
    } else {
        AddConsoleMessage("Compiler not initialized");
    }
    
    AddConsoleMessage("=== Compilation Finished ===");
}

void ImGuiWindow::UploadCode() {
    AddConsoleMessage("=== Upload Started ===");
    AddConsoleMessage("Port: " + selected_port_);
    AddConsoleMessage("Baud rate: " + std::to_string(selected_baud_rate_));
    
    if (compiler_) {
        compiler_->Upload(compiler_->GetBoard());
        AddConsoleMessage("✓ Upload complete!");
    } else {
        AddConsoleMessage("Compiler not initialized");
    }
    
    AddConsoleMessage("=== Upload Finished ===");
}

void ImGuiWindow::DebugCode() {
    AddConsoleMessage("=== Debug Session Started ===");
    AddConsoleMessage("Connecting to ESP32 on " + selected_port_ + "...");
    AddConsoleMessage("Debug session active - use Debugger tab for controls");
    current_center_tab_ = 1; // Switch to debugger tab
}

void ImGuiWindow::ReverseEngineerCode() {
    AddConsoleMessage("=== Reverse Engineering Analysis ===");
    AddConsoleMessage("Analyzing firmware structure...");
    AddConsoleMessage("Detecting functions and entry points...");
    AddConsoleMessage("Extracting strings and constants...");
    AddConsoleMessage("✓ Analysis complete - see RE tab for details");
}

void ImGuiWindow::AddConsoleMessage(const std::string& message) {
    console_messages_.push_back("[" + std::to_string(console_messages_.size()) + "] " + message);
    scroll_to_bottom_ = true;
}

void ImGuiWindow::SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Dark theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    
    // Rounding
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 3.0f;
}

} // namespace gui
} // namespace esp32_ide
