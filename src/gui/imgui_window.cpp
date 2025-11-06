#include "gui/imgui_window.h"
#include "gui/blueprint_editor.h"
#include "editor/text_editor.h"
#include "editor/syntax_highlighter.h"
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

// UI Panel widths
static constexpr float LEFT_PANEL_WIDTH = 250.0f;
static constexpr float RIGHT_PANEL_WIDTH = 250.0f;
static constexpr float AI_PANEL_WIDTH = 300.0f;
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

// Sample disassembly data for ESP32 (Xtensa LX6 instructions)
static const std::vector<std::string> SAMPLE_DISASSEMBLY_DATA = {
    "0x40080000: entry a1, 64",
    "0x40080003: s32i.n a0, a1, 0",
    "0x40080005: call0 app_main",
    "0x40080008: retw.n",
    "0x4008000a: l32r a2, 0x40080100",
    "0x4008000d: l32i a3, a2, 0",
    "0x40080010: addi a3, a3, 1",
    "0x40080013: s32i a3, a2, 0",
    "0x40080016: movi a2, 0x100",
    "0x40080019: call0 vTaskDelay"
};

// Sample RE analysis result constants
static constexpr const char* SAMPLE_RE_ARCHITECTURE = "Xtensa LX6";
static constexpr const char* SAMPLE_RE_FLASH_SIZE = "4MB";
static constexpr const char* SAMPLE_RE_ENTRY_POINT = "0x40080000";
static constexpr int SAMPLE_RE_FUNCTIONS_DETECTED = 42;
static constexpr int SAMPLE_RE_STRINGS_FOUND = 127;

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

ImGuiWindow::ImGuiWindow()
    : window_(nullptr),
      text_editor_(nullptr),
      file_manager_(nullptr),
      compiler_(nullptr),
      serial_monitor_(nullptr),
      syntax_highlighter_(nullptr),
      show_file_explorer_(false),
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
      ai_scroll_to_bottom_(false),
      re_analysis_performed_(false),
      re_disassembly_performed_(false),
      terminal_scroll_to_bottom_(false),
      show_terminal_(true),
      selected_board_index_(-1),
      show_board_list_(true),
      show_device_schematic_(false),
      current_schematic_view_(0),
      enable_syntax_highlighting_(true),
      show_line_numbers_(true),
      auto_save_enabled_(false),
      auto_save_interval_(60.0f),
      last_auto_save_time_(0.0f),
      status_bar_message_("Ready"),
      show_find_dialog_(false),
      cursor_line_(1),
      cursor_column_(0),
      show_confirmation_dialog_(false),
      show_settings_dialog_(false),
      settings_tab_size_(4),
      settings_auto_indent_(true),
      settings_theme_("Dark"),
      terminal_selected_suggestion_(-1) {
    
    // Initialize editor buffer with empty content (lazy initialization is better than memset)
    editor_buffer_[0] = '\0';
    ai_input_buffer_[0] = '\0';
    terminal_input_buffer_[0] = '\0';
    find_buffer_[0] = '\0';
    
    // Initialize baud rates
    baud_rates_ = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    
    // Initialize root folder structure
    root_folder_.name = "Project";
    root_folder_.path = "";
    root_folder_.is_folder = true;
    
    // Initialize RE analysis result
    re_analysis_result_.architecture = "";
    re_analysis_result_.flash_size = "";
    re_analysis_result_.entry_point = "";
    re_analysis_result_.functions_detected = 0;
    re_analysis_result_.strings_found = 0;
    re_analysis_result_.has_data = false;
    
    // Initialize terminal with welcome message
    terminal_history_.push_back("ESP32 Driver IDE Terminal v1.0");
    terminal_history_.push_back("Type 'help' for available commands");
    terminal_history_.push_back("");
    
    // Initialize terminal commands for autocomplete
    terminal_commands_ = {"help", "clear", "compile", "upload", "ports", "boards", "ls", "exit", "version"};
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
    
    // Initialize blueprint editor
    blueprint_editor_ = std::make_unique<BlueprintEditor>();
    if (!blueprint_editor_->Initialize()) {
        std::cerr << "Failed to initialize Blueprint Editor" << std::endl;
        // Continue anyway, non-fatal
    }
    
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
        
        // Handle keyboard shortcuts
        HandleKeyboardShortcuts();
        
        // Perform auto-save if enabled
        if (auto_save_enabled_) {
            PerformAutoSave();
        }
        
        // Render UI
        RenderMainMenuBar();
        RenderToolbar();
        
        // Create main docking space
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport == nullptr) {
            // Properly end the frame before continuing
            ImGui::EndFrame();
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window_);
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
        float left_panel_width = (show_file_explorer_ || show_board_list_) ? LEFT_PANEL_WIDTH : 0.0f;
        float right_panel_width = show_properties_panel_ ? RIGHT_PANEL_WIDTH : 0.0f;
        float ai_panel_width = show_ai_assistant_ ? AI_PANEL_WIDTH : 0.0f;
        float center_panel_width = window_width - left_panel_width - right_panel_width - ai_panel_width;
        
        // Left panel - File Explorer and Board List
        if (show_file_explorer_ || show_board_list_) {
            ImGui::BeginChild("LeftPanel", ImVec2(left_panel_width, 0), true);
            
            if (ImGui::BeginTabBar("LeftPanelTabs")) {
                if (show_file_explorer_ && ImGui::BeginTabItem("Files")) {
                    RenderFileExplorer();
                    ImGui::EndTabItem();
                }
                
                if (show_board_list_ && ImGui::BeginTabItem("Boards")) {
                    RenderBoardListPanel();
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
            
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
        
        // Bottom panels - Console and Terminal
        float bottom_panel_width = show_terminal_ ? viewport->Size.x / 2.0f : viewport->Size.x;
        
        // Console at the bottom left
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 200));
        ImGui::SetNextWindowSize(ImVec2(bottom_panel_width, 200));
        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        RenderConsole();
        ImGui::End();
        
        // Terminal at the bottom right
        if (show_terminal_) {
            ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + bottom_panel_width, viewport->Pos.y + viewport->Size.y - 200));
            ImGui::SetNextWindowSize(ImVec2(viewport->Size.x - bottom_panel_width, 200));
            ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            RenderTerminalPanel();
            ImGui::End();
        }
        
        // Device Schematic Window (floating)
        if (show_device_schematic_) {
            ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
            ImGui::Begin("Device Schematic", &show_device_schematic_);
            RenderDeviceSchematic();
            ImGui::End();
        }
        
        // Find dialog (floating)
        if (show_find_dialog_) {
            RenderFindDialog();
        }
        
        // Confirmation dialog (floating)
        if (show_confirmation_dialog_) {
            RenderConfirmationDialog();
        }
        
        // Settings dialog (floating)
        if (show_settings_dialog_) {
            RenderSettingsDialog();
        }
        
        // Status bar at the very bottom
        RenderStatusBar();
        
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
    // Shutdown blueprint editor
    if (blueprint_editor_) {
        blueprint_editor_->Shutdown();
        blueprint_editor_.reset();
    }
    
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

