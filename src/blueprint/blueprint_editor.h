#ifndef ESP32_IDE_BLUEPRINT_EDITOR_H
#define ESP32_IDE_BLUEPRINT_EDITOR_H

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace esp32_ide {
namespace blueprint {

// Forward declarations
class Component;
class Connection;
class Blueprint;

/**
 * Component Types for ESP32 devices
 */
enum class ComponentType {
    ESP32_BOARD,
    GPIO_PIN,
    SENSOR,
    ACTUATOR,
    LED,
    BUTTON,
    MOTOR,
    DISPLAY,
    COMMUNICATION_MODULE,
    POWER_SUPPLY,
    RESISTOR,
    CAPACITOR,
    CUSTOM
};

/**
 * Component - Represents a hardware component in the blueprint
 */
class Component {
public:
    Component(const std::string& id, ComponentType type, const std::string& name);
    ~Component() = default;

    // Getters
    const std::string& GetId() const { return id_; }
    ComponentType GetType() const { return type_; }
    const std::string& GetName() const { return name_; }
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetWidth() const { return width_; }
    float GetHeight() const { return height_; }
    
    // Setters
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float width, float height) { width_ = width; height_ = height; }
    void SetName(const std::string& name) { name_ = name; }
    
    // Properties
    void SetProperty(const std::string& key, const std::string& value);
    std::string GetProperty(const std::string& key) const;
    const std::map<std::string, std::string>& GetProperties() const { return properties_; }
    
    // Pins
    void AddPin(const std::string& pin_id, const std::string& pin_name);
    std::vector<std::string> GetPins() const;
    
    // Serialization
    std::string Serialize() const;
    static std::unique_ptr<Component> Deserialize(const std::string& data);

private:
    std::string id_;
    ComponentType type_;
    std::string name_;
    float x_, y_;
    float width_, height_;
    std::map<std::string, std::string> properties_;
    std::map<std::string, std::string> pins_;
};

/**
 * Connection - Represents a wire/connection between components
 */
class Connection {
public:
    Connection(const std::string& id, 
               const std::string& from_component, const std::string& from_pin,
               const std::string& to_component, const std::string& to_pin);
    ~Connection() = default;

    // Getters
    const std::string& GetId() const { return id_; }
    const std::string& GetFromComponent() const { return from_component_; }
    const std::string& GetFromPin() const { return from_pin_; }
    const std::string& GetToComponent() const { return to_component_; }
    const std::string& GetToPin() const { return to_pin_; }
    
    // Properties
    void SetProperty(const std::string& key, const std::string& value);
    std::string GetProperty(const std::string& key) const;
    
    // Serialization
    std::string Serialize() const;
    static std::unique_ptr<Connection> Deserialize(const std::string& data);

private:
    std::string id_;
    std::string from_component_;
    std::string from_pin_;
    std::string to_component_;
    std::string to_pin_;
    std::map<std::string, std::string> properties_;
};

/**
 * Blueprint - Container for components and connections
 */
class Blueprint {
public:
    Blueprint();
    ~Blueprint() = default;

    // Component management
    void AddComponent(std::unique_ptr<Component> component);
    void RemoveComponent(const std::string& component_id);
    Component* GetComponent(const std::string& component_id);
    const Component* GetComponent(const std::string& component_id) const;
    const std::vector<std::unique_ptr<Component>>& GetComponents() const { return components_; }
    
    // Connection management
    void AddConnection(std::unique_ptr<Connection> connection);
    void RemoveConnection(const std::string& connection_id);
    Connection* GetConnection(const std::string& connection_id);
    const std::vector<std::unique_ptr<Connection>>& GetConnections() const { return connections_; }
    
    // Blueprint properties
    void SetName(const std::string& name) { name_ = name; }
    const std::string& GetName() const { return name_; }
    void SetDescription(const std::string& desc) { description_ = desc; }
    const std::string& GetDescription() const { return description_; }
    
    // Validation
    bool Validate(std::string& error_message) const;
    
    // Serialization
    std::string Serialize() const;
    bool Deserialize(const std::string& data);
    
    // File operations
    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);
    
    // Clear
    void Clear();

private:
    std::string name_;
    std::string description_;
    std::vector<std::unique_ptr<Component>> components_;
    std::vector<std::unique_ptr<Connection>> connections_;
};

