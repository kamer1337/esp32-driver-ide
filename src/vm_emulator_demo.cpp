#include <iostream>
#include <iomanip>
#include "emulator/vm_emulator.h"

using namespace esp32_ide;

void printSeparator() {
    std::cout << "========================================" << std::endl;
}

void printHeader(const std::string& title) {
    printSeparator();
    std::cout << title << std::endl;
    printSeparator();
}

void demonstrateDeviceConfiguration(VMEmulator& vm) {
    printHeader("Device Configuration Demo");
    
    std::cout << "Board: " << vm.GetBoardName() << std::endl;
    auto config = vm.GetDeviceConfig();
    std::cout << "Flash Size: " << config.flash_size_mb << " MB" << std::endl;
    std::cout << "PSRAM Size: " << config.psram_size_mb << " MB" << std::endl;
    std::cout << "SRAM Size: " << config.sram_size_kb << " KB" << std::endl;
    std::cout << "CPU Frequency: " << config.cpu_frequency_mhz << " MHz" << std::endl;
    std::cout << "Chip Revision: " << config.chip_revision << std::endl;
    std::cout << "Available GPIO Pins: " << config.available_gpio_pins.size() << std::endl;
    std::cout << std::endl;
}

void demonstrateGPIO(VMEmulator& vm) {
    printHeader("GPIO Operations Demo");
    
    int led_pin = 2;
    int sensor_pin = 34;
    
    std::cout << "Setting up GPIO pins..." << std::endl;
    vm.SetPinMode(led_pin, VMEmulator::PinMode::OUTPUT);
    vm.SetPinMode(sensor_pin, VMEmulator::PinMode::INPUT);
    
    std::cout << "  LED Pin " << led_pin << " set to OUTPUT" << std::endl;
    std::cout << "  Sensor Pin " << sensor_pin << " set to INPUT" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Testing LED output..." << std::endl;
    vm.SetPinState(led_pin, VMEmulator::PinState::HIGH);
    std::cout << "  LED Pin " << led_pin << " -> HIGH" << std::endl;
    
    vm.SetPinState(led_pin, VMEmulator::PinState::LOW);
    std::cout << "  LED Pin " << led_pin << " -> LOW" << std::endl;
    std::cout << std::endl;
}

void demonstratePWM(VMEmulator& vm) {
    printHeader("PWM Operations Demo");
    
    int pwm_pin = 5;
    
    std::cout << "Enabling PWM on pin " << pwm_pin << "..." << std::endl;
    vm.EnablePWM(pwm_pin, 1000);
    
    std::cout << "Setting PWM duty cycle values:" << std::endl;
    vm.SetPWMValue(pwm_pin, 64);
    std::cout << "  25% duty cycle: " << vm.GetPWMValue(pwm_pin) << std::endl;
    
    vm.SetPWMValue(pwm_pin, 128);
    std::cout << "  50% duty cycle: " << vm.GetPWMValue(pwm_pin) << std::endl;
    
    vm.SetPWMValue(pwm_pin, 192);
    std::cout << "  75% duty cycle: " << vm.GetPWMValue(pwm_pin) << std::endl;
    
    vm.SetPWMValue(pwm_pin, 255);
    std::cout << "  100% duty cycle: " << vm.GetPWMValue(pwm_pin) << std::endl;
    std::cout << std::endl;
}

void demonstrateADC(VMEmulator& vm) {
    printHeader("ADC Operations Demo");
    
    int adc_pin = 34;
    
    std::cout << "Enabling ADC on pin " << adc_pin << "..." << std::endl;
    vm.EnableADC(adc_pin);
    
    std::cout << "Simulating sensor readings:" << std::endl;
    
    vm.SimulateSensorData(adc_pin, 512);
    std::cout << "  Sensor value: " << vm.ReadADC(adc_pin) << " (12.5%)" << std::endl;
    
    vm.SimulateSensorData(adc_pin, 2048);
    std::cout << "  Sensor value: " << vm.ReadADC(adc_pin) << " (50%)" << std::endl;
    
    vm.SimulateSensorData(adc_pin, 4095);
    std::cout << "  Sensor value: " << vm.ReadADC(adc_pin) << " (100%)" << std::endl;
    std::cout << std::endl;
}

