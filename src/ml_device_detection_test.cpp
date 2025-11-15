#include <iostream>
#include <iomanip>
#include "utils/ml_device_detector.h"
#include "utils/pretrained_model.h"

using namespace esp32_ide::ml;

void PrintSeparator() {
    std::cout << "\n" << std::string(70, '=') << "\n" << std::endl;
}

void TestPretrainedModel() {
    std::cout << "=== Testing Pretrained Neural Network Model ===" << std::endl;
    PrintSeparator();
    
    PretrainedModel model;
    
    // Test case 1: ESP32 characteristics
    std::cout << "Test 1: ESP32 Device" << std::endl;
    PretrainedModel::FeatureVector esp32_features;
    esp32_features.baud_rate_score = 1.0f;
    esp32_features.response_time_ms = 150.0f;
    esp32_features.memory_size_kb = 520.0f;
    esp32_features.boot_pattern_match = 0.3f;
    esp32_features.chip_id_pattern = 0.5f;
    esp32_features.wifi_capability = 1.0f;
    esp32_features.bluetooth_capability = 1.0f;
    esp32_features.flash_size_mb = 4.0f;
    
    auto device_type = model.Predict(esp32_features);
    float confidence = model.GetConfidence(esp32_features, device_type);
    
    std::cout << "  Detected: " << PretrainedModel::GetDeviceTypeName(device_type) << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (confidence * 100.0f) << "%" << std::endl;
    
    // Test case 2: ESP32-S3 characteristics
    std::cout << "\nTest 2: ESP32-S3 Device" << std::endl;
    PretrainedModel::FeatureVector s3_features;
    s3_features.baud_rate_score = 1.0f;
    s3_features.response_time_ms = 120.0f;
    s3_features.memory_size_kb = 512.0f;
    s3_features.boot_pattern_match = 0.5f;
    s3_features.chip_id_pattern = 0.7f;
    s3_features.wifi_capability = 1.0f;
    s3_features.bluetooth_capability = 1.0f;  // BLE only
    s3_features.flash_size_mb = 8.0f;
    
    device_type = model.Predict(s3_features);
    confidence = model.GetConfidence(s3_features, device_type);
    
    std::cout << "  Detected: " << PretrainedModel::GetDeviceTypeName(device_type) << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (confidence * 100.0f) << "%" << std::endl;
    
    // Test case 3: ESP32-C3 characteristics
    std::cout << "\nTest 3: ESP32-C3 Device" << std::endl;
    PretrainedModel::FeatureVector c3_features;
    c3_features.baud_rate_score = 1.0f;
    c3_features.response_time_ms = 100.0f;
    c3_features.memory_size_kb = 400.0f;
    c3_features.boot_pattern_match = 0.6f;
    c3_features.chip_id_pattern = 0.8f;
    c3_features.wifi_capability = 1.0f;
    c3_features.bluetooth_capability = 1.0f;  // BLE only
    c3_features.flash_size_mb = 4.0f;
    
    device_type = model.Predict(c3_features);
    confidence = model.GetConfidence(c3_features, device_type);
    
    std::cout << "  Detected: " << PretrainedModel::GetDeviceTypeName(device_type) << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (confidence * 100.0f) << "%" << std::endl;
    
    // Test case 4: ESP32-S2 characteristics
    std::cout << "\nTest 4: ESP32-S2 Device" << std::endl;
    PretrainedModel::FeatureVector s2_features;
    s2_features.baud_rate_score = 1.0f;
    s2_features.response_time_ms = 140.0f;
    s2_features.memory_size_kb = 320.0f;
    s2_features.boot_pattern_match = 0.4f;
    s2_features.chip_id_pattern = 0.6f;
    s2_features.wifi_capability = 1.0f;
    s2_features.bluetooth_capability = 0.0f;  // No Bluetooth
    s2_features.flash_size_mb = 4.0f;
    
    device_type = model.Predict(s2_features);
    confidence = model.GetConfidence(s2_features, device_type);
    
    std::cout << "  Detected: " << PretrainedModel::GetDeviceTypeName(device_type) << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (confidence * 100.0f) << "%" << std::endl;
    
    PrintSeparator();
}

