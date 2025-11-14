#ifndef ESP32_IDE_DEVICE_LIBRARY_H
#define ESP32_IDE_DEVICE_LIBRARY_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace esp32_ide {
namespace gui {

/**
 * @brief Device types supported by the library
 */
enum class DeviceType {
    ESP32_BOARD,
    SENSOR,
    ACTUATOR,
    DISPLAY,
    COMMUNICATION,
    POWER,
    CUSTOM
};

/**
 * @brief Device parameter definition
 */
struct DeviceParameter {
    std::string name;
    std::string type;        // "string", "int", "float", "bool", "enum"
    std::string default_value;
    std::vector<std::string> enum_values;  // For enum types
    std::string description;
    bool required;
    
    DeviceParameter() : required(false) {}
};

/**
 * @brief Device definition in the library
 */
class DeviceDefinition {
public:
    DeviceDefinition(const std::string& id, const std::string& name, DeviceType type);
    ~DeviceDefinition() = default;
    
    // Basic properties
    const std::string& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    DeviceType GetType() const { return type_; }
    const std::string& GetDescription() const { return description_; }
    const std::string& GetManufacturer() const { return manufacturer_; }
    const std::string& GetVersion() const { return version_; }
    
    void SetDescription(const std::string& desc) { description_ = desc; }
    void SetManufacturer(const std::string& mfr) { manufacturer_ = mfr; }
    void SetVersion(const std::string& ver) { version_ = ver; }
    
    // Parameters
    void AddParameter(const DeviceParameter& param);
    const std::vector<DeviceParameter>& GetParameters() const { return parameters_; }
    DeviceParameter* GetParameter(const std::string& name);
    
    // Preview data (for visualization)
    void SetPreviewData(const std::string& data) { preview_data_ = data; }
    const std::string& GetPreviewData() const { return preview_data_; }
    
    // Code templates
    void SetInitCode(const std::string& code) { init_code_ = code; }
    void SetLoopCode(const std::string& code) { loop_code_ = code; }
    const std::string& GetInitCode() const { return init_code_; }
    const std::string& GetLoopCode() const { return loop_code_; }
    
    // Pins
    void AddPin(const std::string& pin_name, const std::string& description);
    std::vector<std::pair<std::string, std::string>> GetPins() const { return pins_; }

private:
    std::string id_;
    std::string name_;
    DeviceType type_;
    std::string description_;
    std::string manufacturer_;
    std::string version_;
    std::vector<DeviceParameter> parameters_;
    std::string preview_data_;
    std::string init_code_;
    std::string loop_code_;
    std::vector<std::pair<std::string, std::string>> pins_;
};

/**
 * @brief Device instance with configured parameters
 */
class DeviceInstance {
public:
    DeviceInstance(const DeviceDefinition* definition, const std::string& instance_id);
    ~DeviceInstance() = default;
    
    const std::string& GetInstanceId() const { return instance_id_; }
    const DeviceDefinition* GetDefinition() const { return definition_; }
    
    // Parameter values
    void SetParameterValue(const std::string& param_name, const std::string& value);
    std::string GetParameterValue(const std::string& param_name) const;
    const std::map<std::string, std::string>& GetParameterValues() const { return parameter_values_; }
    
    // Code generation
    std::string GenerateInitCode() const;
    std::string GenerateLoopCode() const;
    
    // Validation
    bool Validate(std::string& error_message) const;

private:
    std::string instance_id_;
    const DeviceDefinition* definition_;
    std::map<std::string, std::string> parameter_values_;
    
    std::string ReplaceParameters(const std::string& template_code) const;
};

/**
 * @brief Device library manager
 */
class DeviceLibrary {
public:
    DeviceLibrary();
    ~DeviceLibrary();
    
    // Initialization
    void Initialize();
    void LoadDefaultDevices();
    
    // Device definition management
    void AddDevice(std::unique_ptr<DeviceDefinition> device);
    void RemoveDevice(const std::string& device_id);
    const DeviceDefinition* GetDevice(const std::string& device_id) const;
    std::vector<const DeviceDefinition*> GetAllDevices() const;
    std::vector<const DeviceDefinition*> GetDevicesByType(DeviceType type) const;
    
    // Device instance management
    DeviceInstance* CreateInstance(const std::string& device_id, const std::string& instance_id);
    void RemoveInstance(const std::string& instance_id);
    DeviceInstance* GetInstance(const std::string& instance_id);
    std::vector<DeviceInstance*> GetAllInstances();
    
    // Import/Export
    bool ImportFromFile(const std::string& filename);
    bool ExportToFile(const std::string& filename) const;
    
    // Search
    std::vector<const DeviceDefinition*> SearchDevices(const std::string& query) const;
    
    // Categories
    std::vector<std::string> GetCategories() const;
    std::vector<const DeviceDefinition*> GetDevicesByCategory(const std::string& category) const;

private:
    std::map<std::string, std::unique_ptr<DeviceDefinition>> devices_;
    std::map<std::string, std::unique_ptr<DeviceInstance>> instances_;
    
    // Default device templates
    void AddDefaultBoards();
    void AddDefaultSensors();
    void AddDefaultActuators();
    void AddDefaultDisplays();
    void AddDefaultCommunication();
};

/**
 * @brief Device library preview widget
 */
class DeviceLibraryPreview {
public:
    DeviceLibraryPreview();
    ~DeviceLibraryPreview() = default;
    
    // Set device to preview
    void SetDevice(const DeviceDefinition* device);
    void SetInstance(const DeviceInstance* instance);
    
    // Preview modes
    enum class PreviewMode {
        SCHEMATIC,      // 2D schematic view
        PHYSICAL,       // Physical appearance
        PINOUT,         // Pin diagram
        CODE            // Generated code
    };
    
    void SetPreviewMode(PreviewMode mode) { preview_mode_ = mode; }
    PreviewMode GetPreviewMode() const { return preview_mode_; }
    
    // Render preview (returns text representation for now)
    std::string RenderPreview() const;
    
    // Get device info
    std::string GetDeviceInfo() const;
    std::string GetParameterInfo() const;
    std::string GetPinInfo() const;

private:
    const DeviceDefinition* device_;
    const DeviceInstance* instance_;
    PreviewMode preview_mode_;
    
    std::string RenderSchematic() const;
    std::string RenderPhysical() const;
    std::string RenderPinout() const;
    std::string RenderCode() const;
};

} // namespace gui
} // namespace esp32_ide

#endif // ESP32_IDE_DEVICE_LIBRARY_H