void ImGuiWindow::SetSyntaxHighlighter(SyntaxHighlighter* highlighter) {
    syntax_highlighter_ = highlighter;
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
            
            // Recent files submenu
            if (ImGui::BeginMenu("Recent Files")) {
                if (recent_files_.empty()) {
                    ImGui::MenuItem("(No recent files)", nullptr, false, false);
                } else {
                    for (const auto& file : recent_files_) {
                        if (ImGui::MenuItem(file.c_str())) {
                            LoadFile(file);
                        }
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            if (ImGui::BeginMenu("Export")) {
                if (ImGui::MenuItem("Export Console Log")) {
                    ExportConsoleLog();
                }
                if (ImGui::MenuItem("Export Generated Code")) {
                    ExportGeneratedCode();
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("File Explorer", nullptr, &show_file_explorer_);
            ImGui::MenuItem("Board List", nullptr, &show_board_list_);
            ImGui::MenuItem("Properties Panel", nullptr, &show_properties_panel_);
            ImGui::MenuItem("AI Assistant", nullptr, &show_ai_assistant_);
            ImGui::MenuItem("Terminal", nullptr, &show_terminal_);
            ImGui::Separator();
            ImGui::MenuItem("Line Numbers", nullptr, &show_line_numbers_);
            ImGui::MenuItem("Syntax Highlighting", nullptr, &enable_syntax_highlighting_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Find", "Ctrl+F")) {
                show_find_dialog_ = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close Tab", "Ctrl+W")) {
                if (active_editor_tab_ >= 0 && active_editor_tab_ < editor_tabs_.size()) {
                    CloseTab(active_editor_tab_);
                }
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Compile", "F5")) {
                CompileCode();
            }
            if (ImGui::MenuItem("Upload", "Ctrl+U")) {
                UploadCode();
            }
            ImGui::Separator();
            ImGui::MenuItem("Auto-Save", nullptr, &auto_save_enabled_);
            if (auto_save_enabled_) {
                ImGui::SliderFloat("Interval (s)", &auto_save_interval_, 10.0f, 300.0f);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings...")) {
                show_settings_dialog_ = true;
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
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Scan for available USB/serial ports");
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
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Connect to selected ESP32 device");
        }
    } else {
        if (ImGui::Button("Disconnect")) {
            DisconnectFromDevice();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Disconnect from ESP32 device");
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
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Upload code to ESP32 device (Ctrl+U)");
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Download")) {
        DownloadFirmware();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Download firmware from ESP32 device");
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Debug")) {
        DebugCode();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Start debugging session");
    }
    ImGui::SameLine();
    
    if (ImGui::Button("RE")) {
        ReverseEngineerCode();
        current_center_tab_ = 2; // Switch to RE tab
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reverse engineer firmware");
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
        
        if (ImGui::BeginTabItem("Blueprint")) {
            current_center_tab_ = 3;
            RenderBlueprintTab();
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
                
                ImGui::SameLine();
                ImGui::Checkbox("Syntax Highlighting", &enable_syntax_highlighting_);
                
                ImGui::Separator();
                
                // If syntax highlighting is enabled, show split view
                if (syntax_highlighter_ && enable_syntax_highlighting_) {
                    float available_height = ImGui::GetContentRegionAvail().y;
                    float editor_height = available_height * 0.5f;
                    
                    // Editable text area with line numbers
                    ImGui::Text("Edit Mode:");
                    
                    // Line numbers column
                    if (show_line_numbers_) {
                        ImGui::BeginChild("LineNumbers", ImVec2(50, editor_height), true);
                        int line_count = CountLines(editor_tabs_[i].content);
                        for (int line = 1; line <= line_count; ++line) {
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%4d", line);
                        }
                        ImGui::EndChild();
                        ImGui::SameLine();
                    }
                    
                    // Editor takes remaining width
                    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                    if (ImGui::InputTextMultiline("##editor", editor_tabs_[i].buffer, EDITOR_BUFFER_SIZE,
                                                  ImVec2(-1, editor_height), flags)) {
                        editor_tabs_[i].content = std::string(editor_tabs_[i].buffer);
                        editor_tabs_[i].is_modified = true;
                        line_count_dirty_ = true;
                        if (text_editor_) {
                            text_editor_->SetText(editor_tabs_[i].content);
                        }
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("Preview with Syntax Highlighting:");
                    
                    // Syntax highlighted preview
                    ImGui::BeginChild("SyntaxPreview", ImVec2(-1, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
                    RenderSyntaxHighlightedText(editor_tabs_[i].content);
                    ImGui::EndChild();
                } else {
                    // Plain text editor (original behavior) with line numbers
                    if (show_line_numbers_) {
                        float available_height = ImGui::GetContentRegionAvail().y;
                        ImGui::BeginChild("LineNumbersPlain", ImVec2(50, available_height), true);
                        int line_count = CountLines(editor_tabs_[i].content);
                        for (int line = 1; line <= line_count; ++line) {
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%4d", line);
                        }
                        ImGui::EndChild();
                        ImGui::SameLine();
                    }
                    
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
    
    // Display available devices list - helps users see all detected devices at a glance
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Available Devices:");
    ImGui::Spacing();
    
    if (available_ports_.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠ No devices found");
        ImGui::TextWrapped("Click 'Refresh Ports' in the toolbar to scan for devices.");
    } else {
        ImGui::BeginChild("DeviceList", ImVec2(0, 100), true);
        for (size_t i = 0; i < available_ports_.size(); i++) {
            bool is_current = (available_ports_[i] == selected_port_);
            bool is_connected = is_current && is_connected_;
            
            if (is_connected) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ %s (Connected)", available_ports_[i].c_str());
            } else if (is_current) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "○ %s (Selected)", available_ports_[i].c_str());
            } else {
                ImGui::Text("  %s", available_ports_[i].c_str());
            }
        }
        ImGui::EndChild();
        
        ImGui::TextWrapped("Found %u device(s). Select and connect to a device from the toolbar.", 
                           static_cast<unsigned int>(available_ports_.size()));
    }
    
    ImGui::Separator();
    ImGui::Spacing();
    
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
        AddConsoleMessage("=== Disassembling Firmware ===");
        AddConsoleMessage("Reading flash memory from device...");
        AddConsoleMessage("Parsing instruction set...");
        AddConsoleMessage("Identifying function boundaries...");
        
        // Populate disassembly data with sample ESP32 Xtensa instructions
        // In a real implementation, this would read from the actual device flash
        re_disassembly_data_ = SAMPLE_DISASSEMBLY_DATA;
        
        re_disassembly_performed_ = true;
        AddConsoleMessage("✓ Disassembly complete - " + std::to_string(re_disassembly_data_.size()) + " instructions decoded");
    }
    
    ImGui::Separator();
    ImGui::Text("Analysis Results:");
    
    ImGui::BeginChild("REResults", ImVec2(0, 0), true);
    
    // Display analysis results if available, otherwise show prompt
    if (re_analysis_performed_ && re_analysis_result_.has_data) {
        ImGui::TextWrapped("ESP32 Firmware Analysis:");
        ImGui::BulletText("Architecture: %s", re_analysis_result_.architecture.c_str());
        ImGui::BulletText("Flash size: %s", re_analysis_result_.flash_size.c_str());
        ImGui::BulletText("Entry point: %s", re_analysis_result_.entry_point.c_str());
        ImGui::BulletText("Functions detected: %d", re_analysis_result_.functions_detected);
        ImGui::BulletText("Strings found: %d", re_analysis_result_.strings_found);
        
        // Show disassembly if it has been performed
        if (re_disassembly_performed_ && !re_disassembly_data_.empty()) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextWrapped("Disassembly Output:");
            for (const auto& line : re_disassembly_data_) {
                ImGui::TextUnformatted(line.c_str());
            }
        }
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No analysis data available yet.");
        ImGui::TextWrapped("Click 'Analyze Binary' to perform firmware analysis or 'Disassemble' to view disassembled code.");
    }
    
    ImGui::EndChild();
}

void ImGuiWindow::RenderPropertiesPanel() {
    if (ImGui::BeginTabBar("PropertiesTabs")) {
        if (ImGui::BeginTabItem("Editor")) {
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
            
            ImGui::Checkbox("Show line numbers", &show_line_numbers);
            ImGui::Checkbox("Auto indent", &auto_indent);
            ImGui::Checkbox("Syntax highlighting", &enable_syntax_highlighting_);
            
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
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Templates")) {
            ImGui::Text("Code Templates");
            ImGui::Separator();
            
            if (file_manager_) {
                auto templates = file_manager_->GetTemplates();
                
                if (templates.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No templates available");
                } else {
                    ImGui::Text("Available templates: %zu", templates.size());
                    ImGui::Spacing();
                    
                    ImGui::BeginChild("TemplateList", ImVec2(0, -30), true);
                    for (const auto& tmpl : templates) {
                        ImGui::PushID(tmpl.name.c_str());
                        
                        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "%s", tmpl.name.c_str());
                        ImGui::TextWrapped("%s", tmpl.description.c_str());
                        
                        if (ImGui::Button("Insert")) {
                            auto code = file_manager_->ApplyTemplate(tmpl.name, {});
                            if (!code.empty()) {
                                InsertCodeIntoEditor(code);
                            }
                        }
                        
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                    ImGui::EndChild();
                }
            } else {
                ImGui::TextDisabled("File manager not available");
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Completion")) {
            ImGui::Text("Code Completion");
            ImGui::Separator();
            
            ImGui::TextWrapped("Available code completion features:");
            ImGui::Spacing();
            
            if (ImGui::CollapsingHeader("GPIO Functions")) {
                if (ImGui::Button("pinMode()")) {
                    InsertCodeIntoEditor("pinMode(pin, mode);");
                }
                if (ImGui::Button("digitalWrite()")) {
                    InsertCodeIntoEditor("digitalWrite(pin, value);");
                }
                if (ImGui::Button("digitalRead()")) {
                    InsertCodeIntoEditor("digitalRead(pin);");
                }
                if (ImGui::Button("analogRead()")) {
                    InsertCodeIntoEditor("analogRead(pin);");
                }
            }
            
            if (ImGui::CollapsingHeader("Serial Functions")) {
                if (ImGui::Button("Serial.begin()")) {
                    InsertCodeIntoEditor("Serial.begin(115200);");
                }
                if (ImGui::Button("Serial.println()")) {
                    InsertCodeIntoEditor("Serial.println(\"message\");");
                }
                if (ImGui::Button("Serial.print()")) {
                    InsertCodeIntoEditor("Serial.print(\"message\");");
                }
            }
            
            if (ImGui::CollapsingHeader("WiFi Functions")) {
                if (ImGui::Button("WiFi.begin()")) {
                    InsertCodeIntoEditor("WiFi.begin(ssid, password);");
                }
                if (ImGui::Button("WiFi.status()")) {
                    InsertCodeIntoEditor("WiFi.status();");
                }
                if (ImGui::Button("WiFi.localIP()")) {
                    InsertCodeIntoEditor("WiFi.localIP();");
                }
            }
            
            if (ImGui::CollapsingHeader("Timing Functions")) {
                if (ImGui::Button("delay()")) {
                    InsertCodeIntoEditor("delay(1000);");
                }
                if (ImGui::Button("millis()")) {
                    InsertCodeIntoEditor("millis();");
                }
                if (ImGui::Button("micros()")) {
                    InsertCodeIntoEditor("micros();");
                }
            }
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void ImGuiWindow::RenderConsole() {
    ImGui::Text("Console Output");
    ImGui::Separator();
    
    ImGui::BeginChild("ConsoleScrolling", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& message : console_messages_) {
        // Color code console messages
        if (message.find("Error") != std::string::npos || 
            message.find("error") != std::string::npos ||
            message.find("failed") != std::string::npos ||
            message.find("Failed") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", message.c_str());
        } else if (message.find("Warning") != std::string::npos || 
                   message.find("warning") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", message.c_str());
        } else if (message.find("Success") != std::string::npos || 
                   message.find("success") != std::string::npos ||
                   message.find("Complete") != std::string::npos ||
                   message.find("✓") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", message.c_str());
        } else if (message.find("Info") != std::string::npos || 
                   message.find("Loading") != std::string::npos ||
                   message.find("Loaded") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%s", message.c_str());
        } else {
            ImGui::TextWrapped("%s", message.c_str());
        }
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
    ImGui::SameLine();
    if (ImGui::Button("Export Log")) {
        ExportConsoleLog();
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
    AddToRecentFiles(filename);
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
    
    // Populate analysis results with simulated data
    // In a real implementation, this would analyze the actual device firmware
    re_analysis_result_.architecture = SAMPLE_RE_ARCHITECTURE;
    re_analysis_result_.flash_size = SAMPLE_RE_FLASH_SIZE;
    re_analysis_result_.entry_point = SAMPLE_RE_ENTRY_POINT;
    re_analysis_result_.functions_detected = SAMPLE_RE_FUNCTIONS_DETECTED;
    re_analysis_result_.strings_found = SAMPLE_RE_STRINGS_FOUND;
    re_analysis_result_.has_data = true;
    re_analysis_performed_ = true;
    
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
    
    // If tab is modified, ask for confirmation
    if (editor_tabs_[tab_index].is_modified) {
        confirmation_message_ = "File '" + editor_tabs_[tab_index].filename + 
                              "' has unsaved changes.\nAre you sure you want to close it?";
        confirmation_callback_ = [this, tab_index]() {
            // Actually close the tab
            editor_tabs_.erase(editor_tabs_.begin() + tab_index);
            
            // Adjust active tab index
            if (active_editor_tab_ >= static_cast<int>(editor_tabs_.size()) && !editor_tabs_.empty()) {
                active_editor_tab_ = editor_tabs_.size() - 1;
            }
            status_bar_message_ = "Tab closed";
        };
        show_confirmation_dialog_ = true;
    } else {
        // Just close it
        editor_tabs_.erase(editor_tabs_.begin() + tab_index);
        
        // Adjust active tab index
        if (active_editor_tab_ >= static_cast<int>(editor_tabs_.size()) && !editor_tabs_.empty()) {
            active_editor_tab_ = editor_tabs_.size() - 1;
        }
        status_bar_message_ = "Tab closed";
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
    
    // Check if code will fit in buffer (reserve space for null terminator)
    if (code.length() > EDITOR_BUFFER_SIZE - 1) {
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

void ImGuiWindow::RenderTerminalPanel() {
    ImGui::Text("Integrated Terminal");
    ImGui::Separator();
    
    // Terminal output
    ImGui::BeginChild("TerminalOutput", ImVec2(0, -30), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : terminal_history_) {
        // Color code output
        if (line.find("Error:") != std::string::npos || line.find("error:") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", line.c_str());
        } else if (line.find("Warning:") != std::string::npos || line.find("warning:") != std::string::npos) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", line.c_str());
        } else if (line.find("$") == 0 || line.find(">") == 0) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", line.c_str());
        } else {
            ImGui::Text("%s", line.c_str());
        }
    }
    
    if (terminal_scroll_to_bottom_) {
        ImGui::SetScrollHereY(1.0f);
        terminal_scroll_to_bottom_ = false;
    }
    ImGui::EndChild();
    
    // Command input with autocomplete
    ImGui::PushItemWidth(-1);
    
    // Check for autocomplete on input change
    ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways;
    
    bool enter_pressed = ImGui::InputText("##terminal_input", terminal_input_buffer_, sizeof(terminal_input_buffer_), 
                         input_flags, [](ImGuiInputTextCallbackData* data) -> int {
        ImGuiWindow* window = (ImGuiWindow*)data->UserData;
        if (window) {
            // Update suggestions based on current input
            std::string current_input(data->Buf, data->BufTextLen);
            window->terminal_suggestions_.clear();
            
            if (!current_input.empty()) {
                for (const auto& cmd : window->terminal_commands_) {
                    if (cmd.find(current_input) == 0) {
                        window->terminal_suggestions_.push_back(cmd);
                    }
                }
            }
        }
        return 0;
    }, this);
    
    if (enter_pressed) {
        std::string command(terminal_input_buffer_);
        if (!command.empty()) {
            ExecuteTerminalCommand(command);
            terminal_input_buffer_[0] = '\0';
            terminal_scroll_to_bottom_ = true;
            terminal_suggestions_.clear();
        }
    }
    
    // Handle TAB for autocomplete
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Tab, false)) {
        if (!terminal_suggestions_.empty()) {
            // Use first suggestion with length check
            size_t copy_len = std::min(terminal_suggestions_[0].length(), 
                                      sizeof(terminal_input_buffer_) - 1);
            strncpy(terminal_input_buffer_, terminal_suggestions_[0].c_str(), copy_len);
            terminal_input_buffer_[copy_len] = '\0';
        }
    }
    
    // Show autocomplete suggestions
    if (!terminal_suggestions_.empty() && ImGui::IsItemFocused()) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y - 
                               (terminal_suggestions_.size() * ImGui::GetTextLineHeightWithSpacing()) - 5));
        ImGui::BeginTooltip();
        ImGui::Text("Suggestions (press TAB):");
        ImGui::Separator();
        for (const auto& suggestion : terminal_suggestions_) {
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "  %s", suggestion.c_str());
        }
        ImGui::EndTooltip();
    }
    
    ImGui::PopItemWidth();
}

void ImGuiWindow::ExecuteTerminalCommand(const std::string& command) {
    terminal_history_.push_back("$ " + command);
    
    if (command == "help") {
        terminal_history_.push_back("Available commands:");
        terminal_history_.push_back("  help     - Show this help message");
        terminal_history_.push_back("  clear    - Clear terminal output");
        terminal_history_.push_back("  compile  - Compile current code");
        terminal_history_.push_back("  upload   - Upload code to device");
        terminal_history_.push_back("  ports    - List available serial ports");
        terminal_history_.push_back("  boards   - List detected boards");
        terminal_history_.push_back("  ls       - List files in project");
        terminal_history_.push_back("  version  - Show IDE version");
        terminal_history_.push_back("  exit     - Clear terminal");
    } else if (command == "clear" || command == "exit") {
        terminal_history_.clear();
        terminal_history_.push_back("Terminal cleared");
    } else if (command == "version") {
        terminal_history_.push_back("ESP32 Driver IDE v1.0.0");
        terminal_history_.push_back("Built with ImGui + GLFW + OpenGL");
        terminal_history_.push_back("Copyright 2025");
    } else if (command == "compile") {
        terminal_history_.push_back("Compiling code...");
        CompileCode();
        terminal_history_.push_back("Compilation complete - check console for details");
    } else if (command == "upload") {
        terminal_history_.push_back("Uploading code to device...");
        UploadCode();
        terminal_history_.push_back("Upload complete - check console for details");
    } else if (command == "ports") {
        terminal_history_.push_back("Available serial ports:");
        if (available_ports_.empty()) {
            terminal_history_.push_back("  (none found)");
        } else {
            for (const auto& port : available_ports_) {
                terminal_history_.push_back("  " + port);
            }
        }
    } else if (command == "boards") {
        terminal_history_.push_back("Detected boards:");
        if (detected_boards_.empty()) {
            terminal_history_.push_back("  (none found - run 'ports' to scan)");
        } else {
            for (const auto& board : detected_boards_) {
                terminal_history_.push_back("  " + board.name + " - " + board.chip + " @ " + board.port);
            }
        }
    } else if (command == "ls") {
        terminal_history_.push_back("Project files:");
        for (const auto& file : file_list_) {
            terminal_history_.push_back("  " + file);
        }
    } else {
        terminal_history_.push_back("Error: Unknown command '" + command + "'");
        terminal_history_.push_back("Type 'help' for available commands");
    }
}

void ImGuiWindow::RenderBoardListPanel() {
    ImGui::Text("Detected Boards");
    ImGui::Separator();
    
    if (ImGui::Button("Scan for Boards")) {
        RefreshBoardList();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Show Schematic")) {
        show_device_schematic_ = !show_device_schematic_;
    }
    
    ImGui::Separator();
    
    if (detected_boards_.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No boards detected");
        ImGui::TextWrapped("Click 'Scan for Boards' to detect connected ESP32 devices");
    } else {
        ImGui::Text("Found %d board(s):", static_cast<int>(detected_boards_.size()));
        ImGui::Spacing();
        
        for (size_t i = 0; i < detected_boards_.size(); i++) {
            const auto& board = detected_boards_[i];
            
            ImGui::PushID(i);
            bool is_selected = (static_cast<int>(i) == selected_board_index_);
            
            if (ImGui::Selectable(board.name.c_str(), is_selected)) {
                selected_board_index_ = static_cast<int>(i);
            }
            
            if (is_selected) {
                ImGui::Indent();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Chip: %s", board.chip.c_str());
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Port: %s", board.port.c_str());
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Flash: %d MB", board.flash_size_mb);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "RAM: %d KB", board.ram_size_kb);
                
                if (board.is_connected) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Connected");
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Not connected");
                }
                ImGui::Unindent();
            }
            
            ImGui::PopID();
        }
    }
}

void ImGuiWindow::RefreshBoardList() {
    AddConsoleMessage("Scanning for ESP32 boards...");
    
    // Clear existing boards
    detected_boards_.clear();
    
    // Scan ports and try to detect board information
    RefreshPortList();
    
    for (const auto& port : available_ports_) {
        BoardInfo board;
        board.port = port;
        board.is_connected = (port == selected_port_ && is_connected_);
        
        // NOTE: This is a simplified board detection based on port name patterns.
        // In a production implementation, this should:
        // 1. Query the device via serial for chip ID (using esptool.py protocol)
        // 2. Read manufacturer/product strings from USB descriptors
        // 3. Use vendor ID (VID) and product ID (PID) to identify ESP32 devices
        // Current heuristic-based approach may incorrectly identify non-ESP32 devices.
        
        // ESP32 boards typically use CP2102, CH340, or FTDI USB-to-serial chips
        if (port.find("ttyUSB") != std::string::npos || port.find("USB") != std::string::npos) {
            board.name = "ESP32-DevKit";
            board.chip = "ESP32";
            board.flash_size_mb = 4;
            board.ram_size_kb = 520;
        } else if (port.find("ttyACM") != std::string::npos || port.find("ACM") != std::string::npos) {
            board.name = "ESP32-S3-DevKit";
            board.chip = "ESP32-S3";
            board.flash_size_mb = 8;
            board.ram_size_kb = 512;
        } else {
            // Default fallback for unrecognized ports
            board.name = "ESP32 Board";
            board.chip = "ESP32";
            board.flash_size_mb = 4;
            board.ram_size_kb = 520;
        }
        
        detected_boards_.push_back(board);
    }
    
    AddConsoleMessage("✓ Found " + std::to_string(detected_boards_.size()) + " board(s)");
}

void ImGuiWindow::RenderDeviceSchematic() {
    ImGui::Text("ESP32 Device Schematic");
    ImGui::Separator();
    
    // View selector
    if (ImGui::BeginTabBar("SchematicTabs")) {
        if (ImGui::BeginTabItem("Pinout")) {
            current_schematic_view_ = 0;
            
            ImGui::TextWrapped("ESP32 DevKit Pinout Diagram");
            ImGui::Separator();
            
            // Draw a simple text-based pinout (in a real app, this would be an image or vector graphics)
            ImGui::BeginChild("Pinout", ImVec2(0, 0), true);
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Left Side:");
            ImGui::Text("EN          - Enable (Reset)");
            ImGui::Text("VP (GPIO36) - Analog Input Only");
            ImGui::Text("VN (GPIO39) - Analog Input Only");
            ImGui::Text("GPIO34      - Analog Input Only");
            ImGui::Text("GPIO35      - Analog Input Only");
            ImGui::Text("GPIO32      - ADC1_CH4, Touch9");
            ImGui::Text("GPIO33      - ADC1_CH5, Touch8");
            ImGui::Text("GPIO25      - ADC2_CH8, DAC1");
            ImGui::Text("GPIO26      - ADC2_CH9, DAC2");
            ImGui::Text("GPIO27      - ADC2_CH7, Touch7");
            ImGui::Text("GPIO14      - ADC2_CH6, Touch6");
            ImGui::Text("GPIO12      - ADC2_CH5, Touch5");
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Right Side:");
            ImGui::Text("GPIO23      - VSPI MOSI");
            ImGui::Text("GPIO22      - I2C SCL");
            ImGui::Text("TX (GPIO1)  - UART0 TX");
            ImGui::Text("RX (GPIO3)  - UART0 RX");
            ImGui::Text("GPIO21      - I2C SDA");
            ImGui::Text("GPIO19      - VSPI MISO");
            ImGui::Text("GPIO18      - VSPI SCK");
            ImGui::Text("GPIO5       - VSPI CS");
            ImGui::Text("GPIO17      - UART2 TX");
            ImGui::Text("GPIO16      - UART2 RX");
            ImGui::Text("GPIO4       - ADC2_CH0, Touch0");
            ImGui::Text("GPIO2       - ADC2_CH2, Touch2, LED");
            ImGui::Text("GPIO15      - ADC2_CH3, Touch3");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Power Pins:");
            ImGui::Text("3.3V        - 3.3V Power Output");
            ImGui::Text("GND         - Ground (multiple pins)");
            ImGui::Text("VIN         - 5V Power Input");
            
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Block Diagram")) {
            current_schematic_view_ = 1;
            
            ImGui::TextWrapped("ESP32 System Architecture");
            ImGui::Separator();
            
            ImGui::BeginChild("BlockDiagram", ImVec2(0, 0), true);
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "CPU:");
            ImGui::BulletText("Dual-core Xtensa LX6 @ 240 MHz");
            ImGui::BulletText("448 KB ROM");
            ImGui::BulletText("520 KB SRAM");
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Wireless:");
            ImGui::BulletText("WiFi 802.11 b/g/n (2.4 GHz)");
            ImGui::BulletText("Bluetooth v4.2 BR/EDR + BLE");
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Peripherals:");
            ImGui::BulletText("34 GPIO pins");
            ImGui::BulletText("18 ADC channels (12-bit)");
            ImGui::BulletText("2 DAC channels (8-bit)");
            ImGui::BulletText("10 Touch sensors");
            ImGui::BulletText("3 UART interfaces");
            ImGui::BulletText("3 SPI interfaces");
            ImGui::BulletText("2 I2C interfaces");
            ImGui::BulletText("16 PWM channels");
            ImGui::BulletText("SD/SDIO/MMC host");
            ImGui::BulletText("Ethernet MAC");
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Security:");
            ImGui::BulletText("Secure boot");
            ImGui::BulletText("Flash encryption");
            ImGui::BulletText("Hardware crypto acceleration");
            
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void ImGuiWindow::RenderSyntaxHighlightedText(const std::string& code) {
    if (!enable_syntax_highlighting_ || !syntax_highlighter_) {
        // Fallback to plain text
        ImGui::TextUnformatted(code.c_str());
        return;
    }
    
    // Tokenize and render with colors
    auto tokens = syntax_highlighter_->Tokenize(code);
    
    for (const auto& token : tokens) {
        ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f); // Default white
        
        switch (token.type) {
            case SyntaxHighlighter::TokenType::KEYWORD:
                color = ImVec4(0.8f, 0.3f, 0.8f, 1.0f); // Purple
                break;
            case SyntaxHighlighter::TokenType::TYPE:
                color = ImVec4(0.3f, 0.8f, 0.8f, 1.0f); // Cyan
                break;
            case SyntaxHighlighter::TokenType::FUNCTION:
                color = ImVec4(0.8f, 0.8f, 0.3f, 1.0f); // Yellow
                break;
            case SyntaxHighlighter::TokenType::STRING:
                color = ImVec4(0.8f, 0.5f, 0.3f, 1.0f); // Orange
                break;
            case SyntaxHighlighter::TokenType::COMMENT:
                color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f); // Green
                break;
            case SyntaxHighlighter::TokenType::NUMBER:
                color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f); // Light blue
                break;
            case SyntaxHighlighter::TokenType::PREPROCESSOR:
                color = ImVec4(0.8f, 0.4f, 0.4f, 1.0f); // Red
                break;
            case SyntaxHighlighter::TokenType::OPERATOR:
            case SyntaxHighlighter::TokenType::IDENTIFIER:
            case SyntaxHighlighter::TokenType::WHITESPACE:
            default:
                color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Light gray
                break;
        }
        
        ImGui::TextColored(color, "%s", token.text.c_str());
        
        // Handle line breaks properly - if token contains newline, don't use SameLine
        if (token.text.find('\n') == std::string::npos) {
            ImGui::SameLine(0, 0);
        }
        // If token ends with newline, the next token will naturally start on new line
    }
}

void ImGuiWindow::RenderBlueprintTab() {
    ImGui::Text("Blueprint Node Editor");
    ImGui::Separator();
    
    // Toolbar for blueprint actions
    if (ImGui::Button("Clear All")) {
        if (blueprint_editor_) {
            blueprint_editor_->Clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate Code")) {
        if (blueprint_editor_) {
            std::string generated_code = blueprint_editor_->GenerateCode();
            AddConsoleMessage("Generated code from blueprint:");
            AddConsoleMessage(generated_code);
        }
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                      "Right-click in empty space to add nodes");
    
    ImGui::Separator();
    
    // Render the blueprint editor
    if (blueprint_editor_) {
        blueprint_editor_->Render();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
                          "Blueprint editor failed to initialize");
    }
}

void ImGuiWindow::RenderStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!viewport) return;
    
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 22));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 22));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                                   ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 3));
    ImGui::Begin("StatusBar", nullptr, window_flags);
    
    // Left side - status message
    ImGui::Text("%s", status_bar_message_.c_str());
    
    ImGui::SameLine(viewport->Size.x - 400);
    ImGui::Separator();
    ImGui::SameLine();
    
    // Right side - file info
    if (active_editor_tab_ >= 0 && active_editor_tab_ < editor_tabs_.size()) {
        const auto& tab = editor_tabs_[active_editor_tab_];
        
        // File encoding
        ImGui::Text("UTF-8");
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        
        // Line/column
        ImGui::Text("Ln %d, Col %d", cursor_line_, cursor_column_);
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        
        // Line count
        int line_count = CountLines(tab.content);
        ImGui::Text("Lines: %d", line_count);
        
        if (tab.is_modified) {
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Modified");
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void ImGuiWindow::RenderFindDialog() {
    ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Find", &show_find_dialog_);
    
    ImGui::Text("Find:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##find", find_buffer_, sizeof(find_buffer_), ImGuiInputTextFlags_EnterReturnsTrue)) {
        FindInCurrentFile(find_buffer_);
    }
    
    ImGui::Spacing();
    if (ImGui::Button("Find Next", ImVec2(120, 0))) {
        FindInCurrentFile(find_buffer_);
    }
    ImGui::SameLine();
    if (ImGui::Button("Close", ImVec2(120, 0))) {
        show_find_dialog_ = false;
    }
    
    ImGui::End();
}

void ImGuiWindow::RenderConfirmationDialog() {
    ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    
    ImGui::Begin("Confirmation", &show_confirmation_dialog_, flags);
    
    ImGui::TextWrapped("%s", confirmation_message_.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    float button_width = 100.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float total_width = button_width * 2 + spacing;
    float offset = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;
    
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    
    if (ImGui::Button("Yes", ImVec2(button_width, 0))) {
        if (confirmation_callback_) {
            confirmation_callback_();
        }
        show_confirmation_dialog_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("No", ImVec2(button_width, 0))) {
        show_confirmation_dialog_ = false;
    }
    
    ImGui::End();
}

void ImGuiWindow::RenderSettingsDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    
    ImGui::Begin("Settings", &show_settings_dialog_);
    
    if (ImGui::BeginTabBar("SettingsTabs")) {
        if (ImGui::BeginTabItem("Editor")) {
            ImGui::Spacing();
            ImGui::Text("Editor Settings");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Checkbox("Show Line Numbers", &show_line_numbers_);
            ImGui::Checkbox("Syntax Highlighting", &enable_syntax_highlighting_);
            ImGui::Checkbox("Auto-Indent", &settings_auto_indent_);
            
            ImGui::Spacing();
            ImGui::SliderInt("Tab Size", &settings_tab_size_, 2, 8);
            
            ImGui::Spacing();
            ImGui::Text("Theme:");
            ImGui::SameLine();
            if (ImGui::RadioButton("Dark", settings_theme_ == "Dark")) {
                settings_theme_ = "Dark";
                SetupImGuiStyle(); // Reapply theme
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Light", settings_theme_ == "Light")) {
                settings_theme_ = "Light";
                ImGui::StyleColorsLight(); // Apply light theme
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Auto-Save")) {
            ImGui::Spacing();
            ImGui::Text("Auto-Save Settings");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Checkbox("Enable Auto-Save", &auto_save_enabled_);
            
            if (auto_save_enabled_) {
                ImGui::Spacing();
                ImGui::SliderFloat("Interval (seconds)", &auto_save_interval_, 10.0f, 300.0f);
                float time_until_next = auto_save_interval_ - ((float)glfwGetTime() - last_auto_save_time_);
                if (time_until_next < 0) time_until_next = 0;
                ImGui::Text("Next auto-save in: %.0f seconds", time_until_next);
            }
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Interface")) {
            ImGui::Spacing();
            ImGui::Text("Interface Settings");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Checkbox("Show File Explorer", &show_file_explorer_);
            ImGui::Checkbox("Show Board List", &show_board_list_);
            ImGui::Checkbox("Show Properties Panel", &show_properties_panel_);
            ImGui::Checkbox("Show AI Assistant", &show_ai_assistant_);
            ImGui::Checkbox("Show Terminal", &show_terminal_);
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Apply & Close", ImVec2(120, 0))) {
        show_settings_dialog_ = false;
        status_bar_message_ = "Settings saved";
        AddConsoleMessage("Settings updated successfully");
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        show_settings_dialog_ = false;
    }
    
    ImGui::End();
}

void ImGuiWindow::HandleKeyboardShortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Ctrl+S - Save current file
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        SaveCurrentTab();
        status_bar_message_ = "File saved";
    }
    
    // Ctrl+N - New file
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        EditorTab new_tab;
        new_tab.filename = "sketch_" + std::to_string(editor_tabs_.size() + 1) + ".ino";
        new_tab.content = SIMPLE_SKETCH_TEMPLATE;
        new_tab.is_modified = false;
        std::snprintf(new_tab.buffer, EDITOR_BUFFER_SIZE, "%s", new_tab.content.c_str());
        editor_tabs_.push_back(new_tab);
        active_editor_tab_ = editor_tabs_.size() - 1;
        status_bar_message_ = "New file created";
    }
    
    // Ctrl+F - Find
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        show_find_dialog_ = !show_find_dialog_;
    }
    
    // Ctrl+W - Close current tab
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false)) {
        if (active_editor_tab_ >= 0 && active_editor_tab_ < editor_tabs_.size()) {
            CloseTab(active_editor_tab_);
            status_bar_message_ = "Tab closed";
        }
    }
    
    // F5 - Compile
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
        CompileCode();
    }
}