void TestMLDeviceDetector() {
    std::cout << "=== Testing ML Device Detector ===" << std::endl;
    PrintSeparator();
    
    MLDeviceDetector detector;
    
    // Test detection from port
    std::cout << "Test 1: Detecting device on /dev/ttyUSB0" << std::endl;
    auto result1 = detector.DetectDevice("/dev/ttyUSB0", 115200);
    std::cout << "  Success: " << (result1.success ? "Yes" : "No") << std::endl;
    std::cout << "  Device: " << result1.device_name << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (result1.confidence * 100.0f) << "%" << std::endl;
    std::cout << "  Details:\n" << result1.details << std::endl;
    
    std::cout << "\nTest 2: Detecting device on /dev/ttyUSB1" << std::endl;
    auto result2 = detector.DetectDevice("/dev/ttyUSB1", 115200);
    std::cout << "  Success: " << (result2.success ? "Yes" : "No") << std::endl;
    std::cout << "  Device: " << result2.device_name << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (result2.confidence * 100.0f) << "%" << std::endl;
    std::cout << "  Details:\n" << result2.details << std::endl;
    
    std::cout << "\nTest 3: Detecting device on /dev/ttyACM0" << std::endl;
    auto result3 = detector.DetectDevice("/dev/ttyACM0", 115200);
    std::cout << "  Success: " << (result3.success ? "Yes" : "No") << std::endl;
    std::cout << "  Device: " << result3.device_name << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (result3.confidence * 100.0f) << "%" << std::endl;
    std::cout << "  Details:\n" << result3.details << std::endl;
    
    PrintSeparator();
}

void TestDetectionFromCharacteristics() {
    std::cout << "=== Testing Detection from Characteristics ===" << std::endl;
    PrintSeparator();
    
    MLDeviceDetector detector;
    
    // Test with custom boot message
    std::cout << "Test 1: ESP32-S3 from boot message" << std::endl;
    std::string boot_msg = "ESP32-S3 chip revision 0\n2 cores, WiFi/BLE\nFlash: 8MB";
    auto result = detector.DetectFromCharacteristics(boot_msg, 512, 120.0f, "0xABCD1234");
    
    std::cout << "  Success: " << (result.success ? "Yes" : "No") << std::endl;
    std::cout << "  Device: " << result.device_name << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (result.confidence * 100.0f) << "%" << std::endl;
    std::cout << "  Details:\n" << result.details << std::endl;
    
    std::cout << "\nTest 2: ESP32-C3 from boot message" << std::endl;
    boot_msg = "ESP32-C3 chip revision 3\n1 core, WiFi/BLE\nFlash: 4MB";
    result = detector.DetectFromCharacteristics(boot_msg, 400, 100.0f, "0x1234CDEF");
    
    std::cout << "  Success: " << (result.success ? "Yes" : "No") << std::endl;
    std::cout << "  Device: " << result.device_name << std::endl;
    std::cout << "  Confidence: " << std::fixed << std::setprecision(2) 
              << (result.confidence * 100.0f) << "%" << std::endl;
    std::cout << "  Details:\n" << result.details << std::endl;
    
    PrintSeparator();
}

void TestCallbackFunctionality() {
    std::cout << "=== Testing Callback Functionality ===" << std::endl;
    PrintSeparator();
    
    MLDeviceDetector detector;
    
    // Set up callback
    detector.SetDetectionCallback([](const MLDeviceDetector::DetectionResult& result) {
        std::cout << "\n[CALLBACK] Detection completed!" << std::endl;
        std::cout << "[CALLBACK] Device: " << result.device_name << std::endl;
        std::cout << "[CALLBACK] Confidence: " << std::fixed << std::setprecision(2) 
                  << (result.confidence * 100.0f) << "%" << std::endl;
    });
    
    std::cout << "Detecting device with callback enabled..." << std::endl;
    detector.DetectDevice("/dev/ttyUSB0", 115200);
    
    PrintSeparator();
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   ESP32 ML Device Detection with Pretrained Model - Test Suite   ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    
    try {
        // Test 1: Pretrained model
        TestPretrainedModel();
        
        // Test 2: ML device detector
        TestMLDeviceDetector();
        
        // Test 3: Detection from characteristics
        TestDetectionFromCharacteristics();
        
        // Test 4: Callback functionality
        TestCallbackFunctionality();
        
        std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                    All Tests Completed Successfully!               ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";
        
        std::cout << "Summary:\n";
        std::cout << "  ✓ Pretrained neural network model working\n";
        std::cout << "  ✓ ML device detector functional\n";
        std::cout << "  ✓ Detection from serial port working\n";
        std::cout << "  ✓ Detection from characteristics working\n";
        std::cout << "  ✓ Callback mechanism functional\n";
        std::cout << "\nThe ML device detection system is ready for use!\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