/**
 * BlueprintEditor - Visual editor for creating and modifying blueprints
 */
class BlueprintEditor {
public:
    BlueprintEditor();
    ~BlueprintEditor() = default;

    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Blueprint management
    void NewBlueprint();
    bool LoadBlueprint(const std::string& filename);
    bool SaveBlueprint(const std::string& filename);
    Blueprint* GetCurrentBlueprint() { return current_blueprint_.get(); }
    
    // Component operations
    void AddComponent(ComponentType type, float x, float y);
    void DeleteComponent(const std::string& component_id);
    void MoveComponent(const std::string& component_id, float x, float y);
    
    // Connection operations
    void StartConnection(const std::string& component_id, const std::string& pin_id);
    void EndConnection(const std::string& component_id, const std::string& pin_id);
    void DeleteConnection(const std::string& connection_id);
    
    // Selection
    void SelectComponent(const std::string& component_id);
    void ClearSelection();
    Component* GetSelectedComponent();
    
    // Clipboard operations
    void Copy();
    void Cut();
    void Paste(float x, float y);
    
    // Undo/Redo
    void Undo();
    void Redo();
    bool CanUndo() const { return !undo_stack_.empty(); }
    bool CanRedo() const { return !redo_stack_.empty(); }
    
    // Preview/3D Visualization
    void EnablePreview(bool enable) { preview_enabled_ = enable; }
    bool IsPreviewEnabled() const { return preview_enabled_; }
    
    // Generate code from blueprint
    std::string GenerateCode() const;
    
    // Component library
    static std::vector<ComponentType> GetAvailableComponentTypes();
    static std::string GetComponentTypeName(ComponentType type);
    
    // Connected device integration
    bool LoadConnectedDevice(const std::string& device_name, const std::string& device_type);
    std::string GetConnectedDeviceInfo() const;
    void ClearConnectedDevice();

private:
    std::unique_ptr<Blueprint> current_blueprint_;
    std::string selected_component_id_;
    std::string connection_start_component_;
    std::string connection_start_pin_;
    bool preview_enabled_;
    
    // Undo/Redo stacks
    std::vector<std::string> undo_stack_;
    std::vector<std::string> redo_stack_;
    
    // Clipboard
    std::string clipboard_data_;
    
    // Helper functions
    std::string GenerateUniqueId(const std::string& prefix);
    void SaveState();
    int next_component_id_;
    int next_connection_id_;
    
    // Connected device tracking
    std::string connected_device_name_;
    std::string connected_device_type_;
    std::string connected_device_component_id_;
};

/**
 * BlueprintPreviewer - 3D visualization of blueprint
 */
class BlueprintPreviewer {
public:
    BlueprintPreviewer();
    ~BlueprintPreviewer() = default;

    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Set blueprint to preview
    void SetBlueprint(const Blueprint* blueprint);
    
    // Rendering
    void Render();
    
    // Camera control
    void RotateCamera(float yaw, float pitch);
    void ZoomCamera(float delta);
    void ResetCamera();
    
    // View modes
    enum class ViewMode {
        SCHEMATIC_2D,
        ISOMETRIC_3D,
        PERSPECTIVE_3D,
        PHYSICAL_3D
    };
    
    void SetViewMode(ViewMode mode) { view_mode_ = mode; }
    ViewMode GetViewMode() const { return view_mode_; }
    
    // Highlighting
    void HighlightComponent(const std::string& component_id);
    void ClearHighlight();

private:
    const Blueprint* blueprint_;
    ViewMode view_mode_;
    std::string highlighted_component_;
    
    // Camera state
    float camera_yaw_;
    float camera_pitch_;
    float camera_zoom_;
    float camera_distance_;
    
    // Rendering helpers
    void RenderComponent2D(const Component* component);
    void RenderComponent3D(const Component* component);
    void RenderConnection(const Connection* connection);
};

} // namespace blueprint
} // namespace esp32_ide

#endif // ESP32_IDE_BLUEPRINT_EDITOR_H
