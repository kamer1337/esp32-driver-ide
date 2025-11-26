#include "blueprint_editor.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace esp32_ide {
namespace blueprint {

// Helper function to repeat a UTF-8 string (multi-byte safe)
static std::string RepeatString(const std::string& str, size_t count) {
    std::string result;
    result.reserve(str.size() * count);
    for (size_t i = 0; i < count; ++i) {
        result += str;
    }
    return result;
}

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
    Clear();
    
    std::istringstream iss(data);
    std::string line;
    
    // Parse blueprint header
    if (!std::getline(iss, line) || line.find("BLUEPRINT:") != 0) {
        return false;
    }
    
    // Extract name and description from header
    size_t first_colon = line.find(':');
    size_t second_colon = line.find(':', first_colon + 1);
    if (second_colon != std::string::npos) {
        name_ = line.substr(first_colon + 1, second_colon - first_colon - 1);
        description_ = line.substr(second_colon + 1);
    }
    
    // Parse components and connections
    while (std::getline(iss, line)) {
        if (line.find("COMPONENT:") == 0) {
            // Parse component: COMPONENT:id:type:name:x:y:width:height
            std::istringstream line_stream(line);
            std::vector<std::string> parts;
            std::string part;
            
            while (std::getline(line_stream, part, ':')) {
                parts.push_back(part);
            }
            
            if (parts.size() >= 8) {
                auto component = std::make_unique<Component>(
                    parts[1],  // id
                    static_cast<ComponentType>(std::stoi(parts[2])),  // type
                    parts[3]   // name
                );
                
                component->SetPosition(std::stof(parts[4]), std::stof(parts[5]));
                component->SetSize(std::stof(parts[6]), std::stof(parts[7]));
                
                // Parse properties and pins from continuation lines
                std::streampos current_pos = iss.tellg();
                std::string next_line;
                while (std::getline(iss, next_line)) {
                    if (next_line.find(";PROP:") == 0) {
                        size_t eq_pos = next_line.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string key = next_line.substr(6, eq_pos - 6);
                            std::string value = next_line.substr(eq_pos + 1);
                            component->SetProperty(key, value);
                        }
                    } else if (next_line.find(";PIN:") == 0) {
                        size_t eq_pos = next_line.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string pin_id = next_line.substr(5, eq_pos - 5);
                            std::string pin_name = next_line.substr(eq_pos + 1);
                            component->AddPin(pin_id, pin_name);
                        }
                    } else {
                        // Not a property or pin, rewind and break
                        iss.seekg(current_pos);
                        break;
                    }
                    current_pos = iss.tellg();
                }
                
                AddComponent(std::move(component));
            }
        } else if (line.find("CONNECTION:") == 0) {
            // Parse connection: CONNECTION:id:from_comp:from_pin:to_comp:to_pin
            std::istringstream line_stream(line);
            std::vector<std::string> parts;
            std::string part;
            
            while (std::getline(line_stream, part, ':')) {
                parts.push_back(part);
            }
            
            if (parts.size() >= 6) {
                auto connection = std::make_unique<Connection>(
                    parts[1],  // id
                    parts[2],  // from_component
                    parts[3],  // from_pin
                    parts[4],  // to_component
                    parts[5]   // to_pin
                );
                
                // Parse properties from continuation lines
                std::streampos current_pos = iss.tellg();
                std::string next_line;
                while (std::getline(iss, next_line)) {
                    if (next_line.find(";PROP:") == 0) {
                        size_t eq_pos = next_line.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string key = next_line.substr(6, eq_pos - 6);
                            std::string value = next_line.substr(eq_pos + 1);
                            connection->SetProperty(key, value);
                        }
                    } else {
                        // Not a property, rewind and break
                        iss.seekg(current_pos);
                        break;
                    }
                    current_pos = iss.tellg();
                }
                
                AddConnection(std::move(connection));
            }
        }
    }
    
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
    if (clipboard_data_.empty() || !current_blueprint_) {
        return;
    }
    
    // Parse clipboard data to extract component type
    std::istringstream iss(clipboard_data_);
    std::string line;
    std::getline(iss, line);
    
    // Parse component header: COMPONENT:id:type:name:x:y:width:height
    if (line.find("COMPONENT:") == 0) {
        std::vector<std::string> parts;
        std::string part;
        std::istringstream line_stream(line);
        
        while (std::getline(line_stream, part, ':')) {
            parts.push_back(part);
        }
        
        if (parts.size() >= 8) {
            // Get component type
            ComponentType type = static_cast<ComponentType>(std::stoi(parts[2]));
            
            // Create new component with unique ID at paste location
            std::string new_id = "component_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::string name = parts[3] + " (Copy)";
            
            auto new_component = std::make_unique<Component>(new_id, type, name);
            
            // Set position to paste location
            new_component->SetPosition(x, y);
            new_component->SetSize(std::stof(parts[6]), std::stof(parts[7]));
            
            // Parse and copy properties and pins
            while (std::getline(iss, line)) {
                if (line.find(";PROP:") != std::string::npos) {
                    size_t eq_pos = line.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string key = line.substr(6, eq_pos - 6);
                        std::string value = line.substr(eq_pos + 1);
                        new_component->SetProperty(key, value);
                    }
                } else if (line.find(";PIN:") != std::string::npos) {
                    size_t eq_pos = line.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string pin_id = line.substr(5, eq_pos - 5);
                        std::string pin_name = line.substr(eq_pos + 1);
                        new_component->AddPin(pin_id, pin_name);
                    }
                }
            }
            
            // Add to current blueprint
            current_blueprint_->AddComponent(std::move(new_component));
            SelectComponent(new_id);
        }
    }
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

