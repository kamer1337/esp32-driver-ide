#include "blueprint_editor.h"
#include <sstream>
#include <fstream>
#include <algorithm>

namespace esp32_ide {
namespace blueprint {

// Component implementation
Component::Component(const std::string& id, ComponentType type, const std::string& name)
    : id_(id), type_(type), name_(name), x_(0), y_(0), width_(100), height_(60) {
}

void Component::SetProperty(const std::string& key, const std::string& value) {
    properties_[key] = value;
}

std::string Component::GetProperty(const std::string& key) const {
    auto it = properties_.find(key);
    return (it != properties_.end()) ? it->second : "";
}

void Component::AddPin(const std::string& pin_id, const std::string& pin_name) {
    pins_[pin_id] = pin_name;
}

std::vector<std::string> Component::GetPins() const {
    std::vector<std::string> pin_list;
    for (const auto& pin : pins_) {
        pin_list.push_back(pin.first);
    }
    return pin_list;
}

std::string Component::Serialize() const {
    std::ostringstream oss;
    oss << "COMPONENT:" << id_ << ":" << static_cast<int>(type_) << ":" << name_
        << ":" << x_ << ":" << y_ << ":" << width_ << ":" << height_;
    
    for (const auto& prop : properties_) {
        oss << ";PROP:" << prop.first << "=" << prop.second;
    }
    
    for (const auto& pin : pins_) {
        oss << ";PIN:" << pin.first << "=" << pin.second;
    }
    
    return oss.str();
}

// Connection implementation
Connection::Connection(const std::string& id,
                       const std::string& from_component, const std::string& from_pin,
                       const std::string& to_component, const std::string& to_pin)
    : id_(id), from_component_(from_component), from_pin_(from_pin),
      to_component_(to_component), to_pin_(to_pin) {
}

void Connection::SetProperty(const std::string& key, const std::string& value) {
    properties_[key] = value;
}

std::string Connection::GetProperty(const std::string& key) const {
    auto it = properties_.find(key);
    return (it != properties_.end()) ? it->second : "";
}

std::string Connection::Serialize() const {
    std::ostringstream oss;
    oss << "CONNECTION:" << id_ << ":" << from_component_ << ":" << from_pin_
        << ":" << to_component_ << ":" << to_pin_;
    
    for (const auto& prop : properties_) {
        oss << ";PROP:" << prop.first << "=" << prop.second;
    }
    
    return oss.str();
}

// Blueprint implementation
Blueprint::Blueprint() : name_("Untitled Blueprint") {
}

void Blueprint::AddComponent(std::unique_ptr<Component> component) {
    components_.push_back(std::move(component));
}

void Blueprint::RemoveComponent(const std::string& component_id) {
    components_.erase(
        std::remove_if(components_.begin(), components_.end(),
                      [&](const std::unique_ptr<Component>& c) {
                          return c->GetId() == component_id;
                      }),
        components_.end()
    );
    
    // Remove connections involving this component
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&](const std::unique_ptr<Connection>& conn) {
                          return conn->GetFromComponent() == component_id ||
                                 conn->GetToComponent() == component_id;
                      }),
        connections_.end()
    );
}

Component* Blueprint::GetComponent(const std::string& component_id) {
    for (auto& component : components_) {
        if (component->GetId() == component_id) {
            return component.get();
        }
    }
    return nullptr;
}

const Component* Blueprint::GetComponent(const std::string& component_id) const {
    for (const auto& component : components_) {
        if (component->GetId() == component_id) {
            return component.get();
        }
    }
    return nullptr;
}

void Blueprint::AddConnection(std::unique_ptr<Connection> connection) {
    connections_.push_back(std::move(connection));
}

void Blueprint::RemoveConnection(const std::string& connection_id) {
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [&](const std::unique_ptr<Connection>& c) {
                          return c->GetId() == connection_id;
                      }),
        connections_.end()
    );
}

