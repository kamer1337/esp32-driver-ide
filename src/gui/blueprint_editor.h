#ifndef BLUEPRINT_EDITOR_H
#define BLUEPRINT_EDITOR_H

#include <imgui_node_editor.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace ax {
namespace NodeEditor {
    struct EditorContext;
}
}

namespace esp32_ide {
namespace gui {

/**
 * @brief Node types for ESP32 components
 */
enum class NodeType {
    GPIO_OUTPUT,
    GPIO_INPUT,
    PWM,
    I2C_MASTER,
    I2C_SLAVE,
    SPI_MASTER,
    SPI_SLAVE,
    UART,
    ADC,
    DAC,
    TIMER,
    WIFI,
    BLUETOOTH,
    LED,
    BUTTON,
    SENSOR,
    MOTOR,
    DISPLAY
};

/**
 * @brief Pin types for connections
 */
enum class PinType {
    INPUT,
    OUTPUT,
    BIDIRECTIONAL,
    POWER,
    GROUND
};

/**
 * @brief Pin information
 */
struct Pin {
    int id;
    std::string name;
    PinType type;
    int node_id;
    
    Pin(int id_, const std::string& name_, PinType type_, int node_id_)
        : id(id_), name(name_), type(type_), node_id(node_id_) {}
};

/**
 * @brief Node representing an ESP32 component
 */
struct Node {
    int id;
    NodeType type;
    std::string name;
    std::vector<Pin> input_pins;
    std::vector<Pin> output_pins;
    float position_x;
    float position_y;
    
    Node(int id_, NodeType type_, const std::string& name_)
        : id(id_), type(type_), name(name_), position_x(0), position_y(0) {}
    
    void AddInputPin(const std::string& pin_name, PinType pin_type);
    void AddOutputPin(const std::string& pin_name, PinType pin_type);
};

/**
 * @brief Link between two pins
 */
struct Link {
    int id;
    int start_pin_id;
    int end_pin_id;
    
    Link(int id_, int start_, int end_)
        : id(id_), start_pin_id(start_), end_pin_id(end_) {}
};

/**
 * @brief Blueprint editor for visual component design
 * 
 * Provides a node-based interface for designing ESP32 hardware connections
 * and component interactions
 */
class BlueprintEditor {
public:
    BlueprintEditor();
    ~BlueprintEditor();
    
    // Editor lifecycle
    bool Initialize();
    void Render();
    void Shutdown();
    
    // Node management
    int CreateNode(NodeType type, const std::string& name);
    void DeleteNode(int node_id);
    Node* GetNode(int node_id);
    
    // Link management
    int CreateLink(int start_pin_id, int end_pin_id);
    void DeleteLink(int link_id);
    
    // Pin management
    Pin* GetPin(int pin_id);
    
    // Component templates
    void CreateGPIOOutputNode(const std::string& name, int gpio_pin);
    void CreateGPIOInputNode(const std::string& name, int gpio_pin);
    void CreatePWMNode(const std::string& name, int gpio_pin);
    void CreateI2CMasterNode(const std::string& name, int sda_pin, int scl_pin);
    void CreateSPIMasterNode(const std::string& name, int mosi, int miso, int clk, int cs);
    void CreateUARTNode(const std::string& name, int tx_pin, int rx_pin);
    void CreateADCNode(const std::string& name, int adc_pin);
    void CreateDACNode(const std::string& name, int dac_pin);
    void CreateLEDNode(const std::string& name, int gpio_pin);
    void CreateButtonNode(const std::string& name, int gpio_pin);
    
    // Utility
    void Clear();
    std::string GenerateCode() const;
    
    // Pin ID generation constants (public for Node access)
    static constexpr int PIN_ID_MULTIPLIER = 1000;
    static constexpr int OUTPUT_PIN_OFFSET = 500;
    
private:
    ax::NodeEditor::EditorContext* context_;
    
    std::map<int, Node> nodes_;
    std::map<int, Link> links_;
    std::map<int, Pin*> pins_;
    
    int next_node_id_;
    int next_pin_id_;
    int next_link_id_;
    
    // UI state
    bool show_create_menu_;
    float create_menu_x_;
    float create_menu_y_;
    
    // Helper methods
    void RenderNode(Node& node);
    void RenderPin(Pin& pin);
    void RenderCreateMenu();
    void HandleInteractions();
    
    const char* GetNodeTypeName(NodeType type) const;
    ImVec4 GetNodeTypeColor(NodeType type) const;
    ImVec4 GetPinTypeColor(PinType type) const;
    
    // Pin ID management
    int AllocatePinId();
};

} // namespace gui
} // namespace esp32_ide

#endif // BLUEPRINT_EDITOR_H