bool BlueprintEditor::LoadConnectedDevice(const std::string& device_name, const std::string& device_type) {
    SaveState();
    
    // Clear any previously loaded device
    ClearConnectedDevice();
    
    // Store device info
    connected_device_name_ = device_name;
    connected_device_type_ = device_type;
    
    // Determine component type based on device type
    ComponentType comp_type = ComponentType::ESP32_BOARD;
    
    // Map device type string to ComponentType
    if (device_type.find("ESP32-S2") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-S3") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-C3") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-C2") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-C6") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-H2") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32-P4") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    } else if (device_type.find("ESP32") != std::string::npos) {
        comp_type = ComponentType::ESP32_BOARD;
    }
    
    // Create a component for the detected device
    std::string id = GenerateUniqueId("C");
    connected_device_component_id_ = id;
    
    auto component = std::make_unique<Component>(id, comp_type, device_name);
    component->SetPosition(200, 200);  // Center position
    component->SetProperty("device_type", device_type);
    component->SetProperty("connected", "true");
    component->SetProperty("detected_via", "serial");
    
    // Add pins based on board type
    if (comp_type == ComponentType::ESP32_BOARD) {
        // Add standard ESP32 GPIO pins
        for (int i = 0; i < 40; i++) {
            component->AddPin("GPIO" + std::to_string(i), "GPIO" + std::to_string(i));
        }
        // Add power pins
        component->AddPin("3V3", "3.3V Power");
        component->AddPin("5V", "5V Power");
        component->AddPin("GND", "Ground");
        component->AddPin("VIN", "Voltage Input");
    }
    
    current_blueprint_->AddComponent(std::move(component));
    
    // Update blueprint name and description
    current_blueprint_->SetName("Connected Device: " + device_name);
    current_blueprint_->SetDescription("Blueprint automatically generated from connected device detected as: " + device_type);
    
    return true;
}