Connection* Blueprint::GetConnection(const std::string& connection_id) {
    for (auto& connection : connections_) {
        if (connection->GetId() == connection_id) {
            return connection.get();
        }
    }
    return nullptr;
}

bool Blueprint::Validate(std::string& error_message) const {
    // Check for duplicate component IDs
    std::map<std::string, int> id_counts;
    for (const auto& component : components_) {
        id_counts[component->GetId()]++;
    }
    
    for (const auto& count : id_counts) {
        if (count.second > 1) {
            error_message = "Duplicate component ID: " + count.first;
            return false;
        }
    }
    
    // Validate connections
    for (const auto& connection : connections_) {
        if (GetComponent(connection->GetFromComponent()) == nullptr) {
            error_message = "Connection references non-existent component: " + 
                          connection->GetFromComponent();
            return false;
        }
        if (GetComponent(connection->GetToComponent()) == nullptr) {
            error_message = "Connection references non-existent component: " + 
                          connection->GetToComponent();
            return false;
        }
    }
    
    return true;
}

std::string Blueprint::Serialize() const {
    std::ostringstream oss;
    oss << "BLUEPRINT\n";
    oss << "NAME:" << name_ << "\n";
    oss << "DESCRIPTION:" << description_ << "\n";
    oss << "COMPONENTS:" << components_.size() << "\n";
    
    for (const auto& component : components_) {
        oss << component->Serialize() << "\n";
    }
    
    oss << "CONNECTIONS:" << connections_.size() << "\n";
    for (const auto& connection : connections_) {
        oss << connection->Serialize() << "\n";
    }
    
    return oss.str();
}

bool Blueprint::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << Serialize();
    return true;
}

bool Blueprint::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return Deserialize(content);
}

bool Blueprint::Deserialize(const std::string& data) {
    // Basic deserialization (simplified)
    Clear();
    // TODO: Implement full deserialization
    return true;
}

void Blueprint::Clear() {
    components_.clear();
    connections_.clear();
    name_ = "Untitled Blueprint";
    description_ = "";
}

// BlueprintEditor implementation
BlueprintEditor::BlueprintEditor()
    : preview_enabled_(false), next_component_id_(1), next_connection_id_(1) {
    current_blueprint_ = std::make_unique<Blueprint>();
}

bool BlueprintEditor::Initialize() {
    return true;
}

void BlueprintEditor::Shutdown() {
    current_blueprint_.reset();
}

void BlueprintEditor::NewBlueprint() {
    SaveState();
    current_blueprint_ = std::make_unique<Blueprint>();
    selected_component_id_.clear();
}

bool BlueprintEditor::LoadBlueprint(const std::string& filename) {
    auto new_blueprint = std::make_unique<Blueprint>();
    if (new_blueprint->LoadFromFile(filename)) {
        SaveState();
        current_blueprint_ = std::move(new_blueprint);
        return true;
    }
    return false;
}

bool BlueprintEditor::SaveBlueprint(const std::string& filename) {
    return current_blueprint_->SaveToFile(filename);
}

void BlueprintEditor::AddComponent(ComponentType type, float x, float y) {
    SaveState();
    
    std::string id = GenerateUniqueId("C");
    std::string name = GetComponentTypeName(type);
    
    auto component = std::make_unique<Component>(id, type, name);
    component->SetPosition(x, y);
    
    // Add default pins based on component type
    if (type == ComponentType::ESP32_BOARD) {
        for (int i = 0; i < 40; i++) {
            component->AddPin("GPIO" + std::to_string(i), "GPIO" + std::to_string(i));
        }
    } else if (type == ComponentType::LED) {
        component->AddPin("ANODE", "Anode (+)");
        component->AddPin("CATHODE", "Cathode (-)");
    } else if (type == ComponentType::BUTTON) {
        component->AddPin("PIN1", "Pin 1");
        component->AddPin("PIN2", "Pin 2");
    }
    
    current_blueprint_->AddComponent(std::move(component));
}

