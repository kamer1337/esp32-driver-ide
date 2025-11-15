# Machine Learning Device Detection with Pretrained Model

## Overview

The ESP32 Driver IDE now includes an advanced machine learning-based device detection system. This feature uses a pretrained neural network model to automatically detect and classify ESP32 device types based on their communication patterns and device characteristics.

## Features

### Pretrained Neural Network
- **Architecture**: Simple feedforward neural network
  - 8 input features
  - 16 hidden neurons with ReLU activation
  - 8 output classes with softmax activation
- **Supported Device Types**:
  - ESP32 (original)
  - ESP32-S2 (WiFi only, no Bluetooth)
  - ESP32-S3 (WiFi + BLE, dual core)
  - ESP32-C3 (WiFi + BLE, RISC-V)
  - ESP32-C2 (WiFi + BLE, cost-optimized)
  - ESP32-C6 (WiFi 6 + BLE 5 + Zigbee/Thread)
  - ESP32-H2 (BLE + Zigbee/Thread, no WiFi)
  - ESP32-P4 (High-performance, no wireless)

### Feature Extraction
The system extracts 8 key features from device communication:

1. **Baud Rate Score**: Compatibility with common ESP32 baud rates
2. **Response Time**: Average response time in milliseconds
3. **Memory Size**: Total SRAM size in kilobytes
4. **Boot Pattern Match**: Pattern matching score from boot messages
5. **Chip ID Pattern**: Characteristic patterns in chip identification
6. **WiFi Capability**: Detection of WiFi support (0.0 or 1.0)
7. **Bluetooth Capability**: Detection of Bluetooth/BLE support (0.0 or 1.0)
8. **Flash Size**: Flash memory size in megabytes

### Detection Methods

#### 1. Direct Port Detection
Detect device by connecting to a serial port:
```cpp
#include "utils/ml_device_detector.h"

MLDeviceDetector detector;
auto result = detector.DetectDevice("/dev/ttyUSB0", 115200);

if (result.success) {
    std::cout << "Detected: " << result.device_name << std::endl;
    std::cout << "Confidence: " << (result.confidence * 100) << "%" << std::endl;
}
```

#### 2. Detection from Characteristics
Detect device from parsed boot message and characteristics:
```cpp
std::string boot_msg = "ESP32-S3 chip revision 0\n2 cores, WiFi/BLE";
auto result = detector.DetectFromCharacteristics(boot_msg, 512, 120.0f);
```

#### 3. Custom Feature Extraction
Extract and use custom features:
```cpp
PretrainedModel::FeatureVector features;
features.memory_size_kb = 520.0f;
features.wifi_capability = 1.0f;
features.bluetooth_capability = 1.0f;
// ... set other features

PretrainedModel model;
auto device_type = model.Predict(features);
float confidence = model.GetConfidence(features, device_type);
```

## API Reference

### PretrainedModel Class

#### Methods

**`DeviceType Predict(const FeatureVector& features)`**
- Predicts device type from feature vector
- Returns: DeviceType enum (ESP32, ESP32_S2, ESP32_S3, ESP32_C3, ESP32_C2, ESP32_C6, ESP32_H2, ESP32_P4, UNKNOWN)

**`float GetConfidence(const FeatureVector& features, DeviceType type)`**
- Gets confidence score for a specific device type prediction
- Returns: Confidence value between 0.0 and 1.0

**`static std::string GetDeviceTypeName(DeviceType type)`**
- Converts DeviceType enum to human-readable string
- Returns: Device name as string

### MLDeviceDetector Class

#### Methods

**`DetectionResult DetectDevice(const std::string& port, int baud_rate = 115200)`**
- Detects device from serial port
- Parameters:
  - `port`: Serial port path (e.g., "/dev/ttyUSB0", "COM3")
  - `baud_rate`: Communication baud rate (default: 115200)
- Returns: DetectionResult with device type, confidence, and details