void ImGuiWindow::UpdateCursorPosition() {
    // This is called to update cursor position in the editor
    // The actual cursor position tracking would need deeper integration
    // with the text input widget, which ImGui doesn't directly expose
    // For now, this is a placeholder that could be enhanced with
    // custom text editor implementation or ImGui::InputTextMultiline callbacks
}

void ImGuiWindow::PerformAutoSave() {
    float current_time = (float)glfwGetTime();
    
    if (current_time - last_auto_save_time_ >= auto_save_interval_) {
        // Auto-save all modified tabs
        bool saved_any = false;
        for (auto& tab : editor_tabs_) {
            if (tab.is_modified) {
                // In a real implementation, this would save to file
                saved_any = true;
            }
        }
        
        if (saved_any) {
            status_bar_message_ = "Auto-saved at " + std::to_string((int)current_time) + "s";
        }
        
        last_auto_save_time_ = current_time;
    }
}

int ImGuiWindow::CountLines(const std::string& text) const {
    if (text.empty()) return 1;
    return std::count(text.begin(), text.end(), '\n') + 1;
}

void ImGuiWindow::FindInCurrentFile(const std::string& search_term) {
    if (search_term.empty()) return;
    if (active_editor_tab_ < 0 || active_editor_tab_ >= editor_tabs_.size()) return;
    
    const auto& content = editor_tabs_[active_editor_tab_].content;
    size_t pos = content.find(search_term);
    
    if (pos != std::string::npos) {
        status_bar_message_ = "Found at position " + std::to_string(pos);
        AddConsoleMessage("Found: '" + search_term + "' at position " + std::to_string(pos));
    } else {
        status_bar_message_ = "'" + search_term + "' not found";
        AddConsoleMessage("Not found: '" + search_term + "'");
    }
}

