#ifndef PRETRAINED_MODEL_H
#define PRETRAINED_MODEL_H

#include <vector>
#include <string>
#include <cmath>

namespace esp32_ide {
namespace ml {

/**
 * @brief Simple neural network for device classification
 * 
 * A lightweight pretrained model for detecting ESP32 device types
 * based on serial communication patterns and device characteristics.
 * 
 * Features extracted:
 * - Baud rate compatibility
 * - Response time patterns
 * - Memory characteristics
 * - Boot message patterns
 * - Chip ID patterns
 */
class PretrainedModel {
public:
    // Device types the model can classify
    enum class DeviceType {
        ESP32,
        ESP32_S2,
        ESP32_S3,
        ESP32_C3,
        UNKNOWN
    };
    
    // Feature vector for device detection (8 features)
    struct FeatureVector {
        float baud_rate_score;        // Normalized baud rate compatibility
        float response_time_ms;       // Average response time
        float memory_size_kb;         // Total memory size
        float boot_pattern_match;     // Boot message pattern matching score
        float chip_id_pattern;        // Chip ID characteristics
        float wifi_capability;        // WiFi feature detection
        float bluetooth_capability;   // Bluetooth feature detection
        float flash_size_mb;          // Flash memory size
    };
    
    PretrainedModel();
    
    // Predict device type from features
    DeviceType Predict(const FeatureVector& features) const;
    
    // Get confidence score for prediction (0.0 to 1.0)
    float GetConfidence(const FeatureVector& features, DeviceType type) const;
    
    // Get device type name
    static std::string GetDeviceTypeName(DeviceType type);
    
private:
    // Neural network structure: 8 inputs -> 16 hidden -> 4 outputs
    static const int INPUT_SIZE = 8;
    static const int HIDDEN_SIZE = 16;
    static const int OUTPUT_SIZE = 4;  // ESP32, S2, S3, C3
    
    // Pretrained weights (trained offline on device characteristics)
    // Hidden layer weights [INPUT_SIZE x HIDDEN_SIZE]
    std::vector<std::vector<float>> weights_input_hidden_;
    
    // Hidden layer biases [HIDDEN_SIZE]
    std::vector<float> bias_hidden_;
    
    // Output layer weights [HIDDEN_SIZE x OUTPUT_SIZE]
    std::vector<std::vector<float>> weights_hidden_output_;
    
    // Output layer biases [OUTPUT_SIZE]
    std::vector<float> bias_output_;
    
    // Initialize pretrained weights
    void InitializeWeights();
    
    // Activation functions
    float ReLU(float x) const { return x > 0 ? x : 0; }
    float Sigmoid(float x) const { return 1.0f / (1.0f + std::exp(-x)); }
    
    // Softmax for output layer
    std::vector<float> Softmax(const std::vector<float>& x) const;
    
    // Forward pass through network
    std::vector<float> Forward(const FeatureVector& features) const;
    
    // Convert feature vector to input array
    std::vector<float> FeaturesToInput(const FeatureVector& features) const;
};

} // namespace ml
} // namespace esp32_ide

#endif // PRETRAINED_MODEL_H