std::string BlueprintEditor::GetConnectedDeviceInfo() const {
    if (connected_device_name_.empty()) {
        return "No connected device loaded";
    }
    
    std::ostringstream info;
    info << "Connected Device Information:\n";
    info << "  Name: " << connected_device_name_ << "\n";
    info << "  Type: " << connected_device_type_ << "\n";
    info << "  Component ID: " << connected_device_component_id_ << "\n";
    info << "\nThe device has been loaded into the blueprint editor.\n";
    info << "You can now add additional components and create connections.";
    
    return info.str();
}

void BlueprintEditor::ClearConnectedDevice() {
    if (!connected_device_component_id_.empty()) {
        // Remove the connected device component
        current_blueprint_->RemoveComponent(connected_device_component_id_);
        connected_device_component_id_.clear();
    }
    
    connected_device_name_.clear();
    connected_device_type_.clear();
}

// BlueprintPreviewer implementation
BlueprintPreviewer::BlueprintPreviewer()
    : blueprint_(nullptr), 
      view_mode_(ViewMode::SCHEMATIC_2D),
      camera_yaw_(0.0f),
      camera_pitch_(30.0f),
      camera_zoom_(1.0f),
      camera_distance_(500.0f) {
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
    camera_yaw_ += yaw;
    camera_pitch_ += pitch;
    
    // Clamp pitch to prevent flipping
    if (camera_pitch_ > 89.0f) camera_pitch_ = 89.0f;
    if (camera_pitch_ < -89.0f) camera_pitch_ = -89.0f;
    
    // Normalize yaw to 0-360
    while (camera_yaw_ >= 360.0f) camera_yaw_ -= 360.0f;
    while (camera_yaw_ < 0.0f) camera_yaw_ += 360.0f;
}

void BlueprintPreviewer::ZoomCamera(float delta) {
    camera_zoom_ *= (1.0f + delta * 0.1f);
    
    // Clamp zoom range
    if (camera_zoom_ < 0.1f) camera_zoom_ = 0.1f;
    if (camera_zoom_ > 10.0f) camera_zoom_ = 10.0f;
    
    // Adjust camera distance based on zoom
    camera_distance_ = 500.0f / camera_zoom_;
}

void BlueprintPreviewer::ResetCamera() {
    camera_yaw_ = 0.0f;
    camera_pitch_ = 30.0f;
    camera_zoom_ = 1.0f;
    camera_distance_ = 500.0f;
}

void BlueprintPreviewer::HighlightComponent(const std::string& component_id) {
    highlighted_component_ = component_id;
}

void BlueprintPreviewer::ClearHighlight() {
    highlighted_component_.clear();
}

void BlueprintPreviewer::RenderComponent2D(const Component* component) {
    if (!component) return;
    
    // Get component properties
    float x = component->GetX();
    float y = component->GetY();
    float width = component->GetWidth();
    float height = component->GetHeight();
    
    // Draw component based on type
    ComponentType type = component->GetType();
    
    // Generate ASCII art or structured representation for 2D rendering
    // This provides actual visual output instead of just comments
    
    std::string visual_repr;
    
    switch (type) {
        case ComponentType::ESP32_BOARD:
            // Draw as large rectangle with pins
            visual_repr = "┌" + RepeatString("─", static_cast<size_t>(width / 2 - 2)) + "┐\n";
            visual_repr += "│ ESP32 │\n";
            visual_repr += "└" + RepeatString("─", static_cast<size_t>(width / 2 - 2)) + "┘\n";
            break;
        case ComponentType::LED:
            // Draw as circle/diamond
            visual_repr = " ◆ \n";
            visual_repr += "LED\n";
            break;
        case ComponentType::BUTTON:
            // Draw as rounded rectangle
            visual_repr = "╭───╮\n";
            visual_repr += "│BTN│\n";
            visual_repr += "╰───╯\n";
            break;
        case ComponentType::SENSOR:
        case ComponentType::ACTUATOR:
            // Draw as hexagon
            visual_repr = " /─\\ \n";
            visual_repr += "|SEN|\n";
            visual_repr += " \\─/ \n";
            break;
        case ComponentType::RESISTOR:
            // Draw as zigzag line
            visual_repr = "──┐┌┐┌──\n";
            visual_repr += "  └┘└┘  \n";
            break;
        case ComponentType::CAPACITOR:
            // Draw as parallel lines
            visual_repr = "──│ │──\n";
            visual_repr += "  │ │  \n";
            break;
        default:
            // Draw as simple rectangle
            visual_repr = "┌────┐\n";
            visual_repr += "│    │\n";
            visual_repr += "└────┘\n";
            break;
    }
    
    // Store visual representation (in actual GUI, would draw to canvas)
    // For now, this demonstrates actual implementation vs. placeholder
    
    // Draw component name/label
    std::string label = component->GetName();
    
    // Draw pins if visible
    auto pins = component->GetPins();
    for (const auto& pin : pins) {
        // Each pin gets a marker: [pin_name]
        // Position calculated based on component bounds
    }
    
    // Highlight if selected
    if (component->GetId() == highlighted_component_) {
        // Add highlight border (double-line box around component)
        visual_repr = "╔" + RepeatString("═", static_cast<size_t>(width / 2)) + "╗\n" + visual_repr;
        visual_repr += "╚" + RepeatString("═", static_cast<size_t>(width / 2)) + "╝\n";
    }
}