void demonstrateWiFi(VMEmulator& vm) {
    printHeader("WiFi Simulation Demo");
    
    std::cout << "Configuring WiFi..." << std::endl;
    vm.ConfigureWiFi(VMEmulator::WiFiMode::STATION, "TestNetwork", "password123");
    
    std::cout << "Connecting to WiFi..." << std::endl;
    auto status = vm.ConnectWiFi();
    
    auto wifi_config = vm.GetWiFiConfig();
    std::cout << "  Status: " << (status == VMEmulator::WiFiStatus::CONNECTED ? "CONNECTED" : "FAILED") << std::endl;
    std::cout << "  SSID: " << wifi_config.ssid << std::endl;
    std::cout << "  IP Address: " << wifi_config.ip_address << std::endl;
    std::cout << "  MAC Address: " << wifi_config.mac_address << std::endl;
    std::cout << "  Signal Strength: " << wifi_config.signal_strength << " dBm" << std::endl;
    
    std::cout << "\nSimulating WiFi data..." << std::endl;
    vm.SimulateWiFiData("HTTP GET request to server");
    std::cout << std::endl;
}

void demonstrateBluetooth(VMEmulator& vm) {
    printHeader("Bluetooth Simulation Demo");
    
    std::cout << "Configuring Bluetooth..." << std::endl;
    vm.ConfigureBluetooth(VMEmulator::BluetoothMode::BLE, "ESP32_Device");
    
    std::cout << "Enabling Bluetooth..." << std::endl;
    vm.EnableBluetooth();
    
    auto bt_config = vm.GetBluetoothConfig();
    std::cout << "  Device Name: " << bt_config.device_name << std::endl;
    std::cout << "  MAC Address: " << bt_config.mac_address << std::endl;
    std::cout << "  Mode: BLE" << std::endl;
    std::cout << "  Connected: " << (bt_config.connected ? "Yes" : "No") << std::endl;
    
    std::cout << "\nSimulating Bluetooth data..." << std::endl;
    vm.SimulateBluetoothData("Sensor data packet");
    std::cout << std::endl;
}

void demonstrateMemory(VMEmulator& vm) {
    printHeader("Memory Management Demo");
    
    auto memory = vm.GetMemoryStatus();
    std::cout << "Initial Memory Status:" << std::endl;
    std::cout << "  Total Heap: " << memory.total_heap << " bytes" << std::endl;
    std::cout << "  Free Heap: " << memory.free_heap << " bytes" << std::endl;
    std::cout << "  Total PSRAM: " << memory.total_psram << " bytes" << std::endl;
    std::cout << "  Fragmentation: " << std::fixed << std::setprecision(2) 
              << memory.fragmentation_percent << "%" << std::endl;
    
    std::cout << "\nAllocating 50KB..." << std::endl;
    vm.AllocateMemory(50 * 1024);
    
    memory = vm.GetMemoryStatus();
    std::cout << "  Free Heap: " << memory.free_heap << " bytes" << std::endl;
    std::cout << "  Fragmentation: " << std::fixed << std::setprecision(2) 
              << memory.fragmentation_percent << "%" << std::endl;
    
    std::cout << "\nFreeing 25KB..." << std::endl;
    vm.FreeMemory(25 * 1024);
    
    memory = vm.GetMemoryStatus();
    std::cout << "  Free Heap: " << memory.free_heap << " bytes" << std::endl;
    std::cout << "  Fragmentation: " << std::fixed << std::setprecision(2) 
              << memory.fragmentation_percent << "%" << std::endl;
    std::cout << std::endl;
}

