# ESP32 Device Support

## Overview

The ESP32 Driver IDE supports a comprehensive range of ESP32 device variants through both manual board selection and automatic ML-based device detection.

## Supported Devices

### ESP32 Series (Original)

#### ESP32
- **Description**: The original ESP32 module
- **CPU**: Dual-core Xtensa LX6 @ 240 MHz
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: BLE 4.2 + Classic Bluetooth
- **SRAM**: 520 KB
- **Flash**: Typically 4 MB
- **Use Cases**: General-purpose IoT, WiFi/BT projects

### ESP32-S Series (Low Power)

#### ESP32-S2
- **Description**: Low-power variant with USB OTG
- **CPU**: Single-core Xtensa LX7 @ 240 MHz
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: None
- **SRAM**: 320 KB
- **Flash**: Typically 4 MB
- **Use Cases**: USB devices, low-power WiFi applications

#### ESP32-S3
- **Description**: Enhanced AI capabilities with USB OTG
- **CPU**: Dual-core Xtensa LX7 @ 240 MHz
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: BLE 5.0
- **SRAM**: 512 KB
- **Flash**: Typically 8 MB
- **Use Cases**: AI/ML applications, image processing, voice recognition

### ESP32-C Series (RISC-V)

#### ESP32-C2
- **Description**: Cost-optimized RISC-V based module
- **CPU**: Single-core RISC-V @ 120 MHz
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: BLE 5.0
- **SRAM**: 272 KB
- **Flash**: Typically 2-4 MB
- **Use Cases**: Budget-conscious IoT projects, simple WiFi/BLE applications

#### ESP32-C3
- **Description**: RISC-V based with WiFi and BLE
- **CPU**: Single-core RISC-V @ 160 MHz
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: BLE 5.0
- **SRAM**: 400 KB
- **Flash**: Typically 4 MB
- **Use Cases**: IoT devices, WiFi/BLE projects

#### ESP32-C6
- **Description**: Modern RISC-V with WiFi 6 and Zigbee/Thread
- **CPU**: Single-core RISC-V @ 160 MHz
- **WiFi**: 802.11 ax (WiFi 6, 2.4 GHz)
- **Bluetooth**: BLE 5.0
- **Zigbee/Thread**: IEEE 802.15.4 support
- **SRAM**: 512 KB
- **Flash**: Typically 4-8 MB
- **Use Cases**: Matter-compatible smart home devices, WiFi 6 applications

### ESP32-H Series (Matter/Thread)

#### ESP32-H2
- **Description**: Dedicated Matter/Thread/Zigbee chip
- **CPU**: Single-core RISC-V @ 96 MHz
- **WiFi**: None
- **Bluetooth**: BLE 5.2
- **Zigbee/Thread**: IEEE 802.15.4 support
- **SRAM**: 320 KB
- **Flash**: Typically 2-4 MB
- **Use Cases**: Matter/Thread border routers, Zigbee gateways, BLE mesh

### ESP32-P Series (High Performance)

#### ESP32-P4
- **Description**: High-performance without wireless connectivity
- **CPU**: Dual-core RISC-V @ 400 MHz
- **WiFi**: None (external connectivity via Ethernet/USB)
- **Bluetooth**: None
- **SRAM**: 768 KB (expandable)
- **PSRAM**: Support for up to 32 MB
- **Flash**: External support
- **Use Cases**: High-performance computing, display controllers, industrial automation

## Feature Comparison Matrix

| Device    | CPU Cores | CPU Arch | Max Clock | WiFi   | BLE    | Zigbee/Thread | SRAM   | Typical Flash |
|-----------|-----------|----------|-----------|--------|--------|---------------|--------|---------------|
| ESP32     | 2         | Xtensa   | 240 MHz   | 2.4GHz | 4.2    | No            | 520 KB | 4 MB          |
| ESP32-S2  | 1         | Xtensa   | 240 MHz   | 2.4GHz | No     | No            | 320 KB | 4 MB          |
| ESP32-S3  | 2         | Xtensa   | 240 MHz   | 2.4GHz | 5.0    | No            | 512 KB | 8 MB          |
| ESP32-C2  | 1         | RISC-V   | 120 MHz   | 2.4GHz | 5.0    | No            | 272 KB | 2-4 MB        |
| ESP32-C3  | 1         | RISC-V   | 160 MHz   | 2.4GHz | 5.0    | No            | 400 KB | 4 MB          |
| ESP32-C6  | 1         | RISC-V   | 160 MHz   | WiFi 6 | 5.0    | Yes           | 512 KB | 4-8 MB        |
| ESP32-H2  | 1         | RISC-V   | 96 MHz    | No     | 5.2    | Yes           | 320 KB | 2-4 MB        |
| ESP32-P4  | 2         | RISC-V   | 400 MHz   | No     | No     | No            | 768 KB | External      |

## Selecting a Device

### Manual Selection

In the ESP32 Driver IDE, you can manually select your board type through the board selection menu. The compiler will automatically configure build settings for your chosen device.

### Automatic Detection (ML-Based)

The IDE includes a machine learning-based device detection system that can automatically identify your ESP32 variant by analyzing:
- Serial communication patterns
- Boot messages
- Device characteristics (memory size, response time, etc.)
- Available features (WiFi, Bluetooth, etc.)

See [ML_DEVICE_DETECTION.md](ML_DEVICE_DETECTION.md) for detailed information on the ML detection system.

## Device Selection Guide

### For WiFi + Bluetooth Projects
- **Budget**: ESP32-C2
- **Standard**: ESP32, ESP32-C3
- **Advanced/AI**: ESP32-S3
- **Modern/WiFi 6**: ESP32-C6

### For WiFi-Only Projects
- **USB Support**: ESP32-S2
- **General**: ESP32 or ESP32-C3 (disable Bluetooth)

### For Bluetooth-Only Projects
- **Standard BLE**: ESP32-C3, ESP32-S3
- **Matter/Thread/Zigbee**: ESP32-H2

### For No-Wireless Projects
- **High Performance**: ESP32-P4
- **Display Controllers**: ESP32-P4
- **Industrial**: ESP32-P4

### For Smart Home/Matter
- **WiFi Hub**: ESP32-C6
- **Thread Border Router**: ESP32-H2, ESP32-C6
- **Zigbee Gateway**: ESP32-H2

## Compiler Support

All device types are fully supported in the ESP32 compiler with appropriate board configurations:

```cpp
#include "compiler/esp32_compiler.h"

ESP32Compiler compiler;
compiler.SetBoard(ESP32Compiler::BoardType::ESP32_C6);
auto result = compiler.Compile(code, board);
```

## Integration with ML Detection

The ML device detector can identify devices and automatically configure the compiler:

```cpp
#include "utils/ml_device_detector.h"
#include "compiler/esp32_compiler.h"

MLDeviceDetector detector;
auto result = detector.DetectDevice("/dev/ttyUSB0", 115200);

ESP32Compiler compiler;
if (result.device_type == PretrainedModel::DeviceType::ESP32_C6) {
    compiler.SetBoard(ESP32Compiler::BoardType::ESP32_C6);
}
```

## Resources

- [ML Device Detection Documentation](ML_DEVICE_DETECTION.md)
- [ESP32 Official Documentation](https://docs.espressif.com/)
- [Board Selection in README](README.md#esp32-development-tools)

## Support and Contributions

If you encounter issues with any device type or would like to contribute device-specific features, please open an issue or pull request on the GitHub repository.