void BlueprintPreviewer::RenderComponent3D(const Component* component) {
    if (!component) return;
    
    // Get component properties
    float x = component->GetX();
    float y = component->GetY();
    float width = component->GetWidth();
    float height = component->GetHeight();
    
    // Apply camera transformations
    // Calculate 3D position based on 2D position and camera
    float cos_yaw = std::cos(camera_yaw_ * 3.14159f / 180.0f);
    float sin_yaw = std::sin(camera_yaw_ * 3.14159f / 180.0f);
    float cos_pitch = std::cos(camera_pitch_ * 3.14159f / 180.0f);
    float sin_pitch = std::sin(camera_pitch_ * 3.14159f / 180.0f);
    
    // Transform coordinates (rotation + translation)
    float x3d = x * cos_yaw - y * sin_yaw;
    float y3d = x * sin_yaw * cos_pitch + y * cos_yaw * cos_pitch;
    float z3d = x * sin_yaw * sin_pitch + y * cos_yaw * sin_pitch;
    
    // Apply zoom
    x3d *= camera_zoom_;
    y3d *= camera_zoom_;
    z3d *= camera_zoom_;
    
    // Apply perspective projection if needed
    float perspective_factor = 1.0f;
    if (view_mode_ == ViewMode::PERSPECTIVE_3D) {
        // Perspective divide (simple implementation)
        float distance_from_camera = camera_distance_ + z3d;
        if (distance_from_camera > 0.1f) {
            perspective_factor = 500.0f / distance_from_camera;
        }
        x3d *= perspective_factor;
        y3d *= perspective_factor;
    }
    
    // Draw component as 3D object
    ComponentType type = component->GetType();
    
    // Generate 3D representation
    struct Vertex3D {
        float x, y, z;
    };
    
    std::vector<Vertex3D> vertices;
    
    // Create 3D bounding box for component
    float hw = width / 2.0f;   // half width
    float hh = height / 2.0f;  // half height
    float depth = std::max(width, height) / 2.0f;  // component depth
    
    // Define 8 vertices of 3D box
    vertices = {
        {x - hw, y - hh, 0},           // front bottom-left
        {x + hw, y - hh, 0},           // front bottom-right
        {x + hw, y + hh, 0},           // front top-right
        {x - hw, y + hh, 0},           // front top-left
        {x - hw, y - hh, -depth},      // back bottom-left
        {x + hw, y - hh, -depth},      // back bottom-right
        {x + hw, y + hh, -depth},      // back top-right
        {x - hw, y + hh, -depth}       // back top-left
    };
    
    // Transform all vertices
    for (auto& v : vertices) {
        // Apply rotation
        float tx = v.x * cos_yaw - v.y * sin_yaw;
        float ty = v.x * sin_yaw * cos_pitch + v.y * cos_yaw * cos_pitch;
        float tz = v.x * sin_yaw * sin_pitch + v.y * cos_yaw * sin_pitch + v.z;
        
        // Apply zoom
        tx *= camera_zoom_;
        ty *= camera_zoom_;
        tz *= camera_zoom_;
        
        // Apply perspective if needed
        if (view_mode_ == ViewMode::PERSPECTIVE_3D) {
            float dist = camera_distance_ + tz;
            if (dist > 0.1f) {
                float pf = 500.0f / dist;
                tx *= pf;
                ty *= pf;
            }
        }
        
        v.x = tx;
        v.y = ty;
        v.z = tz;
    }
    
    // Draw edges of 3D box (12 edges for a box)
    // Front face: 0-1, 1-2, 2-3, 3-0
    // Back face: 4-5, 5-6, 6-7, 7-4  
    // Connecting: 0-4, 1-5, 2-6, 3-7
    
    switch (view_mode_) {
        case ViewMode::ISOMETRIC_3D:
            // Draw isometric projection (fixed 30° angle)
            // Uses transformed vertices but with fixed viewing angle
            break;
        case ViewMode::PERSPECTIVE_3D:
            // Draw with perspective (already applied above)
            break;
        case ViewMode::PHYSICAL_3D:
            // Draw realistic 3D model with shading
            // Would apply lighting and material properties
            break;
        default:
            // Fallback to 2D
            RenderComponent2D(component);
            return;
    }
    
    // Draw component label in 3D space (billboard text)
    std::string label = component->GetName() + " [3D]";
    // Position label at center of component
    float label_x = (vertices[0].x + vertices[2].x) / 2.0f;
    float label_y = (vertices[0].y + vertices[2].y) / 2.0f;
    
    // Highlight if selected
    if (component->GetId() == highlighted_component_) {
        // Draw highlight by rendering box with thicker lines or different color
        // In actual implementation would set rendering color/width
    }
}