void demonstratePeripherals(VMEmulator& vm) {
    printHeader("Peripheral Management Demo");
    
    std::cout << "Enabling peripherals..." << std::endl;
    vm.EnablePeripheral("SPI");
    vm.EnablePeripheral("I2C");
    vm.EnablePeripheral("UART");
    
    auto peripheral_status = vm.GetPeripheralStatus();
    std::cout << "  SPI: " << (peripheral_status.spi_enabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  I2C: " << (peripheral_status.i2c_enabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  UART: " << (peripheral_status.uart_enabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  DAC: " << (peripheral_status.dac_enabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  ADC: " << (peripheral_status.adc_enabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << std::endl;
}

void demonstrateCodeExecution(VMEmulator& vm) {
    printHeader("Code Execution Demo");
    
    std::string valid_code = R"(
void setup() {
    pinMode(2, OUTPUT);
}

void loop() {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
}
)";
    
    std::cout << "Executing valid ESP32 code..." << std::endl;
    auto result = vm.ExecuteCode(valid_code);
    
    std::cout << "  Success: " << (result.success ? "Yes" : "No") << std::endl;
    std::cout << "  Output: " << result.output << std::endl;
    std::cout << "  Execution Time: " << result.execution_time_ms << " ms" << std::endl;
    std::cout << "  Memory Used: " << result.memory_used << " bytes" << std::endl;
    
    if (!result.errors.empty()) {
        std::cout << "  Errors:" << std::endl;
        for (const auto& error : result.errors) {
            std::cout << "    - " << error << std::endl;
        }
    }
    std::cout << std::endl;
}

void demonstrateSerialOutput(VMEmulator& vm) {
    printHeader("Serial Output Demo");
    
    std::cout << "Writing to serial output..." << std::endl;
    vm.WriteSerial("Hello from virtual ESP32!");
    vm.WriteSerial("Temperature: 25.5°C");
    vm.WriteSerial("Humidity: 60%");
    
    auto serial_output = vm.ReadSerialOutput();
    std::cout << "\nSerial Output Buffer (" << serial_output.size() << " messages):" << std::endl;
    for (const auto& msg : serial_output) {
        std::cout << "  > " << msg << std::endl;
    }
    std::cout << std::endl;
}

void demonstrateValidation(VMEmulator& vm) {
    printHeader("Configuration Validation Demo");
    
    auto validation = vm.ValidateConfiguration();
    
    std::cout << "Configuration Status: " << (validation.valid ? "VALID" : "INVALID") << std::endl;
    
    if (!validation.errors.empty()) {
        std::cout << "\nErrors:" << std::endl;
        for (const auto& error : validation.errors) {
            std::cout << "  ✗ " << error << std::endl;
        }
    }
    
    if (!validation.warnings.empty()) {
        std::cout << "\nWarnings:" << std::endl;
        for (const auto& warning : validation.warnings) {
            std::cout << "  ⚠ " << warning << std::endl;
        }
    }
    
    if (validation.valid && validation.errors.empty() && validation.warnings.empty()) {
        std::cout << "  ✓ No issues found" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    printHeader("ESP32 Virtual Machine Emulator Demo");
    std::cout << "Demonstrating device configuration testing capabilities" << std::endl;
    std::cout << std::endl;
    
    // Create a virtual ESP32 device
    VMEmulator vm(VMEmulator::BoardType::ESP32);
    
    // Set up output callback
    vm.SetOutputCallback([](const std::string& msg) {
        // Output callback can be used to log messages
    });
    
    // Start the virtual machine
    std::cout << "Starting virtual ESP32 device..." << std::endl;
    vm.Start();
    std::cout << "✓ Virtual device started successfully" << std::endl;
    std::cout << std::endl;
    
    // Run demonstrations
    demonstrateDeviceConfiguration(vm);
    demonstrateGPIO(vm);
    demonstratePWM(vm);
    demonstrateADC(vm);
    demonstrateWiFi(vm);
    demonstrateBluetooth(vm);
    demonstrateMemory(vm);
    demonstratePeripherals(vm);
    demonstrateCodeExecution(vm);
    demonstrateSerialOutput(vm);
    demonstrateValidation(vm);
    
    // Stop the virtual machine
    std::cout << "Stopping virtual ESP32 device..." << std::endl;
    vm.Stop();
    std::cout << "✓ Virtual device stopped successfully" << std::endl;
    std::cout << std::endl;
    
    printSeparator();
    std::cout << "Demo completed successfully!" << std::endl;
    printSeparator();
    
    return 0;
}
