#include <iostream>
#include <cassert>
#include "emulator/vm_emulator.h"

using namespace esp32_ide;

void test_vm_initialization() {
    std::cout << "Testing VM Initialization..." << std::endl;
    
    VMEmulator vm(VMEmulator::BoardType::ESP32);
    assert(vm.GetBoardType() == VMEmulator::BoardType::ESP32);
    assert(!vm.IsRunning());
    
    auto config = vm.GetDeviceConfig();
    assert(config.flash_size_mb == 4);
    assert(config.sram_size_kb == 520);
    assert(!config.available_gpio_pins.empty());
    
    std::cout << "  ✓ VM initialization test passed" << std::endl;
}

void test_vm_lifecycle() {
    std::cout << "Testing VM Lifecycle..." << std::endl;
    
    VMEmulator vm;
    assert(!vm.IsRunning());
    
    bool started = vm.Start();
    assert(started);
    assert(vm.IsRunning());
    
    bool stopped = vm.Stop();
    assert(stopped);
    assert(!vm.IsRunning());
    
    // Test reset
    vm.Start();
    bool reset = vm.Reset();
    assert(reset);
    
    std::cout << "  ✓ VM lifecycle test passed" << std::endl;
}

void test_gpio_operations() {
    std::cout << "Testing GPIO Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    // Test pin mode
    assert(vm.SetPinMode(2, VMEmulator::PinMode::OUTPUT));
    assert(vm.GetPinMode(2) == VMEmulator::PinMode::OUTPUT);
    
    // Test pin state
    assert(vm.SetPinState(2, VMEmulator::PinState::HIGH));
    assert(vm.GetPinState(2) == VMEmulator::PinState::HIGH);
    
    assert(vm.SetPinState(2, VMEmulator::PinState::LOW));
    assert(vm.GetPinState(2) == VMEmulator::PinState::LOW);
    
    // Test invalid pin
    assert(!vm.IsPinValid(999));
    
    std::cout << "  ✓ GPIO operations test passed" << std::endl;
}

void test_pwm_operations() {
    std::cout << "Testing PWM Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    int pin = 2;
    assert(vm.EnablePWM(pin, 1000));
    assert(vm.SetPWMValue(pin, 128));
    assert(vm.GetPWMValue(pin) == 128);
    
    // Test boundary values
    assert(vm.SetPWMValue(pin, 300)); // Should be clamped to 255
    assert(vm.GetPWMValue(pin) == 255);
    
    assert(vm.SetPWMValue(pin, -50)); // Should be clamped to 0
    assert(vm.GetPWMValue(pin) == 0);
    
    std::cout << "  ✓ PWM operations test passed" << std::endl;
}

void test_adc_operations() {
    std::cout << "Testing ADC Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    int pin = 34;
    assert(vm.EnableADC(pin));
    
    // Simulate sensor data
    vm.SimulateSensorData(pin, 2048);
    assert(vm.ReadADC(pin) == 2048);
    
    std::cout << "  ✓ ADC operations test passed" << std::endl;
}

void test_wifi_operations() {
    std::cout << "Testing WiFi Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    assert(vm.ConfigureWiFi(VMEmulator::WiFiMode::STATION, "TestSSID", "password123"));
    
    auto wifi_config = vm.GetWiFiConfig();
    assert(wifi_config.mode == VMEmulator::WiFiMode::STATION);
    assert(wifi_config.ssid == "TestSSID");
    
    auto status = vm.ConnectWiFi();
    assert(status == VMEmulator::WiFiStatus::CONNECTED);
    
    wifi_config = vm.GetWiFiConfig();
    assert(!wifi_config.ip_address.empty());
    assert(!wifi_config.mac_address.empty());
    
    assert(vm.DisconnectWiFi());
    
    std::cout << "  ✓ WiFi operations test passed" << std::endl;
}

void test_bluetooth_operations() {
    std::cout << "Testing Bluetooth Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    assert(vm.ConfigureBluetooth(VMEmulator::BluetoothMode::BLE, "ESP32_BLE"));
    
    auto bt_config = vm.GetBluetoothConfig();
    assert(bt_config.mode == VMEmulator::BluetoothMode::BLE);
    assert(bt_config.device_name == "ESP32_BLE");
    
    assert(vm.EnableBluetooth());
    assert(vm.DisableBluetooth());
    
    std::cout << "  ✓ Bluetooth operations test passed" << std::endl;
}

