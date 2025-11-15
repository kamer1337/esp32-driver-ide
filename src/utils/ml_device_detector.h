#ifndef ML_DEVICE_DETECTOR_H
#define ML_DEVICE_DETECTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "utils/pretrained_model.h"

namespace esp32_ide {
namespace ml {

/**
 * @brief Machine Learning-based device detection for ESP32 devices
 * 
 * Uses a pretrained neural network model to detect and classify
 * ESP32 device types based on communication patterns and device
 * characteristics extracted from serial communication.
 */
class MLDeviceDetector {
public:
    // Detection result
    struct DetectionResult {
        PretrainedModel::DeviceType device_type;
        float confidence;
        std::string device_name;
        std::string details;
        bool success;
    };
    
    // Callback for detection completion
    using DetectionCallback = std::function<void(const DetectionResult&)>;
    
    MLDeviceDetector();
    ~MLDeviceDetector();
    
    // Detect device from serial port
    DetectionResult DetectDevice(const std::string& port, int baud_rate = 115200);
    
    // Detect device from already parsed characteristics
    DetectionResult DetectFromCharacteristics(
        const std::string& boot_message,
        size_t memory_size_kb,
        float response_time_ms,
        const std::string& chip_id = ""
    );
    
    // Extract features from device communication
    PretrainedModel::FeatureVector ExtractFeatures(
        const std::string& port,
        int baud_rate
    );
    
    // Extract features from parsed data
    PretrainedModel::FeatureVector ExtractFeaturesFromData(
        const std::string& boot_message,
        size_t memory_size_kb,
        float response_time_ms,
        const std::string& chip_id,
        bool has_wifi,
        bool has_bluetooth,
        float flash_size_mb
    );
    
    // Set callback for async detection
    void SetDetectionCallback(DetectionCallback callback);
    
    // Get the pretrained model
    const PretrainedModel& GetModel() const { return *model_; }
    
private:
    std::unique_ptr<PretrainedModel> model_;
    DetectionCallback detection_callback_;
    
    // Feature extraction helpers
    float AnalyzeBootPattern(const std::string& boot_message);
    float ExtractChipIdPattern(const std::string& chip_id);
    float CalculateBaudRateScore(int baud_rate);
    
    // Simulate serial communication for feature extraction
    // In a real implementation, this would read from actual serial port
    std::string SimulateSerialRead(const std::string& port, int baud_rate);
    
    // Parse device characteristics from boot message
    void ParseDeviceInfo(
        const std::string& message,
        size_t& memory_kb,
        bool& has_wifi,
        bool& has_bluetooth,
        float& flash_mb
    );
};

} // namespace ml
} // namespace esp32_ide

#endif // ML_DEVICE_DETECTOR_H