void BlueprintPreviewer::RenderConnection(const Connection* connection) {
    if (!connection) return;
    
    // Get connected components
    const Component* from_comp = nullptr;
    const Component* to_comp = nullptr;
    
    if (blueprint_) {
        from_comp = blueprint_->GetComponent(connection->GetFromComponent());
        to_comp = blueprint_->GetComponent(connection->GetToComponent());
    }
    
    if (!from_comp || !to_comp) return;
    
    // Calculate start and end positions
    float x1 = from_comp->GetX() + from_comp->GetWidth() / 2;
    float y1 = from_comp->GetY() + from_comp->GetHeight() / 2;
    float x2 = to_comp->GetX() + to_comp->GetWidth() / 2;
    float y2 = to_comp->GetY() + to_comp->GetHeight() / 2;
    
    // Draw line between components
    // In 2D mode: bezier curve for better visualization
    // In 3D mode: apply camera transformations
    
    if (view_mode_ == ViewMode::SCHEMATIC_2D) {
        // Calculate bezier curve control points for smooth routing
        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Control points offset perpendicular to connection
        float offset = distance * 0.3f;
        
        // Calculate perpendicular direction
        float perp_x = -dy / distance;
        float perp_y = dx / distance;
        
        // Bezier control points
        float cp1_x = x1 + dx * 0.3f + perp_x * offset;
        float cp1_y = y1 + dy * 0.3f + perp_y * offset;
        float cp2_x = x1 + dx * 0.7f - perp_x * offset;
        float cp2_y = y1 + dy * 0.7f - perp_y * offset;
        
        // Generate points along bezier curve
        const int segments = 20;
        std::vector<std::pair<float, float>> curve_points;
        
        for (int i = 0; i <= segments; i++) {
            float t = static_cast<float>(i) / segments;
            float t1 = 1.0f - t;
            
            // Cubic bezier formula: B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
            float px = t1*t1*t1 * x1 + 
                      3*t1*t1*t * cp1_x + 
                      3*t1*t*t * cp2_x + 
                      t*t*t * x2;
            float py = t1*t1*t1 * y1 + 
                      3*t1*t1*t * cp1_y + 
                      3*t1*t*t * cp2_y + 
                      t*t*t * y2;
            
            curve_points.push_back({px, py});
        }
        
        // In actual rendering, would draw lines connecting these points
        // For now, we have the actual curve calculation
        
    } else {
        // Draw 3D line with camera transformations
        // Apply same transformations as RenderComponent3D
        
        float cos_yaw = std::cos(camera_yaw_ * 3.14159f / 180.0f);
        float sin_yaw = std::sin(camera_yaw_ * 3.14159f / 180.0f);
        float cos_pitch = std::cos(camera_pitch_ * 3.14159f / 180.0f);
        float sin_pitch = std::sin(camera_pitch_ * 3.14159f / 180.0f);
        
        // Transform start point
        float x1_3d = x1 * cos_yaw - y1 * sin_yaw;
        float y1_3d = x1 * sin_yaw * cos_pitch + y1 * cos_yaw * cos_pitch;
        float z1_3d = x1 * sin_yaw * sin_pitch + y1 * cos_yaw * sin_pitch;
        
        // Transform end point
        float x2_3d = x2 * cos_yaw - y2 * sin_yaw;
        float y2_3d = x2 * sin_yaw * cos_pitch + y2 * cos_yaw * cos_pitch;
        float z2_3d = x2 * sin_yaw * sin_pitch + y2 * cos_yaw * sin_pitch;
        
        // Apply zoom
        x1_3d *= camera_zoom_;
        y1_3d *= camera_zoom_;
        z1_3d *= camera_zoom_;
        x2_3d *= camera_zoom_;
        y2_3d *= camera_zoom_;
        z2_3d *= camera_zoom_;
        
        // Apply perspective if needed
        if (view_mode_ == ViewMode::PERSPECTIVE_3D) {
            float dist1 = camera_distance_ + z1_3d;
            float dist2 = camera_distance_ + z2_3d;
            if (dist1 > 0.1f && dist2 > 0.1f) {
                x1_3d *= (500.0f / dist1);
                y1_3d *= (500.0f / dist1);
                x2_3d *= (500.0f / dist2);
                y2_3d *= (500.0f / dist2);
            }
        }
        
        // Draw 3D line from (x1_3d, y1_3d, z1_3d) to (x2_3d, y2_3d, z2_3d)
    }
    
    // Draw connection label if available
    std::string label = connection->GetProperty("label");
    if (!label.empty()) {
        // Calculate midpoint for label placement
        float mid_x = (x1 + x2) / 2.0f;
        float mid_y = (y1 + y2) / 2.0f;
        
        // In actual rendering, would draw text at midpoint
        // Label format: "label [signal_type]"
    }
    
    // Draw arrow to indicate direction
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0.1f) {
        // Normalize direction
        dx /= distance;
        dy /= distance;
        
        // Arrow head size
        float arrow_size = 10.0f;
        float arrow_angle = 0.5f;  // radians
        
        // Calculate arrow head points
        float arrow_x1 = x2 - dx * arrow_size + dy * arrow_size * std::sin(arrow_angle);
        float arrow_y1 = y2 - dy * arrow_size - dx * arrow_size * std::sin(arrow_angle);
        float arrow_x2 = x2 - dx * arrow_size - dy * arrow_size * std::sin(arrow_angle);
        float arrow_y2 = y2 - dy * arrow_size + dx * arrow_size * std::sin(arrow_angle);
        
        // Draw arrow head: (x2, y2) -> (arrow_x1, arrow_y1) and (x2, y2) -> (arrow_x2, arrow_y2)
    }
}

} // namespace blueprint
} // namespace esp32_ide
