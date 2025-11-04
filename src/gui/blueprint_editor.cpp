#include "gui/blueprint_editor.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <algorithm>
#include <sstream>

namespace ed = ax::NodeEditor;

namespace esp32_ide {
namespace gui {

// Node implementation
void Node::AddInputPin(const std::string& pin_name, PinType pin_type) {
    int pin_id = input_pins.size() + output_pins.size();
    input_pins.emplace_back(pin_id, pin_name, pin_type, id);
}

void Node::AddOutputPin(const std::string& pin_name, PinType pin_type) {
    int pin_id = input_pins.size() + output_pins.size();
    output_pins.emplace_back(pin_id, pin_name, pin_type, id);
}

// BlueprintEditor implementation
BlueprintEditor::BlueprintEditor()
    : context_(nullptr),
      next_node_id_(1),
      next_pin_id_(1),
      next_link_id_(1),
      show_create_menu_(false),
      create_menu_x_(0),
      create_menu_y_(0) {
}

BlueprintEditor::~BlueprintEditor() {
    Shutdown();
}

bool BlueprintEditor::Initialize() {
    ed::Config config;
    config.SettingsFile = nullptr; // Don't save settings for now
    context_ = ed::CreateEditor(&config);
    
    if (!context_) {
        return false;
    }
    
    return true;
}

void BlueprintEditor::Shutdown() {
    if (context_) {
        ed::DestroyEditor(context_);
        context_ = nullptr;
    }
}

void BlueprintEditor::Render() {
    if (!context_) {
        return;
    }
    
    ed::SetCurrentEditor(context_);
    ed::Begin("Blueprint Editor");
    
    // Render all nodes
    for (auto& [node_id, node] : nodes_) {
        RenderNode(node);
    }
    
    // Render all links
    for (auto& [link_id, link] : links_) {
        ed::Link(link.id, link.start_pin_id, link.end_pin_id);
    }
    
    // Handle interactions
    HandleInteractions();
    
    ed::End();
    ed::SetCurrentEditor(nullptr);
    
    // Render context menu
    if (show_create_menu_) {
        RenderCreateMenu();
    }
}

void BlueprintEditor::RenderNode(Node& node) {
    ed::BeginNode(node.id);
    
    // Node header
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", node.name.c_str());
    ImGui::PopStyleColor();
    
    ImGui::Separator();
    
    // Input pins on the left
    for (auto& pin : node.input_pins) {
        ed::BeginPin(pin.id, ed::PinKind::Input);
        ImGui::PushStyleColor(ImGuiCol_Text, GetPinTypeColor(pin.type));
        ImGui::Text("-> %s", pin.name.c_str());
        ImGui::PopStyleColor();
        ed::EndPin();
    }
    
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(40, 0)); // Spacing between input and output
    ImGui::SameLine();
    
    // Output pins on the right
    ImGui::BeginGroup();
    for (auto& pin : node.output_pins) {
        ed::BeginPin(pin.id, ed::PinKind::Output);
        ImGui::PushStyleColor(ImGuiCol_Text, GetPinTypeColor(pin.type));
        ImGui::Text("%s ->", pin.name.c_str());
        ImGui::PopStyleColor();
        ed::EndPin();
    }
    ImGui::EndGroup();
    
