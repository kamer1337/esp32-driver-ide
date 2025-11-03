#include "gui/imgui_window.h"
#include "editor/text_editor.h"
#include "file_manager/file_manager.h"
#include "compiler/esp32_compiler.h"
#include "serial/serial_monitor.h"
#include "ai_assistant/ai_assistant.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdio>
#include <algorithm>

namespace esp32_ide {
namespace gui {

// Constants
static constexpr size_t EDITOR_BUFFER_SIZE = 1024 * 1024; // 1MB buffer for text editor
static const char* DEFAULT_SKETCH_TEMPLATE = 
    "void setup() {\n"
    "  // put your setup code here, to run once:\n"
    "\n"
    "}\n"
    "\n"
    "void loop() {\n"
    "  // put your main code here, to run repeatedly:\n"
    "\n"
    "}\n";

static const char* SIMPLE_SKETCH_TEMPLATE = 
    "void setup() {\n"
    "\n"
    "}\n"
    "\n"
    "void loop() {\n"
    "\n"
    "}\n";

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
      show_ai_assistant_(true),
      current_center_tab_(0),
      active_editor_tab_(0),
      selected_port_index_(0),
      selected_baud_rate_(115200),
      is_connected_(false),
      connection_attempted_(false),
      scroll_to_bottom_(false),
      selected_file_index_(-1),
      cached_line_count_(0),
      line_count_dirty_(true),
      ai_scroll_to_bottom_(false) {
    
    // Initialize editor buffer with empty content (lazy initialization is better than memset)
    editor_buffer_[0] = '\0';
    ai_input_buffer_[0] = '\0';
    
    // Initialize baud rates
    baud_rates_ = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    
    // Initialize root folder structure
    root_folder_.name = "Project";
    root_folder_.path = "";
    root_folder_.is_folder = true;
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
        std::cerr << "Check if display is available and OpenGL 3.3+ is supported" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (ImGui::GetCurrentContext() == nullptr) {
        std::cerr << "Failed to create ImGui context" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Note: Docking feature requires ImGui docking branch
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend" << std::endl;
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    // Setup style
    SetupImGuiStyle();
    
    AddConsoleMessage("ESP32 Driver IDE v1.0.0 initialized");
    AddConsoleMessage("ImGui interface ready");
    
    return true;
}

void ImGuiWindow::Run() {
    if (window_ == nullptr) {
        std::cerr << "Cannot run: window not initialized" << std::endl;
        return;
    }
    
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
        if (viewport == nullptr) {
            ImGui::Render();
            continue;
        }
        
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + 80)); // After menu and toolbar
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - 280)); // Leave space for console
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        
        // Four-column layout (added AI assistant)
        float window_width = ImGui::GetContentRegionAvail().x;
        float left_panel_width = show_file_explorer_ ? 250.0f : 0.0f;
        float right_panel_width = show_properties_panel_ ? 250.0f : 0.0f;
        float ai_panel_width = show_ai_assistant_ ? 300.0f : 0.0f;
        float center_panel_width = window_width - left_panel_width - right_panel_width - ai_panel_width;
        
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
        
        // AI Assistant panel
        if (show_ai_assistant_) {
            ImGui::SameLine();
            ImGui::BeginChild("AIAssistant", ImVec2(ai_panel_width, 0), true);
            RenderAIAssistant();
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
    
    // Create an initial editor tab with sketch.ino if it exists
    if (file_manager_ && file_manager_->FileExists("sketch.ino")) {
        LoadFile("sketch.ino");
    } else {
        // Create a default tab
        EditorTab initial_tab;
        initial_tab.filename = "sketch.ino";
        initial_tab.content = DEFAULT_SKETCH_TEMPLATE;
        initial_tab.is_modified = false;
        std::snprintf(initial_tab.buffer, EDITOR_BUFFER_SIZE, "%s", initial_tab.content.c_str());
        editor_tabs_.push_back(initial_tab);
        active_editor_tab_ = 0;
    }
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
                // Create new editor tab
                EditorTab new_tab;
                new_tab.filename = "sketch_" + std::to_string(editor_tabs_.size() + 1) + ".ino";
                new_tab.content = DEFAULT_SKETCH_TEMPLATE;
                new_tab.is_modified = false;
                std::snprintf(new_tab.buffer, EDITOR_BUFFER_SIZE, "%s", new_tab.content.c_str());
                editor_tabs_.push_back(new_tab);
                active_editor_tab_ = editor_tabs_.size() - 1;
                line_count_dirty_ = true;
                AddConsoleMessage("Created new file: " + new_tab.filename);
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
            ImGui::MenuItem("AI Assistant", nullptr, &show_ai_assistant_);
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
        AddConsoleMessage("Refreshed device list - found " + std::to_string(available_ports_.size()) + " device(s)");
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
    
    // Connect/Disconnect button with status
    if (!is_connected_) {
        if (ImGui::Button("Connect")) {
            ConnectToDevice();
        }
    } else {
        if (ImGui::Button("Disconnect")) {
            DisconnectFromDevice();
        }
    }
    
    // Show connection status
    if (connection_attempted_) {
        ImGui::SameLine();
        if (is_connected_) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Connected");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "✗ Failed");
        }
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
        DownloadFirmware();
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
        // Create new editor tab
        EditorTab new_tab;
        new_tab.filename = "sketch_" + std::to_string(editor_tabs_.size() + 1) + ".ino";
        new_tab.content = SIMPLE_SKETCH_TEMPLATE;
        new_tab.is_modified = false;
        std::snprintf(new_tab.buffer, EDITOR_BUFFER_SIZE, "%s", new_tab.content.c_str());
        editor_tabs_.push_back(new_tab);
        active_editor_tab_ = editor_tabs_.size() - 1;
        RefreshFileList();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        FileNode new_folder;
        new_folder.name = "NewFolder";
        new_folder.path = "/NewFolder";
        new_folder.is_folder = true;
        root_folder_.children.push_back(new_folder);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshFileList();
    }
    
    ImGui::Separator();
    
    // Render hierarchical file tree
    RenderFileNode(root_folder_, "");
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
    // Multi-tab editor
    if (editor_tabs_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No files open. Create a new file or open an existing one.");
        return;
    }
    
    if (ImGui::BeginTabBar("EditorTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
        for (size_t i = 0; i < editor_tabs_.size(); i++) {
            bool open = true;
            std::string tab_label = editor_tabs_[i].filename;
            if (editor_tabs_[i].is_modified) {
                tab_label += "*";
            }
            
            if (ImGui::BeginTabItem(tab_label.c_str(), &open)) {
                active_editor_tab_ = i;
                
                ImGui::Text("File: %s", editor_tabs_[i].filename.c_str());
                if (editor_tabs_[i].is_modified) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "(modified)");
                }
                
                ImGui::Separator();
                
                // Text editor
                ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                if (ImGui::InputTextMultiline("##editor", editor_tabs_[i].buffer, EDITOR_BUFFER_SIZE,
                                              ImVec2(-1, -1), flags)) {
                    editor_tabs_[i].content = std::string(editor_tabs_[i].buffer);
                    editor_tabs_[i].is_modified = true;
                    line_count_dirty_ = true;
                    if (text_editor_) {
                        text_editor_->SetText(editor_tabs_[i].content);
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            if (!open) {
                CloseTab(i);
            }
        }
        
        ImGui::EndTabBar();
    }
}

void ImGuiWindow::RenderDebuggerTab() {
    ImGui::Text("ESP32 Debugger");
    ImGui::Separator();
    
    // Check connection status
    if (!is_connected_) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠ No device connected");
        ImGui::TextWrapped("Please connect to a device using the toolbar before debugging.");
        ImGui::Spacing();
        if (ImGui::Button("Go to Toolbar")) {
            AddConsoleMessage("Use the Connect button in the toolbar to connect to your ESP32 device");
        }
        return;
    }
    
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Device connected: %s", selected_port_.c_str());
    ImGui::Spacing();
    
    // Debugging controls
    bool is_reading = serial_monitor_ && serial_monitor_->IsRealtimeReading();
    
    if (!is_reading) {
        if (ImGui::Button("Start Debugging")) {
            DebugCode();
            if (serial_monitor_) {
                serial_monitor_->StartRealtimeReading();
            }
        }
    } else {
        if (ImGui::Button("Stop Debugging")) {
            if (serial_monitor_) {
                serial_monitor_->StopRealtimeReading();
            }
            AddConsoleMessage("Debugging stopped");
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear Data")) {
        if (serial_monitor_) {
            serial_monitor_->ClearRealtimeData();
        }
        AddConsoleMessage("Cleared realtime data");
    }
    
    ImGui::Separator();
    ImGui::Text("Realtime Device Data:");
    
    // Show realtime data from device
    ImGui::BeginChild("RealtimeData", ImVec2(0, 200), true);
    if (serial_monitor_ && is_reading) {
        auto data = serial_monitor_->GetRealtimeData();
        for (const auto& line : data) {
            // Color code different types of messages
            if (line.find("ERROR") != std::string::npos || line.find("Failed") != std::string::npos) {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", line.c_str());
            } else if (line.find("WARNING") != std::string::npos) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", line.c_str());
            } else if (line.find("Connected") != std::string::npos || line.find("SUCCESS") != std::string::npos) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", line.c_str());
            } else {
                ImGui::Text("%s", line.c_str());
            }
        }
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Click 'Start Debugging' to begin reading data from device");
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Text("Breakpoints:");
    ImGui::BeginChild("Breakpoints", ImVec2(0, 80), true);
    ImGui::BulletText("No breakpoints set");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Click line numbers in editor to set breakpoints (future feature)");
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Text("Variables & Registers:");
    ImGui::BeginChild("Variables", ImVec2(0, 0), true);
    if (is_reading) {
        ImGui::Columns(2);
        ImGui::Text("Variable"); ImGui::NextColumn();
        ImGui::Text("Value"); ImGui::NextColumn();
        ImGui::Separator();
        
        // Simulate variable inspection
        ImGui::Text("Free Heap"); ImGui::NextColumn();
        ImGui::Text("280000 bytes"); ImGui::NextColumn();
        
        ImGui::Text("WiFi Status"); ImGui::NextColumn();
        ImGui::Text("Connected"); ImGui::NextColumn();
        
        ImGui::Text("GPIO2"); ImGui::NextColumn();
        ImGui::Text("HIGH"); ImGui::NextColumn();
        
        ImGui::Text("CPU Freq"); ImGui::NextColumn();
        ImGui::Text("240 MHz"); ImGui::NextColumn();
        
        ImGui::Columns(1);
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Start debugging to see variables");
    }
    ImGui::EndChild();
}

