#include "gui/device_library.h"
#include <algorithm>
#include <sstream>
#include <fstream>

namespace esp32_ide {
namespace gui {

// DeviceDefinition implementation
DeviceDefinition::DeviceDefinition(const std::string& id, const std::string& name, DeviceType type)
    : id_(id), name_(name), type_(type) {
}

void DeviceDefinition::AddParameter(const DeviceParameter& param) {
    parameters_.push_back(param);
}

DeviceParameter* DeviceDefinition::GetParameter(const std::string& name) {
    for (auto& param : parameters_) {
        if (param.name == name) {
            return &param;
        }
    }
    return nullptr;
}

void DeviceDefinition::AddPin(const std::string& pin_name, const std::string& description) {
    pins_.push_back({pin_name, description});
}

// DeviceInstance implementation
DeviceInstance::DeviceInstance(const DeviceDefinition* definition, const std::string& instance_id)
    : instance_id_(instance_id), definition_(definition) {
    // Initialize parameters with default values
    for (const auto& param : definition->GetParameters()) {
        parameter_values_[param.name] = param.default_value;
    }
}

void DeviceInstance::SetParameterValue(const std::string& param_name, const std::string& value) {
    parameter_values_[param_name] = value;
}

std::string DeviceInstance::GetParameterValue(const std::string& param_name) const {
    auto it = parameter_values_.find(param_name);
    return (it != parameter_values_.end()) ? it->second : "";
}

std::string DeviceInstance::GenerateInitCode() const {
    return ReplaceParameters(definition_->GetInitCode());
}

std::string DeviceInstance::GenerateLoopCode() const {
    return ReplaceParameters(definition_->GetLoopCode());
}

bool DeviceInstance::Validate(std::string& error_message) const {
    for (const auto& param : definition_->GetParameters()) {
        if (param.required) {
            auto it = parameter_values_.find(param.name);
            if (it == parameter_values_.end() || it->second.empty()) {
                error_message = "Required parameter '" + param.name + "' is not set";
                return false;
            }
        }
    }
    return true;
}

std::string DeviceInstance::ReplaceParameters(const std::string& template_code) const {
    std::string result = template_code;
    
    // Replace ${INSTANCE_ID}
    size_t pos = 0;
    while ((pos = result.find("${INSTANCE_ID}", pos)) != std::string::npos) {
        result.replace(pos, 14, instance_id_);
        pos += instance_id_.length();
    }
    
    // Replace parameter values
    for (const auto& param : parameter_values_) {
        std::string placeholder = "${" + param.first + "}";
        pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), param.second);
            pos += param.second.length();
        }
    }
    
    return result;
}

// DeviceLibrary implementation
DeviceLibrary::DeviceLibrary() {
}

DeviceLibrary::~DeviceLibrary() {
}

void DeviceLibrary::Initialize() {
    LoadDefaultDevices();
}

void DeviceLibrary::LoadDefaultDevices() {
    AddDefaultBoards();
    AddDefaultSensors();
    AddDefaultActuators();
    AddDefaultDisplays();
    AddDefaultCommunication();
}

void DeviceLibrary::AddDevice(std::unique_ptr<DeviceDefinition> device) {
    if (device) {
        std::string id = device->GetId();
        devices_[id] = std::move(device);
    }
}

void DeviceLibrary::RemoveDevice(const std::string& device_id) {
    devices_.erase(device_id);
}

const DeviceDefinition* DeviceLibrary::GetDevice(const std::string& device_id) const {
    auto it = devices_.find(device_id);
    return (it != devices_.end()) ? it->second.get() : nullptr;
}

