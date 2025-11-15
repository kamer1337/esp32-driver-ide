# VM Emulator Documentation

## Overview

The VM Emulator is a virtual machine that emulates ESP32 device configurations for testing without requiring physical hardware. It provides a complete testing environment for device configurations, GPIO operations, WiFi/Bluetooth simulation, memory management, and code execution.

## Features

### Device Configuration
- Support for multiple ESP32 board types:
  - ESP32 Dev Module
  - ESP32-S2
  - ESP32-S3
  - ESP32-C3
- Accurate flash, PSRAM, and SRAM specifications
- Configurable CPU frequency
- Chip revision information

### GPIO Operations
- Pin mode configuration (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN)
- Digital pin state control (HIGH/LOW)
- PWM (Pulse Width Modulation) simulation
- ADC (Analog-to-Digital Converter) simulation
- Pin change callbacks

### WiFi Simulation
- Multiple WiFi modes (STATION, ACCESS_POINT, STATION_AP)
- Connection simulation with status tracking
- IP and MAC address generation
- Signal strength simulation
- WiFi data transmission simulation

### Bluetooth Simulation
- Multiple Bluetooth modes (CLASSIC, BLE, DUAL)
- Device name and MAC address configuration
- Connection state management
- Bluetooth data transmission simulation

### Memory Management
- Heap memory allocation and deallocation
- PSRAM management (for supported boards)
- Memory fragmentation tracking
- Memory status monitoring

### Peripheral Management
- SPI, I2C, UART, DAC, ADC peripherals
- Custom peripheral support
- Enable/disable control

### Code Execution
- Virtual code execution simulation
- Execution time tracking
- Memory usage monitoring
- Error reporting

### Serial Output
- Serial output buffering
- Message logging
- Output callback support

### Configuration Validation
- Device configuration validation
- Error and warning reporting
- Configuration health checks

## Usage Examples

### Basic VM Emulator Setup

```cpp
#include "emulator/vm_emulator.h"

using namespace esp32_ide;

// Create a VM emulator for ESP32
VMEmulator vm(VMEmulator::BoardType::ESP32);

// Start the virtual machine
vm.Start();

// Get device configuration
auto config = vm.GetDeviceConfig();
std::cout << "Board: " << vm.GetBoardName() << std::endl;
std::cout << "Flash: " << config.flash_size_mb << " MB" << std::endl;
```

### GPIO Operations

```cpp
// Set pin mode
vm.SetPinMode(2, VMEmulator::PinMode::OUTPUT);

// Set pin state
vm.SetPinState(2, VMEmulator::PinState::HIGH);

// Read pin state
auto state = vm.GetPinState(2);
```

### PWM Operations

```cpp
// Enable PWM on pin
vm.EnablePWM(5, 1000);  // 1000 Hz frequency

// Set PWM duty cycle (0-255)
vm.SetPWMValue(5, 128);  // 50% duty cycle

// Read PWM value
int value = vm.GetPWMValue(5);
```

### ADC Operations

```cpp
// Enable ADC on pin
vm.EnableADC(34);

// Simulate sensor data
vm.SimulateSensorData(34, 2048);

// Read ADC value
int adc_value = vm.ReadADC(34);  // Returns 2048
```

### WiFi Configuration

```cpp
// Configure WiFi
vm.ConfigureWiFi(VMEmulator::WiFiMode::STATION, "MyNetwork", "password");

// Connect to WiFi
auto status = vm.ConnectWiFi();

// Get WiFi configuration
auto wifi_config = vm.GetWiFiConfig();
std::cout << "IP: " << wifi_config.ip_address << std::endl;

// Simulate WiFi data
vm.SimulateWiFiData("HTTP GET request");
```

### Bluetooth Configuration

```cpp
// Configure Bluetooth
vm.ConfigureBluetooth(VMEmulator::BluetoothMode::BLE, "ESP32_Device");

// Enable Bluetooth
vm.EnableBluetooth();

// Get Bluetooth configuration
auto bt_config = vm.GetBluetoothConfig();

// Simulate Bluetooth data
vm.SimulateBluetoothData("Sensor data");
```

### Memory Management

```cpp
// Get memory status
auto memory = vm.GetMemoryStatus();
std::cout << "Free heap: " << memory.free_heap << " bytes" << std::endl;

// Allocate memory
vm.AllocateMemory(1024);

// Free memory
vm.FreeMemory(512);

// Simulate memory usage
vm.SimulateMemoryUsage(50000, 0);
```

### Code Execution