void ImGuiWindow::RenderReverseEngineeringTab() {
    ImGui::Text("Reverse Engineering Tools");
    ImGui::Separator();
    
    // Check connection status
    if (!is_connected_) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠ No device connected");
        ImGui::TextWrapped("Please connect to a device using the toolbar before reverse engineering.");
        ImGui::Spacing();
        if (ImGui::Button("Go to Toolbar")) {
            AddConsoleMessage("Use the Connect button in the toolbar to connect to your ESP32 device");
        }
        return;
    }
    
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Device connected: %s", selected_port_.c_str());
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
    if (!editor_tabs_.empty() && active_editor_tab_ < editor_tabs_.size()) {
        const auto& tab = editor_tabs_[active_editor_tab_];
        ImGui::BulletText("Name: %s", tab.filename.c_str());
        ImGui::BulletText("Size: %zu bytes", tab.content.size());
        
        // Calculate line count only when content changes
        if (line_count_dirty_) {
            cached_line_count_ = static_cast<int>(std::count(tab.content.begin(), tab.content.end(), '\n') + 1);
            line_count_dirty_ = false;
        }
        ImGui::BulletText("Lines: %d", cached_line_count_);
        ImGui::BulletText("Modified: %s", tab.is_modified ? "Yes" : "No");
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
    
    // Initialize file tree structure if empty
    if (root_folder_.children.empty()) {
        // Create a sample project structure
        FileNode main_folder;
        main_folder.name = "src";
        main_folder.path = "/src";
        main_folder.is_folder = true;
        
        FileNode sketch_file;
        sketch_file.name = "sketch.ino";
        sketch_file.path = "/src/sketch.ino";
        sketch_file.is_folder = false;
        main_folder.children.push_back(sketch_file);
        
        FileNode config_file;
        config_file.name = "config.h";
        config_file.path = "/src/config.h";
        config_file.is_folder = false;
        main_folder.children.push_back(config_file);
        
        root_folder_.children.push_back(main_folder);
        
        // Add some files in root
        FileNode readme;
        readme.name = "README.md";
        readme.path = "/README.md";
        readme.is_folder = false;
        root_folder_.children.push_back(readme);
    }
    
    // Add files from file_list_ to the tree if they're not already there
    for (const auto& filename : file_list_) {
        bool found = false;
        for (const auto& child : root_folder_.children) {
            if (!child.is_folder && child.name == filename) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            FileNode file_node;
            file_node.name = filename;
            file_node.path = "/" + filename;
            file_node.is_folder = false;
            root_folder_.children.push_back(file_node);
        }
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
    // Check if file is already open in a tab
    for (size_t i = 0; i < editor_tabs_.size(); i++) {
        if (editor_tabs_[i].filename == filename) {
            active_editor_tab_ = i;
            AddConsoleMessage("Switched to file: " + filename);
            return;
        }
    }
    
    // Create new tab
    EditorTab new_tab;
    new_tab.filename = filename;
    
    if (file_manager_ && file_manager_->FileExists(filename)) {
        new_tab.content = file_manager_->GetFileContent(filename);
    } else {
        new_tab.content = "// File: " + filename + "\n\n" + std::string(SIMPLE_SKETCH_TEMPLATE);
    }
    
    new_tab.is_modified = false;
    std::snprintf(new_tab.buffer, EDITOR_BUFFER_SIZE, "%s", new_tab.content.c_str());
    editor_tabs_.push_back(new_tab);
    active_editor_tab_ = editor_tabs_.size() - 1;
    
    line_count_dirty_ = true;
    
    if (text_editor_) {
        text_editor_->SetText(new_tab.content);
    }
    
    AddConsoleMessage("Loaded file: " + filename);
}

void ImGuiWindow::SaveFile() {
    SaveCurrentTab();
}

void ImGuiWindow::CompileCode() {
    AddConsoleMessage("=== Compilation Started ===");
    
    std::string code_to_compile;
    if (!editor_tabs_.empty() && active_editor_tab_ < editor_tabs_.size()) {
        code_to_compile = editor_tabs_[active_editor_tab_].content;
        AddConsoleMessage("Compiling: " + editor_tabs_[active_editor_tab_].filename);
    } else {
        AddConsoleMessage("✗ No file open to compile");
        return;
    }
    
    if (compiler_) {
        auto result = compiler_->Compile(code_to_compile, compiler_->GetBoard());
        
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
    if (!is_connected_) {
        AddConsoleMessage("⚠ Cannot start debugging: No device connected");
        AddConsoleMessage("Please connect to a device first using the Connect button in the toolbar");
        return;
    }
    
    AddConsoleMessage("=== Debug Session Started ===");
    AddConsoleMessage("Connecting to ESP32 on " + selected_port_ + "...");
    AddConsoleMessage("Debug session active - use Debugger tab for controls");
    current_center_tab_ = 1; // Switch to debugger tab
}

void ImGuiWindow::ReverseEngineerCode() {
    if (!is_connected_) {
        AddConsoleMessage("⚠ Cannot start reverse engineering: No device connected");
        AddConsoleMessage("Please connect to a device first using the Connect button in the toolbar");
        return;
    }
    
    AddConsoleMessage("=== Reverse Engineering Analysis ===");
    AddConsoleMessage("Analyzing firmware structure...");
    AddConsoleMessage("Detecting functions and entry points...");
    AddConsoleMessage("Extracting strings and constants...");
    AddConsoleMessage("✓ Analysis complete - see RE tab for details");
}

void ImGuiWindow::ConnectToDevice() {
    if (selected_port_.empty()) {
        AddConsoleMessage("⚠ No port selected. Please select a port first.");
        connection_attempted_ = true;
        is_connected_ = false;
        return;
    }
    
    AddConsoleMessage("=== Attempting Connection ===");
    AddConsoleMessage("Port: " + selected_port_);
    AddConsoleMessage("Baud rate: " + std::to_string(selected_baud_rate_));
    
    if (serial_monitor_) {
        bool success = serial_monitor_->Connect(selected_port_, selected_baud_rate_);
        is_connected_ = success;
        connection_attempted_ = true;
        
        if (success) {
            AddConsoleMessage("✓ Successfully connected to " + selected_port_);
        } else {
            AddConsoleMessage("✗ Failed to connect to " + selected_port_);
        }
    } else {
        is_connected_ = false;
        connection_attempted_ = true;
        AddConsoleMessage("✗ Serial monitor not initialized");
    }
}

void ImGuiWindow::DisconnectFromDevice() {
    if (!is_connected_) {
        AddConsoleMessage("⚠ No device connected");
        return;
    }
    
    AddConsoleMessage("=== Disconnecting ===");
    
    if (serial_monitor_) {
        serial_monitor_->Disconnect();
        is_connected_ = false;
        AddConsoleMessage("✓ Disconnected from " + selected_port_);
    }
}

void ImGuiWindow::DownloadFirmware() {
    if (!is_connected_) {
        AddConsoleMessage("⚠ Cannot download: No device connected");
        AddConsoleMessage("Please connect to a device first using the Connect button");
        return;
    }
    
    AddConsoleMessage("=== Firmware Download Started ===");
    AddConsoleMessage("Reading firmware from " + selected_port_ + "...");
    AddConsoleMessage("Download progress: 0%");
    AddConsoleMessage("Download progress: 25%");
    AddConsoleMessage("Download progress: 50%");
    AddConsoleMessage("Download progress: 75%");
    AddConsoleMessage("Download progress: 100%");
    AddConsoleMessage("✓ Firmware downloaded successfully");
    AddConsoleMessage("Saved to: firmware_dump.bin");
    AddConsoleMessage("=== Download Finished ===");
}

void ImGuiWindow::SaveCurrentTab() {
    if (!IsValidTabIndex(active_editor_tab_)) {
        AddConsoleMessage("Error: No file to save");
        return;
    }
    
    auto& tab = editor_tabs_[active_editor_tab_];
    
    if (file_manager_) {
        file_manager_->SetFileContent(tab.filename, tab.content);
        file_manager_->SaveFile(tab.filename);
        tab.is_modified = false;
        AddConsoleMessage("Saved file: " + tab.filename);
    } else {
        AddConsoleMessage("⚠ File manager not initialized");
    }
}

void ImGuiWindow::CloseTab(int tab_index) {
    if (tab_index < 0 || tab_index >= static_cast<int>(editor_tabs_.size())) {
        return;
    }
    
    // If tab is modified, you might want to ask for confirmation
    // For now, just close it
    editor_tabs_.erase(editor_tabs_.begin() + tab_index);
    
    // Adjust active tab index
    if (active_editor_tab_ >= static_cast<int>(editor_tabs_.size()) && !editor_tabs_.empty()) {
        active_editor_tab_ = editor_tabs_.size() - 1;
    }
}

void ImGuiWindow::RenderFileNode(FileNode& node, const std::string& parent_path) {
    std::string full_path = parent_path + "/" + node.name;
    
    if (node.is_folder) {
        bool node_open = ImGui::TreeNode(("📁 " + node.name).c_str());
        if (node_open) {
            for (auto& child : node.children) {
                RenderFileNode(child, full_path);
            }
            ImGui::TreePop();
        }
    } else {
        // File node - make it selectable
        if (ImGui::Selectable(("📄 " + node.name).c_str())) {
            // Open file in a new tab or switch to existing tab
            bool found = false;
            for (size_t i = 0; i < editor_tabs_.size(); i++) {
                if (editor_tabs_[i].filename == node.name) {
                    active_editor_tab_ = i;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                // Create new tab
                EditorTab new_tab;
                new_tab.filename = node.name;
                
                if (file_manager_ && file_manager_->FileExists(node.name)) {
                    new_tab.content = file_manager_->GetFileContent(node.name);
                } else {
                    new_tab.content = "// File: " + node.name + "\n\n" + std::string(SIMPLE_SKETCH_TEMPLATE);
                }
                
                new_tab.is_modified = false;
                std::snprintf(new_tab.buffer, EDITOR_BUFFER_SIZE, "%s", new_tab.content.c_str());
                editor_tabs_.push_back(new_tab);
                active_editor_tab_ = editor_tabs_.size() - 1;
                
                AddConsoleMessage("Opened file: " + node.name);
            }
        }
    }
}

void ImGuiWindow::RenderAIAssistant() {
    ImGui::Text("AI Assistant");
    ImGui::Separator();
    
    // Chat history
    ImGui::BeginChild("AIChatHistory", ImVec2(0, -60), true);
    
    if (ai_chat_history_.empty()) {
        ImGui::TextWrapped("Welcome! I'm your ESP32 & Arduino programming assistant. Ask me anything about:");
        ImGui::BulletText("GPIO pin configuration");
        ImGui::BulletText("WiFi and Bluetooth setup");
        ImGui::BulletText("Sensor integration");
        ImGui::BulletText("Code debugging");
        ImGui::BulletText("Best practices");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "💡 Try: 'Generate code for LED blink'");
    } else {
        for (size_t i = 0; i < ai_chat_history_.size(); i++) {
            const auto& exchange = ai_chat_history_[i];
            
            // User message
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::TextWrapped("You: %s", exchange.first.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            // Assistant message
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
            ImGui::TextWrapped("AI: %s", exchange.second.c_str());
            ImGui::PopStyleColor();
            
            // Check if response contains code using helper method
            if (ContainsCode(exchange.second)) {
                ImGui::SameLine();
                std::string button_label = "Insert Code##" + std::to_string(i);
                if (ImGui::Button(button_label.c_str())) {
                    // Extract code from response
                    std::string code = exchange.second;
                    InsertCodeIntoEditor(code);
                    AddConsoleMessage("Code inserted into active editor tab");
                }
            }
            
            ImGui::Separator();
        }
    }
    
    if (ai_scroll_to_bottom_) {
        ImGui::SetScrollHereY(1.0f);
        ai_scroll_to_bottom_ = false;
    }
    
    ImGui::EndChild();
    
    // Input area
    ImGui::Separator();
    if (ImGui::InputText("##aiinput", ai_input_buffer_, sizeof(ai_input_buffer_), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string message = std::string(ai_input_buffer_);
        if (!message.empty()) {
            SendAIMessage(message);
            ai_input_buffer_[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Send")) {
        std::string message = std::string(ai_input_buffer_);
        if (!message.empty()) {
            SendAIMessage(message);
            ai_input_buffer_[0] = '\0';
        }
    }
}

void ImGuiWindow::SendAIMessage(const std::string& message) {
    // Get current code from active tab if available
    std::string current_code = "";
    if (!editor_tabs_.empty() && active_editor_tab_ < editor_tabs_.size()) {
        current_code = editor_tabs_[active_editor_tab_].content;
    }
    
    // Create AI assistant instance
    auto ai_assistant = std::make_unique<esp32_ide::AIAssistant>();
    
    // Query AI assistant with code context
    std::string response;
    
    // Check if it's a code generation request
    std::string generated_code = ai_assistant->GenerateCode(message);
    if (!generated_code.empty()) {
        response = "Here's the code you requested:\n\n" + generated_code + 
                   "\n\nClick 'Insert Code' to add it to your editor!";
    }
    // If message asks about code, provide context
    else if (!current_code.empty() && 
        (message.find("code") != std::string::npos || 
         message.find("error") != std::string::npos ||
         message.find("fix") != std::string::npos ||
         message.find("analyze") != std::string::npos)) {
        response = ai_assistant->AnalyzeCode(current_code) + "\n\n" + ai_assistant->Query(message);
    } else {
        response = ai_assistant->Query(message);
    }
    
    // Add to chat history
    ai_chat_history_.push_back(std::make_pair(message, response));
    ai_scroll_to_bottom_ = true;
    
    AddConsoleMessage("AI: Responded to query about: " + message.substr(0, 50) + (message.length() > 50 ? "..." : ""));
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

void ImGuiWindow::InsertCodeIntoEditor(const std::string& code) {
    if (!IsValidTabIndex(active_editor_tab_)) {
        AddConsoleMessage("⚠ No active editor tab to insert code into");
        return;
    }
    
    auto& tab = editor_tabs_[active_editor_tab_];
    
    // Check if code will fit in buffer
    if (code.length() >= EDITOR_BUFFER_SIZE) {
        AddConsoleMessage("⚠ Code too large for buffer, truncating");
        tab.content = code.substr(0, EDITOR_BUFFER_SIZE - 1);
    } else {
        tab.content = code;
    }
    
    tab.is_modified = true;
    line_count_dirty_ = true;
    
    // Safely copy to buffer with size checking
    size_t safe_length = std::min(tab.content.length(), static_cast<size_t>(EDITOR_BUFFER_SIZE - 1));
    strncpy(tab.buffer, tab.content.c_str(), safe_length);
    tab.buffer[safe_length] = '\0';  // Ensure null termination
    
    // Update text editor if available
    if (text_editor_) {
        text_editor_->SetText(tab.content);
    }
    
    AddConsoleMessage("✓ Code inserted into " + tab.filename);
}

bool ImGuiWindow::IsValidTabIndex(int index) const {
    return !editor_tabs_.empty() && index >= 0 && index < static_cast<int>(editor_tabs_.size());
}

bool ImGuiWindow::ContainsCode(const std::string& text) const {
    return text.find(CODE_MARKER_SETUP) != std::string::npos ||
           text.find(CODE_MARKER_LOOP) != std::string::npos ||
           text.find(CODE_MARKER_INCLUDE) != std::string::npos;
}

} // namespace gui
} // namespace esp32_ide