void test_memory_operations() {
    std::cout << "Testing Memory Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    auto initial_memory = vm.GetMemoryStatus();
    assert(initial_memory.total_heap > 0);
    
    size_t bytes_to_allocate = 1024;
    assert(vm.AllocateMemory(bytes_to_allocate));
    
    auto memory_after_alloc = vm.GetMemoryStatus();
    assert(memory_after_alloc.free_heap < initial_memory.free_heap);
    
    assert(vm.FreeMemory(bytes_to_allocate));
    
    auto memory_after_free = vm.GetMemoryStatus();
    assert(memory_after_free.free_heap > memory_after_alloc.free_heap);
    
    std::cout << "  ✓ Memory operations test passed" << std::endl;
}

void test_peripheral_operations() {
    std::cout << "Testing Peripheral Operations..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    assert(vm.EnablePeripheral("SPI"));
    assert(vm.EnablePeripheral("I2C"));
    
    auto peripheral_status = vm.GetPeripheralStatus();
    assert(peripheral_status.spi_enabled);
    assert(peripheral_status.i2c_enabled);
    
    assert(vm.DisablePeripheral("SPI"));
    peripheral_status = vm.GetPeripheralStatus();
    assert(!peripheral_status.spi_enabled);
    
    std::cout << "  ✓ Peripheral operations test passed" << std::endl;
}

void test_serial_output() {
    std::cout << "Testing Serial Output..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    vm.WriteSerial("Test message 1");
    vm.WriteSerial("Test message 2");
    
    auto output = vm.ReadSerialOutput();
    assert(output.size() == 2);
    assert(output[0] == "Test message 1");
    assert(output[1] == "Test message 2");
    
    vm.ClearSerialOutput();
    output = vm.ReadSerialOutput();
    assert(output.empty());
    
    std::cout << "  ✓ Serial output test passed" << std::endl;
}

void test_code_execution() {
    std::cout << "Testing Code Execution..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    std::string valid_code = "void setup() {} void loop() {}";
    auto result = vm.ExecuteCode(valid_code);
    assert(result.success);
    assert(result.errors.empty());
    
    std::string invalid_code = "void main() {}";
    result = vm.ExecuteCode(invalid_code);
    assert(!result.success);
    assert(!result.errors.empty());
    
    std::cout << "  ✓ Code execution test passed" << std::endl;
}

void test_configuration_validation() {
    std::cout << "Testing Configuration Validation..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    auto validation = vm.ValidateConfiguration();
    assert(validation.valid);
    
    // Configure WiFi without SSID
    vm.ConfigureWiFi(VMEmulator::WiFiMode::STATION, "", "");
    validation = vm.ValidateConfiguration();
    assert(!validation.warnings.empty());
    
    std::cout << "  ✓ Configuration validation test passed" << std::endl;
}

void test_board_types() {
    std::cout << "Testing Different Board Types..." << std::endl;
    
    // ESP32
    VMEmulator vm_esp32(VMEmulator::BoardType::ESP32);
    auto config_esp32 = vm_esp32.GetDeviceConfig();
    assert(config_esp32.cpu_frequency_mhz == 240);
    
    // ESP32-S2
    VMEmulator vm_s2(VMEmulator::BoardType::ESP32_S2);
    auto config_s2 = vm_s2.GetDeviceConfig();
    assert(config_s2.psram_size_mb == 2);
    
    // ESP32-S3
    VMEmulator vm_s3(VMEmulator::BoardType::ESP32_S3);
    auto config_s3 = vm_s3.GetDeviceConfig();
    assert(config_s3.flash_size_mb == 8);
    assert(config_s3.psram_size_mb == 8);
    
    // ESP32-C3
    VMEmulator vm_c3(VMEmulator::BoardType::ESP32_C3);
    auto config_c3 = vm_c3.GetDeviceConfig();
    assert(config_c3.cpu_frequency_mhz == 160);
    
    std::cout << "  ✓ Board types test passed" << std::endl;
}

void test_execution_log() {
    std::cout << "Testing Execution Log..." << std::endl;
    
    VMEmulator vm;
    vm.Start();
    
    auto log = vm.GetExecutionLog();
    assert(!log.empty());
    
    vm.SetPinMode(2, VMEmulator::PinMode::OUTPUT);
    vm.SetPinState(2, VMEmulator::PinState::HIGH);
    
    log = vm.GetExecutionLog();
    assert(log.size() > 2);
    
    vm.ClearExecutionLog();
    log = vm.GetExecutionLog();
    assert(log.empty());
    
    std::cout << "  ✓ Execution log test passed" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ESP32 VM Emulator Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        test_vm_initialization();
        test_vm_lifecycle();
        test_gpio_operations();
        test_pwm_operations();
        test_adc_operations();
        test_wifi_operations();
        test_bluetooth_operations();
        test_memory_operations();
        test_peripheral_operations();
        test_serial_output();
        test_code_execution();
        test_configuration_validation();
        test_board_types();
        test_execution_log();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ ALL VM EMULATOR TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