```cpp
std::string code = R"(
void setup() {
    pinMode(2, OUTPUT);
}

void loop() {
    digitalWrite(2, HIGH);
    delay(1000);
}
)";

// Execute code in virtual environment
auto result = vm.ExecuteCode(code);

if (result.success) {
    std::cout << "Execution successful" << std::endl;
    std::cout << "Time: " << result.execution_time_ms << " ms" << std::endl;
    std::cout << "Memory: " << result.memory_used << " bytes" << std::endl;
} else {
    for (const auto& error : result.errors) {
        std::cout << "Error: " << error << std::endl;
    }
}
```

### Serial Output

```cpp
// Write to serial
vm.WriteSerial("Hello from VM!");

// Read serial output
auto output = vm.ReadSerialOutput();
for (const auto& msg : output) {
    std::cout << msg << std::endl;
}

// Clear serial output
vm.ClearSerialOutput();
```

### Configuration Validation

```cpp
// Validate configuration
auto validation = vm.ValidateConfiguration();

if (!validation.valid) {
    std::cout << "Configuration errors:" << std::endl;
    for (const auto& error : validation.errors) {
        std::cout << "  - " << error << std::endl;
    }
}

if (!validation.warnings.empty()) {
    std::cout << "Configuration warnings:" << std::endl;
    for (const auto& warning : validation.warnings) {
        std::cout << "  - " << warning << std::endl;
    }
}
```

### Using Callbacks

```cpp
// Set output callback
vm.SetOutputCallback([](const std::string& msg) {
    std::cout << "[VM] " << msg << std::endl;
});

// Set pin change callback
vm.SetPinChangeCallback([](int pin, VMEmulator::PinState state) {
    std::string state_str = (state == VMEmulator::PinState::HIGH) ? "HIGH" : "LOW";
    std::cout << "Pin " << pin << " changed to " << state_str << std::endl;
});
```

## Integration with IDE

The VM Emulator is integrated with the main window and can be accessed through:

1. **OnStartEmulator()** - Starts the virtual machine
2. **OnStopEmulator()** - Stops the virtual machine
3. **OnTestInEmulator()** - Tests current code in the virtual environment

### Testing Code in VM

```cpp
// In MainWindow
void OnTestInEmulator() {
    // Starts VM if not running
    // Compiles the code
    // Executes in virtual environment
    // Shows execution results
    // Displays serial output
    // Shows memory status
    // Validates configuration
}
```

## Supported Board Specifications

### ESP32 Dev Module
- Flash: 4 MB
- PSRAM: 0 MB
- SRAM: 520 KB
- CPU: 240 MHz
- GPIO Pins: 26

### ESP32-S2
- Flash: 4 MB
- PSRAM: 2 MB
- SRAM: 320 KB
- CPU: 240 MHz
- GPIO Pins: 35

### ESP32-S3
- Flash: 8 MB
- PSRAM: 8 MB
- SRAM: 512 KB
- CPU: 240 MHz
- GPIO Pins: 36

### ESP32-C3
- Flash: 4 MB
- PSRAM: 0 MB
- SRAM: 400 KB
- CPU: 160 MHz
- GPIO Pins: 15

## Testing Utilities

### Sensor Data Simulation
```cpp
vm.SimulateSensorData(pin, value);
```

### Interrupt Simulation
```cpp
vm.SimulateInterrupt(pin);
```

### Delay Simulation
```cpp
vm.SimulateDelay(milliseconds);
```

### Execution Log
```cpp
auto log = vm.GetExecutionLog();
for (const auto& entry : log) {
    std::cout << entry << std::endl;
}

vm.ClearExecutionLog();
```

## Best Practices

1. **Start the VM before testing**: Always call `Start()` before running tests
2. **Validate configuration**: Use `ValidateConfiguration()` to check for issues
3. **Use callbacks**: Set up callbacks for real-time monitoring
4. **Check execution results**: Always check the `success` field of execution results
5. **Monitor memory**: Keep track of memory usage to avoid fragmentation
6. **Clear buffers**: Regularly clear serial output and execution logs

## Limitations

- Code execution is simulated, not actually compiled and run
- Timing is approximate and not cycle-accurate
- Some hardware-specific behaviors may not be fully emulated
- Network communication is simulated, not real

## Future Enhancements

- More accurate timing simulation
- Enhanced peripheral simulation
- Hardware interrupt emulation
- Real-time debugging support
- Advanced memory profiling
- Network packet simulation

## Demo Application

A complete demo application is available at `src/vm_emulator_demo.cpp`. Run it with:

```bash
./build/esp32-vm-emulator-demo
```

This demonstrates all VM emulator features with example code.