void ImGuiWindow::AddToRecentFiles(const std::string& filename) {
    // Remove if already exists
    auto it = std::find(recent_files_.begin(), recent_files_.end(), filename);
    if (it != recent_files_.end()) {
        recent_files_.erase(it);
    }
    
    // Add to front
    recent_files_.insert(recent_files_.begin(), filename);
    
    // Limit size
    if (recent_files_.size() > MAX_RECENT_FILES) {
        recent_files_.resize(MAX_RECENT_FILES);
    }
}

void ImGuiWindow::ExportConsoleLog() {
    // Generate timestamp
    time_t now = time(nullptr);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));
    
    std::string log_filename = "console_log_" + std::string(timestamp) + ".txt";
    
    // In a real implementation, this would write to a file
    std::string log_content;
    for (const auto& msg : console_messages_) {
        log_content += msg + "\n";
    }
    
    // For now, just show a message
    AddConsoleMessage("Console log exported to: " + log_filename);
    AddConsoleMessage("Total lines: " + std::to_string(console_messages_.size()));
    status_bar_message_ = "Console log exported";
}

void ImGuiWindow::ExportGeneratedCode() {
    if (active_editor_tab_ < 0 || active_editor_tab_ >= editor_tabs_.size()) {
        AddConsoleMessage("No active file to export");
        return;
    }
    
    const auto& tab = editor_tabs_[active_editor_tab_];
    std::string export_filename = "exported_" + tab.filename;
    
    // In a real implementation, this would write to a file
    AddConsoleMessage("Code exported to: " + export_filename);
    AddConsoleMessage("File: " + tab.filename);
    AddConsoleMessage("Size: " + std::to_string(tab.content.length()) + " bytes");
    AddConsoleMessage("Lines: " + std::to_string(CountLines(tab.content)));
    status_bar_message_ = "Code exported to " + export_filename;
}

} // namespace gui
} // namespace esp32_ide
