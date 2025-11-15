#include "utils/ml_device_detector.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>

namespace esp32_ide {
namespace ml {

MLDeviceDetector::MLDeviceDetector() {
    model_ = std::make_unique<PretrainedModel>();
}

MLDeviceDetector::~MLDeviceDetector() = default;

void MLDeviceDetector::SetDetectionCallback(DetectionCallback callback) {
    detection_callback_ = callback;
}

MLDeviceDetector::DetectionResult MLDeviceDetector::DetectDevice(
    const std::string& port, 
    int baud_rate
) {
    DetectionResult result;
    result.success = false;
    
    try {
        // Extract features from device
        PretrainedModel::FeatureVector features = ExtractFeatures(port, baud_rate);
        
        // Use pretrained model to predict device type
        PretrainedModel::DeviceType device_type = model_->Predict(features);
        float confidence = model_->GetConfidence(features, device_type);
        
        // Populate result
        result.device_type = device_type;
        result.confidence = confidence;
        result.device_name = PretrainedModel::GetDeviceTypeName(device_type);
        result.success = (device_type != PretrainedModel::DeviceType::UNKNOWN);
        
        // Generate details
        std::ostringstream details;
        details << "Detected device: " << result.device_name << "\n";
        details << "Confidence: " << (confidence * 100.0f) << "%\n";
        details << "Port: " << port << "\n";
        details << "Baud Rate: " << baud_rate;
        result.details = details.str();
        
        // Call callback if set
        if (detection_callback_) {
            detection_callback_(result);
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.device_type = PretrainedModel::DeviceType::UNKNOWN;
        result.confidence = 0.0f;
        result.device_name = "Unknown";
        result.details = std::string("Detection failed: ") + e.what();
    }
    
    return result;
}

MLDeviceDetector::DetectionResult MLDeviceDetector::DetectFromCharacteristics(
    const std::string& boot_message,
    size_t memory_size_kb,
    float response_time_ms,
    const std::string& chip_id
) {
    DetectionResult result;
    result.success = false;
    
    try {
        // Parse additional info from boot message
        size_t parsed_memory = memory_size_kb;
        bool has_wifi = false;
        bool has_bluetooth = false;
        float flash_mb = 4.0f;
        
        ParseDeviceInfo(boot_message, parsed_memory, has_wifi, has_bluetooth, flash_mb);
        
        // If memory wasn't provided, use parsed value
        if (memory_size_kb == 0) {
            memory_size_kb = parsed_memory;
        }
        
        // Extract features
        PretrainedModel::FeatureVector features = ExtractFeaturesFromData(
            boot_message,
            memory_size_kb,
            response_time_ms,
            chip_id,
            has_wifi,
            has_bluetooth,
            flash_mb
        );
        
        // Predict
        PretrainedModel::DeviceType device_type = model_->Predict(features);
        float confidence = model_->GetConfidence(features, device_type);
        
        // Populate result
        result.device_type = device_type;
        result.confidence = confidence;
        result.device_name = PretrainedModel::GetDeviceTypeName(device_type);
        result.success = (device_type != PretrainedModel::DeviceType::UNKNOWN);
        
        // Generate details
        std::ostringstream details;
        details << "Detected device: " << result.device_name << "\n";
        details << "Confidence: " << (confidence * 100.0f) << "%\n";
        details << "Memory: " << memory_size_kb << " KB\n";
        details << "WiFi: " << (has_wifi ? "Yes" : "No") << "\n";
        details << "Bluetooth: " << (has_bluetooth ? "Yes" : "No");
        result.details = details.str();
        
        // Call callback if set
        if (detection_callback_) {
            detection_callback_(result);
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.device_type = PretrainedModel::DeviceType::UNKNOWN;
        result.confidence = 0.0f;
        result.device_name = "Unknown";
        result.details = std::string("Detection failed: ") + e.what();
    }
    
    return result;
}

PretrainedModel::FeatureVector MLDeviceDetector::ExtractFeatures(
    const std::string& port,
    int baud_rate
) {
    // Simulate reading from serial port
    std::string boot_message = SimulateSerialRead(port, baud_rate);
    
    // Parse device characteristics
    size_t memory_kb = 520;  // Default ESP32 memory
    bool has_wifi = true;
    bool has_bluetooth = true;
    float flash_mb = 4.0f;
    
    ParseDeviceInfo(boot_message, memory_kb, has_wifi, has_bluetooth, flash_mb);
    
    // Measure response time (simulated)
    float response_time_ms = 150.0f;  // Typical ESP32 response time
    
    // Extract chip ID pattern (simulated)
    std::string chip_id = "0x1234ABCD";
    
    return ExtractFeaturesFromData(
        boot_message,
        memory_kb,
        response_time_ms,
        chip_id,
        has_wifi,
        has_bluetooth,
        flash_mb
    );
}

PretrainedModel::FeatureVector MLDeviceDetector::ExtractFeaturesFromData(
    const std::string& boot_message,
    size_t memory_size_kb,
    float response_time_ms,
    const std::string& chip_id,
    bool has_wifi,
    bool has_bluetooth,
    float flash_size_mb
) {
    PretrainedModel::FeatureVector features;
    
    // Feature 1: Baud rate score (assuming 115200 is standard)
    features.baud_rate_score = 1.0f;  // Would be calculated from actual baud rate
    
    // Feature 2: Response time
    features.response_time_ms = response_time_ms;
    
    // Feature 3: Memory size
    features.memory_size_kb = static_cast<float>(memory_size_kb);
    
    // Feature 4: Boot pattern matching
    features.boot_pattern_match = AnalyzeBootPattern(boot_message);
    
    // Feature 5: Chip ID pattern
    features.chip_id_pattern = ExtractChipIdPattern(chip_id);
    
    // Feature 6: WiFi capability
    features.wifi_capability = has_wifi ? 1.0f : 0.0f;
    
    // Feature 7: Bluetooth capability
    features.bluetooth_capability = has_bluetooth ? 1.0f : 0.0f;
    
    // Feature 8: Flash size
    features.flash_size_mb = flash_size_mb;
    
    return features;
}

float MLDeviceDetector::AnalyzeBootPattern(const std::string& boot_message) {
    // Analyze boot message patterns to extract signature
    float score = 0.0f;
    
    // Check for common ESP32 boot patterns
    if (boot_message.find("ESP32") != std::string::npos) {
        score += 0.3f;
    }
    if (boot_message.find("ESP32-S2") != std::string::npos) {
        score += 0.4f;
    }
    if (boot_message.find("ESP32-S3") != std::string::npos) {
        score += 0.5f;
    }
    if (boot_message.find("ESP32-C3") != std::string::npos) {
        score += 0.6f;
    }
    if (boot_message.find("ets") != std::string::npos) {
        score += 0.2f;
    }
    if (boot_message.find("boot:") != std::string::npos) {
        score += 0.1f;
    }
    
    return std::min(score, 1.0f);
}

float MLDeviceDetector::ExtractChipIdPattern(const std::string& chip_id) {
    // Extract pattern from chip ID
    // Different ESP32 variants have different chip ID patterns
    if (chip_id.empty()) {
        return 0.5f;  // Unknown
    }
    
    // Simple hashing to get a normalized value
    size_t hash = 0;
    for (char c : chip_id) {
        hash = hash * 31 + static_cast<size_t>(c);
    }
    
    // Normalize to 0-1 range
    return static_cast<float>(hash % 1000) / 1000.0f;
}

float MLDeviceDetector::CalculateBaudRateScore(int baud_rate) {
    // Common ESP32 baud rates: 9600, 115200, 230400, 460800, 921600
    std::vector<int> common_rates = {9600, 115200, 230400, 460800, 921600};
    
    for (int rate : common_rates) {
        if (baud_rate == rate) {
            return 1.0f;
        }
    }
    
    // Calculate proximity to nearest common rate
    int min_diff = std::abs(baud_rate - common_rates[0]);
    for (int rate : common_rates) {
        int diff = std::abs(baud_rate - rate);
        if (diff < min_diff) {
            min_diff = diff;
        }
    }
    
    // Score based on proximity
    return 1.0f / (1.0f + static_cast<float>(min_diff) / 10000.0f);
}

std::string MLDeviceDetector::SimulateSerialRead(const std::string& port, int baud_rate) {
    // In a real implementation, this would read from the actual serial port
    // For now, simulate different device responses based on port
    
    // Simulate different devices on different ports
    if (port.find("USB0") != std::string::npos || port.find("COM3") != std::string::npos) {
        return "rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)\n"
               "ESP32 chip revision 3\n"
               "2 cores, WiFi/BT/BLE\n"
               "Flash: 4MB\n";
    } else if (port.find("USB1") != std::string::npos || port.find("COM4") != std::string::npos) {
        return "rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)\n"
               "ESP32-S3 chip revision 0\n"
               "2 cores, WiFi/BLE\n"
               "Flash: 8MB\n";
    } else if (port.find("ACM") != std::string::npos) {
        return "rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)\n"
               "ESP32-C3 chip revision 3\n"
               "1 core, WiFi/BLE\n"
               "Flash: 4MB\n";
    } else {
        return "rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)\n"
               "ESP32 chip\n"
               "WiFi enabled\n";
    }
}

void MLDeviceDetector::ParseDeviceInfo(
    const std::string& message,
    size_t& memory_kb,
    bool& has_wifi,
    bool& has_bluetooth,
    float& flash_mb
) {
    // Parse WiFi capability
    has_wifi = (message.find("WiFi") != std::string::npos);
    
    // Parse Bluetooth capability
    has_bluetooth = (message.find("BT") != std::string::npos || 
                     message.find("BLE") != std::string::npos ||
                     message.find("Bluetooth") != std::string::npos);
    
    // Parse flash size
    if (message.find("Flash: 8MB") != std::string::npos) {
        flash_mb = 8.0f;
    } else if (message.find("Flash: 4MB") != std::string::npos) {
        flash_mb = 4.0f;
    } else if (message.find("Flash: 2MB") != std::string::npos) {
        flash_mb = 2.0f;
    }
    
    // Parse memory - ESP32 variants have different memory sizes
    if (message.find("ESP32-S3") != std::string::npos) {
        memory_kb = 512;  // ESP32-S3 has 512KB SRAM
    } else if (message.find("ESP32-S2") != std::string::npos) {
        memory_kb = 320;  // ESP32-S2 has 320KB SRAM
    } else if (message.find("ESP32-C3") != std::string::npos) {
        memory_kb = 400;  // ESP32-C3 has 400KB SRAM
    } else if (message.find("ESP32") != std::string::npos) {
        memory_kb = 520;  // ESP32 has 520KB SRAM
    }
}

} // namespace ml
} // namespace esp32_ide