void BlueprintEditor::DeleteComponent(const std::string& component_id) {
    SaveState();
    current_blueprint_->RemoveComponent(component_id);
    if (selected_component_id_ == component_id) {
        selected_component_id_.clear();
    }
}

void BlueprintEditor::MoveComponent(const std::string& component_id, float x, float y) {
    Component* component = current_blueprint_->GetComponent(component_id);
    if (component) {
        component->SetPosition(x, y);
    }
}

void BlueprintEditor::StartConnection(const std::string& component_id, const std::string& pin_id) {
    connection_start_component_ = component_id;
    connection_start_pin_ = pin_id;
}

void BlueprintEditor::EndConnection(const std::string& component_id, const std::string& pin_id) {
    if (!connection_start_component_.empty()) {
        SaveState();
        
        std::string id = GenerateUniqueId("CONN");
        auto connection = std::make_unique<Connection>(
            id, connection_start_component_, connection_start_pin_,
            component_id, pin_id
        );
        
        current_blueprint_->AddConnection(std::move(connection));
        
        connection_start_component_.clear();
        connection_start_pin_.clear();
    }
}

void BlueprintEditor::DeleteConnection(const std::string& connection_id) {
    SaveState();
    current_blueprint_->RemoveConnection(connection_id);
}

void BlueprintEditor::SelectComponent(const std::string& component_id) {
    selected_component_id_ = component_id;
}

void BlueprintEditor::ClearSelection() {
    selected_component_id_.clear();
}

Component* BlueprintEditor::GetSelectedComponent() {
    if (!selected_component_id_.empty()) {
        return current_blueprint_->GetComponent(selected_component_id_);
    }
    return nullptr;
}

void BlueprintEditor::Copy() {
    if (!selected_component_id_.empty()) {
        Component* component = current_blueprint_->GetComponent(selected_component_id_);
        if (component) {
            clipboard_data_ = component->Serialize();
        }
    }
}

void BlueprintEditor::Cut() {
    Copy();
    if (!selected_component_id_.empty()) {
        DeleteComponent(selected_component_id_);
    }
}

void BlueprintEditor::Paste(float x, float y) {
    // TODO: Implement paste functionality
}

void BlueprintEditor::Undo() {
    if (!undo_stack_.empty()) {
        redo_stack_.push_back(current_blueprint_->Serialize());
        current_blueprint_->Deserialize(undo_stack_.back());
        undo_stack_.pop_back();
    }
}

void BlueprintEditor::Redo() {
    if (!redo_stack_.empty()) {
        undo_stack_.push_back(current_blueprint_->Serialize());
        current_blueprint_->Deserialize(redo_stack_.back());
        redo_stack_.pop_back();
    }
}

std::string BlueprintEditor::GenerateCode() const {
    std::ostringstream code;
    
    code << "// Generated from blueprint: " << current_blueprint_->GetName() << "\n";
    code << "// " << current_blueprint_->GetDescription() << "\n\n";
    
    code << "void setup() {\n";
    code << "  Serial.begin(115200);\n";
    
    // Generate pin modes and setup code
    for (const auto& component : current_blueprint_->GetComponents()) {
        if (component->GetType() == ComponentType::LED) {
            std::string pin = component->GetProperty("pin");
            if (!pin.empty()) {
                code << "  pinMode(" << pin << ", OUTPUT); // " 
                     << component->GetName() << "\n";
            }
        } else if (component->GetType() == ComponentType::BUTTON) {
            std::string pin = component->GetProperty("pin");
            if (!pin.empty()) {
                code << "  pinMode(" << pin << ", INPUT_PULLUP); // " 
                     << component->GetName() << "\n";
            }
        }
    }
    
    code << "}\n\n";
    code << "void loop() {\n";
    code << "  // TODO: Add your code here\n";
    code << "  delay(10);\n";
    code << "}\n";
    
    return code.str();
}