std::vector<const DeviceDefinition*> DeviceLibrary::GetAllDevices() const {
    std::vector<const DeviceDefinition*> result;
    for (const auto& pair : devices_) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::vector<const DeviceDefinition*> DeviceLibrary::GetDevicesByType(DeviceType type) const {
    std::vector<const DeviceDefinition*> result;
    for (const auto& pair : devices_) {
        if (pair.second->GetType() == type) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

DeviceInstance* DeviceLibrary::CreateInstance(const std::string& device_id, const std::string& instance_id) {
    const DeviceDefinition* device = GetDevice(device_id);
    if (!device) {
        return nullptr;
    }
    
    auto instance = std::make_unique<DeviceInstance>(device, instance_id);
    DeviceInstance* ptr = instance.get();
    instances_[instance_id] = std::move(instance);
    return ptr;
}

void DeviceLibrary::RemoveInstance(const std::string& instance_id) {
    instances_.erase(instance_id);
}

DeviceInstance* DeviceLibrary::GetInstance(const std::string& instance_id) {
    auto it = instances_.find(instance_id);
    return (it != instances_.end()) ? it->second.get() : nullptr;
}

std::vector<DeviceInstance*> DeviceLibrary::GetAllInstances() {
    std::vector<DeviceInstance*> result;
    for (auto& pair : instances_) {
        result.push_back(pair.second.get());
    }
    return result;
}

bool DeviceLibrary::ImportFromFile(const std::string& filename) {
    // TODO: Implement JSON import
    return false;
}

bool DeviceLibrary::ExportToFile(const std::string& filename) const {
    // TODO: Implement JSON export
    return false;
}

std::vector<const DeviceDefinition*> DeviceLibrary::SearchDevices(const std::string& query) const {
    std::vector<const DeviceDefinition*> result;
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);
    
    for (const auto& pair : devices_) {
        std::string lower_name = pair.second->GetName();
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find(lower_query) != std::string::npos) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<std::string> DeviceLibrary::GetCategories() const {
    return {"Boards", "Sensors", "Actuators", "Displays", "Communication", "Power"};
}

std::vector<const DeviceDefinition*> DeviceLibrary::GetDevicesByCategory(const std::string& category) const {
    DeviceType type;
    if (category == "Boards") type = DeviceType::ESP32_BOARD;
    else if (category == "Sensors") type = DeviceType::SENSOR;
    else if (category == "Actuators") type = DeviceType::ACTUATOR;
    else if (category == "Displays") type = DeviceType::DISPLAY;
    else if (category == "Communication") type = DeviceType::COMMUNICATION;
    else if (category == "Power") type = DeviceType::POWER;
    else return {};
    
    return GetDevicesByType(type);
}

void DeviceLibrary::AddDefaultBoards() {
    // ESP32 Dev Board
    auto esp32 = std::make_unique<DeviceDefinition>("esp32_dev", "ESP32 DevKit", DeviceType::ESP32_BOARD);
    esp32->SetDescription("Standard ESP32 development board with WiFi and Bluetooth");
    esp32->SetManufacturer("Espressif");
    esp32->SetVersion("1.0");
    
    DeviceParameter board_type;
    board_type.name = "board_type";
    board_type.type = "enum";
    board_type.enum_values = {"ESP32", "ESP32-S2", "ESP32-S3", "ESP32-C3"};
    board_type.default_value = "ESP32";
    board_type.description = "ESP32 board variant";
    board_type.required = true;
    esp32->AddParameter(board_type);
    
    esp32->AddPin("GPIO0", "GPIO 0 - Boot button");
    esp32->AddPin("GPIO2", "GPIO 2 - Built-in LED");
    esp32->AddPin("3V3", "3.3V Power");
    esp32->AddPin("GND", "Ground");
    
    esp32->SetInitCode("// ESP32 ${INSTANCE_ID} initialization\nvoid setup() {\n  Serial.begin(115200);\n}");
    esp32->SetLoopCode("// ESP32 ${INSTANCE_ID} loop\nvoid loop() {\n  delay(1000);\n}");
    
    AddDevice(std::move(esp32));
}

void DeviceLibrary::AddDefaultSensors() {
    // DHT22 Temperature/Humidity Sensor
    auto dht22 = std::make_unique<DeviceDefinition>("dht22", "DHT22 Temp/Humidity", DeviceType::SENSOR);
    dht22->SetDescription("Digital temperature and humidity sensor");
    dht22->SetManufacturer("Aosong");
    dht22->SetVersion("1.0");
    
    DeviceParameter pin;
    pin.name = "pin";
    pin.type = "int";
    pin.default_value = "4";
    pin.description = "GPIO pin connected to DHT22 data pin";
    pin.required = true;
    dht22->AddParameter(pin);
    
    dht22->AddPin("VCC", "Power (3.3V-5V)");
    dht22->AddPin("DATA", "Data pin");
    dht22->AddPin("GND", "Ground");
    
    dht22->SetInitCode("#include <DHT.h>\nDHT ${INSTANCE_ID}(${pin}, DHT22);\nvoid setup() {\n  ${INSTANCE_ID}.begin();\n}");
    dht22->SetLoopCode("float temp = ${INSTANCE_ID}.readTemperature();\nfloat humid = ${INSTANCE_ID}.readHumidity();");
    
    AddDevice(std::move(dht22));
}

void DeviceLibrary::AddDefaultActuators() {
    // Servo Motor
    auto servo = std::make_unique<DeviceDefinition>("servo", "Servo Motor", DeviceType::ACTUATOR);
    servo->SetDescription("Standard hobby servo motor");
    servo->SetManufacturer("Generic");
    servo->SetVersion("1.0");
    
    DeviceParameter pin;
    pin.name = "pin";
    pin.type = "int";
    pin.default_value = "5";
    pin.description = "GPIO pin for PWM control";
    pin.required = true;
    servo->AddParameter(pin);
    
    servo->AddPin("VCC", "Power (5V)");
    servo->AddPin("SIGNAL", "PWM signal");
    servo->AddPin("GND", "Ground");
    
    servo->SetInitCode("#include <Servo.h>\nServo ${INSTANCE_ID};\nvoid setup() {\n  ${INSTANCE_ID}.attach(${pin});\n}");
    servo->SetLoopCode("${INSTANCE_ID}.write(90); // Set angle");
    
    AddDevice(std::move(servo));
}

void DeviceLibrary::AddDefaultDisplays() {
    // OLED Display
    auto oled = std::make_unique<DeviceDefinition>("oled_ssd1306", "OLED SSD1306", DeviceType::DISPLAY);
    oled->SetDescription("128x64 OLED display with I2C interface");
    oled->SetManufacturer("Generic");
    oled->SetVersion("1.0");
    
    DeviceParameter addr;
    addr.name = "i2c_address";
    addr.type = "string";
    addr.default_value = "0x3C";
    addr.description = "I2C address (usually 0x3C or 0x3D)";
    addr.required = true;
    oled->AddParameter(addr);
    
    oled->AddPin("VCC", "Power (3.3V)");
    oled->AddPin("GND", "Ground");
    oled->AddPin("SCL", "I2C Clock");
    oled->AddPin("SDA", "I2C Data");
    
    oled->SetInitCode("#include <Wire.h>\n#include <Adafruit_SSD1306.h>\nAdafruit_SSD1306 ${INSTANCE_ID}(128, 64, &Wire, -1);\nvoid setup() {\n  ${INSTANCE_ID}.begin(SSD1306_SWITCHCAPVCC, ${i2c_address});\n}");
    oled->SetLoopCode("${INSTANCE_ID}.clearDisplay();\n${INSTANCE_ID}.display();");
    
    AddDevice(std::move(oled));
}

void DeviceLibrary::AddDefaultCommunication() {
    // Bluetooth Module
    auto bluetooth = std::make_unique<DeviceDefinition>("hc05", "HC-05 Bluetooth", DeviceType::COMMUNICATION);
    bluetooth->SetDescription("Bluetooth serial communication module");
    bluetooth->SetManufacturer("Generic");
    bluetooth->SetVersion("1.0");
    
    DeviceParameter baud;
    baud.name = "baud_rate";
    baud.type = "int";
    baud.default_value = "9600";
    baud.description = "Baud rate for serial communication";
    baud.required = true;
    bluetooth->AddParameter(baud);
    
    bluetooth->AddPin("VCC", "Power (5V)");
    bluetooth->AddPin("GND", "Ground");
    bluetooth->AddPin("TXD", "Transmit");
    bluetooth->AddPin("RXD", "Receive");
    
    bluetooth->SetInitCode("#include <BluetoothSerial.h>\nBluetoothSerial ${INSTANCE_ID};\nvoid setup() {\n  ${INSTANCE_ID}.begin(\"ESP32\");\n}");
    bluetooth->SetLoopCode("if (${INSTANCE_ID}.available()) {\n  char c = ${INSTANCE_ID}.read();\n}");
    
    AddDevice(std::move(bluetooth));
}

// DeviceLibraryPreview implementation
DeviceLibraryPreview::DeviceLibraryPreview()
    : device_(nullptr), instance_(nullptr), preview_mode_(PreviewMode::SCHEMATIC) {
}

void DeviceLibraryPreview::SetDevice(const DeviceDefinition* device) {
    device_ = device;
    instance_ = nullptr;
}

void DeviceLibraryPreview::SetInstance(const DeviceInstance* instance) {
    instance_ = instance;
    if (instance) {
        device_ = instance->GetDefinition();
    }
}

std::string DeviceLibraryPreview::RenderPreview() const {
    if (!device_) {
        return "No device selected";
    }
    
    switch (preview_mode_) {
        case PreviewMode::SCHEMATIC:
            return RenderSchematic();
        case PreviewMode::PHYSICAL:
            return RenderPhysical();
        case PreviewMode::PINOUT:
            return RenderPinout();
        case PreviewMode::CODE:
            return RenderCode();
        default:
            return "";
    }
}

std::string DeviceLibraryPreview::GetDeviceInfo() const {
    if (!device_) return "";
    
    std::ostringstream oss;
    oss << "Device: " << device_->GetName() << "\n";
    oss << "ID: " << device_->GetId() << "\n";
    oss << "Manufacturer: " << device_->GetManufacturer() << "\n";
    oss << "Version: " << device_->GetVersion() << "\n";
    oss << "Description: " << device_->GetDescription() << "\n";
    return oss.str();
}

std::string DeviceLibraryPreview::GetParameterInfo() const {
    if (!device_) return "";
    
    std::ostringstream oss;
    oss << "Parameters:\n";
    for (const auto& param : device_->GetParameters()) {
        oss << "  - " << param.name << " (" << param.type << ")";
        if (param.required) oss << " [Required]";
        oss << "\n";
        oss << "    " << param.description << "\n";
        oss << "    Default: " << param.default_value << "\n";
        
        if (instance_) {
            oss << "    Current: " << instance_->GetParameterValue(param.name) << "\n";
        }
    }
    return oss.str();
}

std::string DeviceLibraryPreview::GetPinInfo() const {
    if (!device_) return "";
    
    std::ostringstream oss;
    oss << "Pins:\n";
    for (const auto& pin : device_->GetPins()) {
        oss << "  - " << pin.first << ": " << pin.second << "\n";
    }
    return oss.str();
}

std::string DeviceLibraryPreview::RenderSchematic() const {
    std::ostringstream oss;
    oss << "╔════════════════════════╗\n";
    oss << "║  " << device_->GetName() << std::string(22 - device_->GetName().length(), ' ') << "║\n";
    oss << "╠════════════════════════╣\n";
    
    auto pins = device_->GetPins();
    for (size_t i = 0; i < pins.size(); ++i) {
        oss << "║ " << pins[i].first << std::string(22 - pins[i].first.length(), ' ') << "║\n";
    }
    
    oss << "╚════════════════════════╝\n";
    return oss.str();
}

std::string DeviceLibraryPreview::RenderPhysical() const {
    return "[Physical preview of " + device_->GetName() + "]\n" +
           "Width: 50mm\nHeight: 30mm\nDepth: 10mm\n";
}

std::string DeviceLibraryPreview::RenderPinout() const {
    return GetPinInfo();
}

std::string DeviceLibraryPreview::RenderCode() const {
    if (!instance_) {
        return "// No instance configured\n// " + device_->GetName() + " template code:\n\n" +
               device_->GetInitCode() + "\n\n" + device_->GetLoopCode();
    }
    
    return "// Generated code for instance: " + instance_->GetInstanceId() + "\n\n" +
           "// Initialization:\n" + instance_->GenerateInitCode() + "\n\n" +
           "// Loop code:\n" + instance_->GenerateLoopCode();
}

} // namespace gui
} // namespace esp32_ide