**`DetectionResult DetectFromCharacteristics(...)`**
- Detects device from parsed characteristics
- Parameters:
  - `boot_message`: Device boot message string
  - `memory_size_kb`: Memory size in kilobytes
  - `response_time_ms`: Response time in milliseconds
  - `chip_id`: Optional chip ID string
- Returns: DetectionResult with device type and confidence

**`void SetDetectionCallback(DetectionCallback callback)`**
- Sets callback function for asynchronous detection notifications
- Parameter: Callback function receiving DetectionResult

### DetectionResult Structure

```cpp
struct DetectionResult {
    PretrainedModel::DeviceType device_type;  // Detected device type
    float confidence;                          // Confidence score (0.0-1.0)
    std::string device_name;                   // Human-readable device name
    std::string details;                       // Detailed information
    bool success;                              // Detection success flag
};
```

## Usage Examples

### Example 1: Basic Device Detection

```cpp
#include "utils/ml_device_detector.h"
#include <iostream>

int main() {
    MLDeviceDetector detector;
    
    // Detect device on serial port
    auto result = detector.DetectDevice("/dev/ttyUSB0", 115200);
    
    if (result.success) {
        std::cout << "Device detected: " << result.device_name << std::endl;
        std::cout << "Confidence: " << (result.confidence * 100.0f) << "%" << std::endl;
        std::cout << "Details:\n" << result.details << std::endl;
    } else {
        std::cout << "Detection failed" << std::endl;
    }
    
    return 0;
}
```

### Example 2: Using Callbacks

```cpp
#include "utils/ml_device_detector.h"
#include <iostream>

void onDeviceDetected(const MLDeviceDetector::DetectionResult& result) {
    std::cout << "Detection callback triggered!" << std::endl;
    std::cout << "Device: " << result.device_name << std::endl;
    std::cout << "Confidence: " << (result.confidence * 100.0f) << "%" << std::endl;
}

int main() {
    MLDeviceDetector detector;
    
    // Set callback
    detector.SetDetectionCallback(onDeviceDetected);
    
    // Perform detection (callback will be triggered)
    detector.DetectDevice("/dev/ttyUSB0", 115200);
    
    return 0;
}
```

### Example 3: Detection from Boot Message

```cpp
#include "utils/ml_device_detector.h"
#include <iostream>

int main() {
    MLDeviceDetector detector;
    
    // Boot message from device
    std::string boot_msg = 
        "rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)\n"
        "ESP32-S3 chip revision 0\n"
        "2 cores, WiFi/BLE\n"
        "Flash: 8MB\n";
    
    // Detect from characteristics
    auto result = detector.DetectFromCharacteristics(
        boot_msg,
        512,      // 512KB SRAM
        120.0f,   // 120ms response time
        "0xABCD"  // Chip ID
    );
    
    if (result.success) {
        std::cout << "Detected: " << result.device_name << std::endl;
        std::cout << "Confidence: " << (result.confidence * 100.0f) << "%" << std::endl;
    }
    
    return 0;
}
```

### Example 4: Low-Level Model Usage

```cpp
#include "utils/pretrained_model.h"
#include <iostream>

int main() {
    PretrainedModel model;
    
    // Create feature vector manually
    PretrainedModel::FeatureVector features;
    features.baud_rate_score = 1.0f;
    features.response_time_ms = 150.0f;
    features.memory_size_kb = 520.0f;
    features.boot_pattern_match = 0.3f;
    features.chip_id_pattern = 0.5f;
    features.wifi_capability = 1.0f;
    features.bluetooth_capability = 1.0f;
    features.flash_size_mb = 4.0f;
    
    // Predict device type
    auto device_type = model.Predict(features);
    float confidence = model.GetConfidence(features, device_type);
    
    std::cout << "Device: " << PretrainedModel::GetDeviceTypeName(device_type) << std::endl;
    std::cout << "Confidence: " << (confidence * 100.0f) << "%" << std::endl;
    
    return 0;
}
```

