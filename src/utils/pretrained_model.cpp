#include "utils/pretrained_model.h"
#include <algorithm>
#include <numeric>

namespace esp32_ide {
namespace ml {

PretrainedModel::PretrainedModel() {
    InitializeWeights();
}

void PretrainedModel::InitializeWeights() {
    // Initialize pretrained weights
    // These weights are "trained" to recognize ESP32 device patterns
    // In a real scenario, these would come from actual ML training
    
    // Input to hidden layer weights [8 x 16]
    weights_input_hidden_ = {
        // Weights for each input feature to hidden neurons
        {0.8f, -0.3f, 0.5f, 0.2f, -0.1f, 0.4f, 0.7f, -0.2f, 0.3f, 0.1f, -0.4f, 0.6f, 0.2f, -0.5f, 0.3f, 0.4f},  // baud_rate_score
        {0.3f, 0.6f, -0.2f, 0.5f, 0.4f, -0.3f, 0.2f, 0.7f, -0.1f, 0.5f, 0.3f, -0.2f, 0.6f, 0.1f, -0.4f, 0.3f},  // response_time
        {0.9f, 0.4f, -0.6f, 0.3f, 0.7f, -0.2f, 0.5f, 0.2f, -0.3f, 0.8f, 0.1f, -0.4f, 0.6f, 0.3f, -0.5f, 0.2f},  // memory_size
        {0.5f, -0.4f, 0.7f, 0.3f, -0.2f, 0.6f, 0.1f, -0.5f, 0.8f, 0.2f, -0.3f, 0.4f, 0.5f, -0.6f, 0.3f, 0.7f},  // boot_pattern
        {0.7f, 0.3f, -0.5f, 0.6f, 0.4f, -0.2f, 0.8f, 0.1f, -0.4f, 0.5f, 0.3f, -0.6f, 0.2f, 0.7f, -0.3f, 0.4f},  // chip_id
        {0.4f, -0.6f, 0.3f, 0.8f, 0.2f, -0.4f, 0.5f, 0.7f, -0.2f, 0.3f, 0.6f, -0.5f, 0.4f, 0.2f, -0.7f, 0.5f},  // wifi
        {0.6f, 0.2f, -0.4f, 0.5f, 0.7f, -0.3f, 0.4f, 0.3f, -0.5f, 0.6f, 0.2f, -0.7f, 0.5f, 0.4f, -0.2f, 0.8f},  // bluetooth
        {0.5f, -0.3f, 0.6f, 0.4f, -0.5f, 0.7f, 0.2f, -0.6f, 0.5f, 0.3f, -0.4f, 0.8f, 0.1f, -0.5f, 0.6f, 0.3f}   // flash_size
    };
    
    // Hidden layer biases [16]
    bias_hidden_ = {
        0.1f, -0.2f, 0.3f, -0.1f, 0.2f, -0.3f, 0.4f, -0.2f,
        0.1f, 0.3f, -0.4f, 0.2f, -0.1f, 0.3f, -0.2f, 0.1f
    };
    
    // Hidden to output layer weights [16 x 4]
    weights_hidden_output_ = {
        {0.8f, -0.3f, 0.2f, -0.5f},   // ESP32, S2, S3, C3 weights from hidden neuron 0
        {-0.4f, 0.7f, 0.3f, -0.2f},
        {0.5f, -0.2f, 0.6f, 0.3f},
        {-0.3f, 0.4f, -0.5f, 0.8f},
        {0.6f, 0.2f, -0.4f, 0.5f},
        {-0.2f, 0.5f, 0.7f, -0.3f},
        {0.7f, -0.4f, 0.3f, 0.2f},
        {-0.5f, 0.6f, -0.2f, 0.7f},
        {0.4f, 0.3f, -0.6f, 0.4f},
        {-0.3f, 0.8f, 0.4f, -0.2f},
        {0.5f, -0.2f, 0.7f, 0.3f},
        {-0.6f, 0.4f, -0.3f, 0.6f},
        {0.3f, 0.5f, -0.5f, 0.4f},
        {-0.4f, 0.2f, 0.6f, -0.5f},
        {0.6f, -0.5f, 0.4f, 0.3f},
        {-0.2f, 0.7f, -0.3f, 0.5f}
    };
    
    // Output layer biases [4]
    bias_output_ = {
        0.2f,   // ESP32
        -0.1f,  // ESP32-S2
        0.1f,   // ESP32-S3
        -0.2f   // ESP32-C3
    };
}

std::vector<float> PretrainedModel::FeaturesToInput(const FeatureVector& features) const {
    return {
        features.baud_rate_score,
        features.response_time_ms / 1000.0f,  // Normalize to seconds
        features.memory_size_kb / 512.0f,     // Normalize to ~1.0 for typical ESP32
        features.boot_pattern_match,
        features.chip_id_pattern,
        features.wifi_capability,
        features.bluetooth_capability,
        features.flash_size_mb / 4.0f         // Normalize to ~1.0 for typical 4MB flash
    };
}

std::vector<float> PretrainedModel::Forward(const FeatureVector& features) const {
    std::vector<float> input = FeaturesToInput(features);
    
    // Hidden layer
    std::vector<float> hidden(HIDDEN_SIZE, 0.0f);
    for (int h = 0; h < HIDDEN_SIZE; ++h) {
        float sum = bias_hidden_[h];
        for (int i = 0; i < INPUT_SIZE; ++i) {
            sum += input[i] * weights_input_hidden_[i][h];
        }
        hidden[h] = ReLU(sum);
    }
    
    // Output layer
    std::vector<float> output(OUTPUT_SIZE, 0.0f);
    for (int o = 0; o < OUTPUT_SIZE; ++o) {
        float sum = bias_output_[o];
        for (int h = 0; h < HIDDEN_SIZE; ++h) {
            sum += hidden[h] * weights_hidden_output_[h][o];
        }
        output[o] = sum;
    }
    
    // Apply softmax
    return Softmax(output);
}

std::vector<float> PretrainedModel::Softmax(const std::vector<float>& x) const {
    std::vector<float> result(x.size());
    float max_val = *std::max_element(x.begin(), x.end());
    
    // Subtract max for numerical stability
    float sum = 0.0f;
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = std::exp(x[i] - max_val);
        sum += result[i];
    }
    