    ed::EndNode();
}

void BlueprintEditor::RenderPin(Pin& pin) {
    // Pins are rendered as part of nodes
}

void BlueprintEditor::RenderCreateMenu() {
    ImGui::SetNextWindowPos(ImVec2(create_menu_x_, create_menu_y_));
    if (ImGui::Begin("Create Node", &show_create_menu_, 
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        
        if (ImGui::MenuItem("GPIO Output")) {
            CreateGPIOOutputNode("GPIO Output", 2);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("GPIO Input")) {
            CreateGPIOInputNode("GPIO Input", 4);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("PWM")) {
            CreatePWMNode("PWM", 5);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("I2C Master")) {
            CreateI2CMasterNode("I2C Master", 21, 22);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("SPI Master")) {
            CreateSPIMasterNode("SPI Master", 23, 19, 18, 5);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("UART")) {
            CreateUARTNode("UART", 1, 3);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("ADC")) {
            CreateADCNode("ADC", 34);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("DAC")) {
            CreateDACNode("DAC", 25);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("LED")) {
            CreateLEDNode("LED", 2);
            show_create_menu_ = false;
        }
        if (ImGui::MenuItem("Button")) {
            CreateButtonNode("Button", 0);
            show_create_menu_ = false;
        }
        
        ImGui::End();
    }
}

void BlueprintEditor::HandleInteractions() {
    // Handle node creation
    if (ed::BeginCreate()) {
        ed::PinId start_pin_id, end_pin_id;
        if (ed::QueryNewLink(&start_pin_id, &end_pin_id)) {
            if (start_pin_id && end_pin_id) {
                if (ed::AcceptNewItem()) {
                    CreateLink(start_pin_id.Get(), end_pin_id.Get());
                }
            }
        }
    }
    ed::EndCreate();
    
    // Handle node deletion
    if (ed::BeginDelete()) {
        ed::NodeId node_id;
        while (ed::QueryDeletedNode(&node_id)) {
            if (ed::AcceptDeletedItem()) {
                DeleteNode(node_id.Get());
            }
        }
        
        ed::LinkId link_id;
        while (ed::QueryDeletedLink(&link_id)) {
            if (ed::AcceptDeletedItem()) {
                DeleteLink(link_id.Get());
            }
        }
    }
    ed::EndDelete();
    
    // Handle background context menu
    ed::Suspend();
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ed::ShowBackgroundContextMenu()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        create_menu_x_ = mouse_pos.x;
        create_menu_y_ = mouse_pos.y;
        show_create_menu_ = true;
    }
    ed::Resume();
}

int BlueprintEditor::CreateNode(NodeType type, const std::string& name) {
    int node_id = next_node_id_++;
    nodes_.emplace(node_id, Node(node_id, type, name));
    return node_id;
}

void BlueprintEditor::DeleteNode(int node_id) {
    // Delete all links connected to this node
    std::vector<int> links_to_delete;
    for (auto& [link_id, link] : links_) {
        Pin* start_pin = GetPin(link.start_pin_id);
        Pin* end_pin = GetPin(link.end_pin_id);
        if ((start_pin && start_pin->node_id == node_id) ||
            (end_pin && end_pin->node_id == node_id)) {
            links_to_delete.push_back(link_id);
        }
    }
    
    for (int link_id : links_to_delete) {
        DeleteLink(link_id);
    }
    
    // Delete the node
    auto it = nodes_.find(node_id);
    if (it != nodes_.end()) {
        // Remove pins from pin map
        for (auto& pin : it->second.input_pins) {
            pins_.erase(pin.id);
        }
        for (auto& pin : it->second.output_pins) {
            pins_.erase(pin.id);
        }
        nodes_.erase(it);
    }
}

Node* BlueprintEditor::GetNode(int node_id) {
    auto it = nodes_.find(node_id);
    return (it != nodes_.end()) ? &it->second : nullptr;
}

int BlueprintEditor::CreateLink(int start_pin_id, int end_pin_id) {
    int link_id = next_link_id_++;
    links_.emplace(link_id, Link(link_id, start_pin_id, end_pin_id));
    return link_id;
}

void BlueprintEditor::DeleteLink(int link_id) {
    links_.erase(link_id);
}

Pin* BlueprintEditor::GetPin(int pin_id) {
    auto it = pins_.find(pin_id);
    return (it != pins_.end()) ? it->second : nullptr;
}

int BlueprintEditor::AllocatePinId() {
    return next_pin_id_++;
}

void BlueprintEditor::CreateGPIOOutputNode(const std::string& name, int gpio_pin) {
    int node_id = CreateNode(NodeType::GPIO_OUTPUT, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "Signal", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "GPIO " + std::to_string(gpio_pin), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateGPIOInputNode(const std::string& name, int gpio_pin) {
    int node_id = CreateNode(NodeType::GPIO_INPUT, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "GPIO " + std::to_string(gpio_pin), PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "Value", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreatePWMNode(const std::string& name, int gpio_pin) {
    int node_id = CreateNode(NodeType::PWM, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "Duty Cycle", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "Frequency", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "GPIO " + std::to_string(gpio_pin), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateI2CMasterNode(const std::string& name, int sda_pin, int scl_pin) {
    int node_id = CreateNode(NodeType::I2C_MASTER, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "Write Data", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "Read Data", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "SDA " + std::to_string(sda_pin), PinType::BIDIRECTIONAL, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "SCL " + std::to_string(scl_pin), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateSPIMasterNode(const std::string& name, int mosi, int miso, int clk, int cs) {
    int node_id = CreateNode(NodeType::SPI_MASTER, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "TX Data", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "RX Data", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "MOSI " + std::to_string(mosi), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "MISO " + std::to_string(miso), PinType::INPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "CLK " + std::to_string(clk), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "CS " + std::to_string(cs), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateUARTNode(const std::string& name, int tx_pin, int rx_pin) {
    int node_id = CreateNode(NodeType::UART, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "TX Data", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "RX Data", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "TX " + std::to_string(tx_pin), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "RX " + std::to_string(rx_pin), PinType::INPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateADCNode(const std::string& name, int adc_pin) {
    int node_id = CreateNode(NodeType::ADC, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "ADC " + std::to_string(adc_pin), PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "Value", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateDACNode(const std::string& name, int dac_pin) {
    int node_id = CreateNode(NodeType::DAC, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "Value", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "DAC " + std::to_string(dac_pin), PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
    }
}

void BlueprintEditor::CreateLEDNode(const std::string& name, int gpio_pin) {
    int node_id = CreateNode(NodeType::LED, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "State", PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
        
        pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "GPIO " + std::to_string(gpio_pin), PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
    }
}

void BlueprintEditor::CreateButtonNode(const std::string& name, int gpio_pin) {
    int node_id = CreateNode(NodeType::BUTTON, name);
    Node* node = GetNode(node_id);
    if (node) {
        int pin_id = AllocatePinId();
        node->output_pins.emplace_back(pin_id, "Pressed", PinType::OUTPUT, node_id);
        pins_[pin_id] = &node->output_pins.back();
        
        pin_id = AllocatePinId();
        node->input_pins.emplace_back(pin_id, "GPIO " + std::to_string(gpio_pin), PinType::INPUT, node_id);
        pins_[pin_id] = &node->input_pins.back();
    }
}

void BlueprintEditor::Clear() {
    nodes_.clear();
    links_.clear();
    pins_.clear();
    next_node_id_ = 1;
    next_pin_id_ = 1;
    next_link_id_ = 1;
}

std::string BlueprintEditor::GenerateCode() const {
    std::stringstream ss;
    ss << "// Generated code from Blueprint Editor\n\n";
    
    // Generate setup code
    ss << "void setup() {\n";
    for (const auto& [node_id, node] : nodes_) {
        switch (node.type) {
            case NodeType::GPIO_OUTPUT:
                ss << "  pinMode(GPIO_PIN, OUTPUT);\n";
                break;
            case NodeType::GPIO_INPUT:
                ss << "  pinMode(GPIO_PIN, INPUT);\n";
                break;
            case NodeType::PWM:
                ss << "  ledcSetup(0, 5000, 8);\n";
                ss << "  ledcAttachPin(GPIO_PIN, 0);\n";
                break;
            case NodeType::I2C_MASTER:
                ss << "  Wire.begin(SDA_PIN, SCL_PIN);\n";
                break;
            default:
                break;
        }
    }
    ss << "}\n\n";
    
    ss << "void loop() {\n";
    ss << "  // Add your logic here\n";
    ss << "}\n";
    
    return ss.str();
}

const char* BlueprintEditor::GetNodeTypeName(NodeType type) const {
    switch (type) {
        case NodeType::GPIO_OUTPUT: return "GPIO Output";
        case NodeType::GPIO_INPUT: return "GPIO Input";
        case NodeType::PWM: return "PWM";
        case NodeType::I2C_MASTER: return "I2C Master";
        case NodeType::I2C_SLAVE: return "I2C Slave";
        case NodeType::SPI_MASTER: return "SPI Master";
        case NodeType::SPI_SLAVE: return "SPI Slave";
        case NodeType::UART: return "UART";
        case NodeType::ADC: return "ADC";
        case NodeType::DAC: return "DAC";
        case NodeType::TIMER: return "Timer";
        case NodeType::WIFI: return "WiFi";
        case NodeType::BLUETOOTH: return "Bluetooth";
        case NodeType::LED: return "LED";
        case NodeType::BUTTON: return "Button";
        case NodeType::SENSOR: return "Sensor";
        case NodeType::MOTOR: return "Motor";
        case NodeType::DISPLAY: return "Display";
        default: return "Unknown";
    }
}

ImVec4 BlueprintEditor::GetNodeTypeColor(NodeType type) const {
    switch (type) {
        case NodeType::GPIO_OUTPUT:
        case NodeType::GPIO_INPUT:
            return ImVec4(0.2f, 0.7f, 0.3f, 1.0f); // Green
        case NodeType::PWM:
            return ImVec4(0.7f, 0.5f, 0.2f, 1.0f); // Orange
        case NodeType::I2C_MASTER:
        case NodeType::I2C_SLAVE:
            return ImVec4(0.2f, 0.5f, 0.8f, 1.0f); // Blue
        case NodeType::SPI_MASTER:
        case NodeType::SPI_SLAVE:
            return ImVec4(0.5f, 0.2f, 0.8f, 1.0f); // Purple
        case NodeType::UART:
            return ImVec4(0.8f, 0.7f, 0.2f, 1.0f); // Yellow
        case NodeType::ADC:
        case NodeType::DAC:
            return ImVec4(0.8f, 0.2f, 0.4f, 1.0f); // Pink
        case NodeType::LED:
            return ImVec4(1.0f, 0.9f, 0.2f, 1.0f); // Bright Yellow
        case NodeType::BUTTON:
            return ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Gray
        default:
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default gray
    }
}

ImVec4 BlueprintEditor::GetPinTypeColor(PinType type) const {
    switch (type) {
        case PinType::INPUT:
            return ImVec4(0.3f, 0.8f, 0.3f, 1.0f); // Green
        case PinType::OUTPUT:
            return ImVec4(0.8f, 0.3f, 0.3f, 1.0f); // Red
        case PinType::BIDIRECTIONAL:
            return ImVec4(0.8f, 0.8f, 0.3f, 1.0f); // Yellow
        case PinType::POWER:
            return ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Bright Red
        case PinType::GROUND:
            return ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Dark Gray
        default:
            return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Light Gray
    }
}

} // namespace gui
} // namespace esp32_ide