## Integration with Existing Components

### Integration with Serial Monitor

The ML device detector can be integrated with the existing `SerialMonitor` class:

```cpp
#include "serial/serial_monitor.h"
#include "utils/ml_device_detector.h"

// Connect to serial port
SerialMonitor serial;
serial.Connect("/dev/ttyUSB0", 115200);

// Detect device type
MLDeviceDetector detector;
auto result = detector.DetectDevice("/dev/ttyUSB0", 115200);

if (result.success) {
    // Use detected device information
    std::cout << "Connected to: " << result.device_name << std::endl;
}
```

### Integration with ESP32 Compiler

The detected device type can inform compilation settings:

```cpp
#include "compiler/esp32_compiler.h"
#include "utils/ml_device_detector.h"

MLDeviceDetector detector;
auto result = detector.DetectDevice("/dev/ttyUSB0", 115200);

ESP32Compiler compiler;
if (result.device_type == PretrainedModel::DeviceType::ESP32_S3) {
    compiler.SetBoard(ESP32Compiler::BoardType::ESP32_S3);
} else if (result.device_type == PretrainedModel::DeviceType::ESP32_C3) {
    compiler.SetBoard(ESP32Compiler::BoardType::ESP32_C3);
}
// ... compile with correct settings
```

## Testing

A comprehensive test suite is included at `src/ml_device_detection_test.cpp`.

### Building and Running Tests

```bash
cd build
cmake ..
cmake --build . --target esp32-ml-device-detection-test
./esp32-ml-device-detection-test
```

### Test Coverage

The test suite includes:
1. **Pretrained Model Tests**: Validates neural network predictions
2. **Device Detection Tests**: Tests detection from various serial ports
3. **Characteristics Detection Tests**: Tests detection from boot messages
4. **Callback Functionality Tests**: Validates async callback mechanism

## Model Training (For Developers)

The pretrained model weights are embedded in `pretrained_model.cpp`. To retrain the model:

1. Collect feature vectors from real ESP32 devices
2. Label each sample with the correct device type
3. Train a neural network using your preferred ML framework (TensorFlow, PyTorch, etc.)
4. Export the trained weights
5. Update the weight matrices in `InitializeWeights()` method

### Weight Format

The model uses three sets of weights:
- `weights_input_hidden_` [8 × 16]: Input to hidden layer
- `bias_hidden_` [16]: Hidden layer biases
- `weights_hidden_output_` [16 × 8]: Hidden to output layer
- `bias_output_` [8]: Output layer biases

## Performance Considerations

- **Speed**: Detection typically completes in < 100ms (excluding serial communication)
- **Memory**: Model requires ~1KB for weight storage
- **Accuracy**: Confidence scores typically range from 40-70% depending on feature quality
- **Threshold**: Predictions with confidence < 40% return UNKNOWN device type

## Limitations

1. **Simulated Serial Communication**: Current implementation simulates serial port reading. For production use, integrate with actual serial port libraries.
2. **Training Data**: Weights are initialized with synthetic values. For best accuracy, train on real device data.
3. **Feature Engineering**: Current features are basic. Additional features (clock speed, peripheral detection) could improve accuracy.

## Future Enhancements

Potential improvements:
- Real serial port integration (libserial, boost::asio)
- More sophisticated feature extraction
- Support for additional ESP32 variants as they are released
- Model retraining with real device data for improved accuracy
- Confidence calibration
- Ensemble methods for improved accuracy

## Dependencies

- **STL only**: No external dependencies required
- C++17 standard library
- `<vector>`, `<string>`, `<cmath>`, `<algorithm>`, `<functional>`

## License

This feature is part of the ESP32 Driver IDE and follows the same MIT License.

## Support

For issues or questions about ML device detection:
1. Check the test suite for usage examples
2. Review API documentation above
3. Open an issue on the GitHub repository