std::vector<ComponentType> BlueprintEditor::GetAvailableComponentTypes() {
    return {
        ComponentType::ESP32_BOARD,
        ComponentType::GPIO_PIN,
        ComponentType::SENSOR,
        ComponentType::ACTUATOR,
        ComponentType::LED,
        ComponentType::BUTTON,
        ComponentType::MOTOR,
        ComponentType::DISPLAY,
        ComponentType::COMMUNICATION_MODULE,
        ComponentType::POWER_SUPPLY,
        ComponentType::RESISTOR,
        ComponentType::CAPACITOR
    };
}

std::string BlueprintEditor::GetComponentTypeName(ComponentType type) {
    switch (type) {
        case ComponentType::ESP32_BOARD: return "ESP32 Board";
        case ComponentType::GPIO_PIN: return "GPIO Pin";
        case ComponentType::SENSOR: return "Sensor";
        case ComponentType::ACTUATOR: return "Actuator";
        case ComponentType::LED: return "LED";
        case ComponentType::BUTTON: return "Button";
        case ComponentType::MOTOR: return "Motor";
        case ComponentType::DISPLAY: return "Display";
        case ComponentType::COMMUNICATION_MODULE: return "Communication Module";
        case ComponentType::POWER_SUPPLY: return "Power Supply";
        case ComponentType::RESISTOR: return "Resistor";
        case ComponentType::CAPACITOR: return "Capacitor";
        case ComponentType::CUSTOM: return "Custom Component";
        default: return "Unknown";
    }
}

std::string BlueprintEditor::GenerateUniqueId(const std::string& prefix) {
    if (prefix == "C") {
        return prefix + std::to_string(next_component_id_++);
    } else {
        return prefix + std::to_string(next_connection_id_++);
    }
}

void BlueprintEditor::SaveState() {
    undo_stack_.push_back(current_blueprint_->Serialize());
    redo_stack_.clear();
    
    // Limit undo stack size
    if (undo_stack_.size() > 50) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

// BlueprintPreviewer implementation
BlueprintPreviewer::BlueprintPreviewer()
    : blueprint_(nullptr), view_mode_(ViewMode::SCHEMATIC_2D) {
}

bool BlueprintPreviewer::Initialize() {
    return true;
}

void BlueprintPreviewer::Shutdown() {
    blueprint_ = nullptr;
}

void BlueprintPreviewer::SetBlueprint(const Blueprint* blueprint) {
    blueprint_ = blueprint;
}

void BlueprintPreviewer::Render() {
    if (!blueprint_) return;
    
    // Render based on view mode
    for (const auto& component : blueprint_->GetComponents()) {
        if (view_mode_ == ViewMode::SCHEMATIC_2D) {
            RenderComponent2D(component.get());
        } else {
            RenderComponent3D(component.get());
        }
    }
    
    for (const auto& connection : blueprint_->GetConnections()) {
        RenderConnection(connection.get());
    }
}

void BlueprintPreviewer::RotateCamera(float yaw, float pitch) {
    // TODO: Implement camera rotation
}

void BlueprintPreviewer::ZoomCamera(float delta) {
    // TODO: Implement camera zoom
}

void BlueprintPreviewer::ResetCamera() {
    // TODO: Implement camera reset
}

void BlueprintPreviewer::HighlightComponent(const std::string& component_id) {
    highlighted_component_ = component_id;
}

void BlueprintPreviewer::ClearHighlight() {
    highlighted_component_.clear();
}

void BlueprintPreviewer::RenderComponent2D(const Component* component) {
    // TODO: Implement 2D rendering
}

void BlueprintPreviewer::RenderComponent3D(const Component* component) {
    // TODO: Implement 3D rendering using PureCRenderer
}

void BlueprintPreviewer::RenderConnection(const Connection* connection) {
    // TODO: Implement connection rendering
}

} // namespace blueprint
} // namespace esp32_ide