    // Normalize
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] /= sum;
    }
    
    return result;
}

PretrainedModel::DeviceType PretrainedModel::Predict(const FeatureVector& features) const {
    std::vector<float> probabilities = Forward(features);
    
    // Find class with highest probability
    int max_idx = 0;
    float max_prob = probabilities[0];
    for (size_t i = 1; i < probabilities.size(); ++i) {
        if (probabilities[i] > max_prob) {
            max_prob = probabilities[i];
            max_idx = i;
        }
    }
    
    // Confidence threshold
    if (max_prob < 0.4f) {
        return DeviceType::UNKNOWN;
    }
    
    // Map index to device type
    switch (max_idx) {
        case 0: return DeviceType::ESP32;
        case 1: return DeviceType::ESP32_S2;
        case 2: return DeviceType::ESP32_S3;
        case 3: return DeviceType::ESP32_C3;
        default: return DeviceType::UNKNOWN;
    }
}

float PretrainedModel::GetConfidence(const FeatureVector& features, DeviceType type) const {
    std::vector<float> probabilities = Forward(features);
    
    int type_idx = -1;
    switch (type) {
        case DeviceType::ESP32: type_idx = 0; break;
        case DeviceType::ESP32_S2: type_idx = 1; break;
        case DeviceType::ESP32_S3: type_idx = 2; break;
        case DeviceType::ESP32_C3: type_idx = 3; break;
        default: return 0.0f;
    }
    
    return probabilities[type_idx];
}

std::string PretrainedModel::GetDeviceTypeName(DeviceType type) {
    switch (type) {
        case DeviceType::ESP32: return "ESP32";
        case DeviceType::ESP32_S2: return "ESP32-S2";
        case DeviceType::ESP32_S3: return "ESP32-S3";
        case DeviceType::ESP32_C3: return "ESP32-C3";
        case DeviceType::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

} // namespace ml
} // namespace esp32_ide
