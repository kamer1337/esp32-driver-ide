#include "emulator/vm_emulator.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>

namespace esp32_ide {

VMEmulator::VMEmulator() : VMEmulator(BoardType::ESP32) {}

VMEmulator::VMEmulator(BoardType board) 
    : board_type_(board), 
      running_(false),
      allocated_heap_(0),
      allocated_psram_(0) {
    InitializeDeviceConfig();
    InitializeGPIOPins();
    InitializeMemory();
}

VMEmulator::~VMEmulator() {
    if (running_) {
        Stop();
    }
}

void VMEmulator::InitializeDeviceConfig() {
    device_config_.board_type = board_type_;
    
    switch (board_type_) {
        case BoardType::ESP32:
            device_config_.flash_size_mb = 4;
            device_config_.psram_size_mb = 0;
            device_config_.sram_size_kb = 520;
            device_config_.cpu_frequency_mhz = 240;
            device_config_.chip_revision = "1.0";
            device_config_.available_gpio_pins = {
                0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 
                25, 26, 27, 32, 33, 34, 35, 36, 39
            };
            break;
            
        case BoardType::ESP32_S2:
            device_config_.flash_size_mb = 4;
            device_config_.psram_size_mb = 2;
            device_config_.sram_size_kb = 320;
            device_config_.cpu_frequency_mhz = 240;
            device_config_.chip_revision = "1.0";
            device_config_.available_gpio_pins = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
                18, 19, 20, 21, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45
            };
            break;
            
        case BoardType::ESP32_S3:
            device_config_.flash_size_mb = 8;
            device_config_.psram_size_mb = 8;
            device_config_.sram_size_kb = 512;
            device_config_.cpu_frequency_mhz = 240;
            device_config_.chip_revision = "1.0";
            device_config_.available_gpio_pins = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
                18, 19, 20, 21, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48
            };
            break;
            
        case BoardType::ESP32_C3:
            device_config_.flash_size_mb = 4;
            device_config_.psram_size_mb = 0;
            device_config_.sram_size_kb = 400;
            device_config_.cpu_frequency_mhz = 160;
            device_config_.chip_revision = "1.0";
            device_config_.available_gpio_pins = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 18, 19, 20, 21
            };
            break;
    }
}

void VMEmulator::InitializeGPIOPins() {
    gpio_pins_.clear();
    
    for (int pin : device_config_.available_gpio_pins) {
        GPIOPin gpio_pin;
        gpio_pin.pin_number = pin;
        gpio_pin.mode = PinMode::INPUT;
        gpio_pin.state = PinState::LOW;
        gpio_pin.pwm_enabled = false;
        gpio_pin.pwm_value = 0;
        gpio_pin.adc_enabled = false;
        gpio_pin.adc_value = 0;
        
        gpio_pins_[pin] = gpio_pin;
    }
}

void VMEmulator::InitializeMemory() {
    memory_status_.total_heap = device_config_.sram_size_kb * 1024;
    memory_status_.free_heap = memory_status_.total_heap;
    memory_status_.total_psram = device_config_.psram_size_mb * 1024 * 1024;
    memory_status_.free_psram = memory_status_.total_psram;
    memory_status_.largest_free_block = memory_status_.free_heap;
    memory_status_.fragmentation_percent = 0.0f;
    
    allocated_heap_ = 0;
    allocated_psram_ = 0;
}

void VMEmulator::SetBoardType(BoardType board) {
    if (running_) {
        LogExecution("Cannot change board type while VM is running");
        return;
    }
    
    board_type_ = board;
    InitializeDeviceConfig();
    InitializeGPIOPins();
    InitializeMemory();
    
    // Reset WiFi and Bluetooth
    wifi_config_ = WiFiConfig();
    bluetooth_config_ = BluetoothConfig();
    
    LogExecution("Board type changed to " + GetBoardName());
}

VMEmulator::BoardType VMEmulator::GetBoardType() const {
    return board_type_;
}

VMEmulator::DeviceConfig VMEmulator::GetDeviceConfig() const {
    return device_config_;
}

std::string VMEmulator::GetBoardName() const {
    switch (board_type_) {
        case BoardType::ESP32:    return "ESP32 Dev Module";
        case BoardType::ESP32_S2: return "ESP32-S2";
        case BoardType::ESP32_S3: return "ESP32-S3";
        case BoardType::ESP32_C3: return "ESP32-C3";
        default:                  return "Unknown Board";
    }
}

bool VMEmulator::Start() {
    if (running_) {
        LogExecution("VM is already running");
        return false;
    }
    
    running_ = true;
    LogExecution("VM started for " + GetBoardName());
    LogExecution("Flash: " + std::to_string(device_config_.flash_size_mb) + "MB, " +
                 "PSRAM: " + std::to_string(device_config_.psram_size_mb) + "MB, " +
                 "SRAM: " + std::to_string(device_config_.sram_size_kb) + "KB");
    
    OutputMessage("Virtual ESP32 device started");
    return true;
}

bool VMEmulator::Stop() {
    if (!running_) {
        LogExecution("VM is not running");
        return false;
    }
    
    running_ = false;
    LogExecution("VM stopped");
    OutputMessage("Virtual ESP32 device stopped");
    return true;
}

bool VMEmulator::Reset() {
    bool was_running = running_;
    
    if (was_running) {
        Stop();
    }
    
    InitializeGPIOPins();
    InitializeMemory();
    
    wifi_config_ = WiFiConfig();
    bluetooth_config_ = BluetoothConfig();
    peripheral_status_ = PeripheralStatus();
    
    serial_output_.clear();
    execution_log_.clear();
    
    LogExecution("VM reset");
    OutputMessage("Virtual ESP32 device reset");
    
    if (was_running) {
        Start();
    }
    
    return true;
}

bool VMEmulator::IsRunning() const {
    return running_;
}

bool VMEmulator::IsPinValid(int pin) const {
    return gpio_pins_.find(pin) != gpio_pins_.end();
}

bool VMEmulator::SetPinMode(int pin, PinMode mode) {
    if (!IsPinValid(pin)) {
        LogExecution("Invalid pin: " + std::to_string(pin));
        return false;
    }
    
    gpio_pins_[pin].mode = mode;
    LogExecution("Pin " + std::to_string(pin) + " set to mode " + std::to_string(static_cast<int>(mode)));
    return true;
}

VMEmulator::PinMode VMEmulator::GetPinMode(int pin) const {
    auto it = gpio_pins_.find(pin);
    if (it != gpio_pins_.end()) {
        return it->second.mode;
    }
    return PinMode::INPUT;
}

bool VMEmulator::SetPinState(int pin, PinState state) {
    if (!IsPinValid(pin)) {
        LogExecution("Invalid pin: " + std::to_string(pin));
        return false;
    }
    
    if (gpio_pins_[pin].mode != PinMode::OUTPUT) {
        LogExecution("Pin " + std::to_string(pin) + " is not in OUTPUT mode");
        return false;
    }
    
    PinState old_state = gpio_pins_[pin].state;
    gpio_pins_[pin].state = state;
    
    if (old_state != state && pin_change_callback_) {
        pin_change_callback_(pin, state);
    }
    
    LogExecution("Pin " + std::to_string(pin) + " set to " + 
                 (state == PinState::HIGH ? "HIGH" : "LOW"));
    return true;
}

VMEmulator::PinState VMEmulator::GetPinState(int pin) const {
    auto it = gpio_pins_.find(pin);
    if (it != gpio_pins_.end()) {
        return it->second.state;
    }
    return PinState::LOW;
}

std::vector<VMEmulator::GPIOPin> VMEmulator::GetAllPins() const {
    std::vector<GPIOPin> pins;
    for (const auto& pair : gpio_pins_) {
        pins.push_back(pair.second);
    }
    return pins;
}

bool VMEmulator::EnablePWM(int pin, int frequency) {
    if (!IsPinValid(pin)) {
        return false;
    }
    
    gpio_pins_[pin].pwm_enabled = true;
    gpio_pins_[pin].pwm_value = 0;
    LogExecution("PWM enabled on pin " + std::to_string(pin) + 
                 " at " + std::to_string(frequency) + "Hz");
    return true;
}

bool VMEmulator::SetPWMValue(int pin, int value) {
    if (!IsPinValid(pin)) {
        return false;
    }
    
    if (!gpio_pins_[pin].pwm_enabled) {
        LogExecution("PWM not enabled on pin " + std::to_string(pin));
        return false;
    }
    
    gpio_pins_[pin].pwm_value = std::max(0, std::min(255, value));
    LogExecution("PWM value on pin " + std::to_string(pin) + 
                 " set to " + std::to_string(gpio_pins_[pin].pwm_value));
    return true;
}

int VMEmulator::GetPWMValue(int pin) const {
    auto it = gpio_pins_.find(pin);
    if (it != gpio_pins_.end()) {
        return it->second.pwm_value;
    }
    return 0;
}

bool VMEmulator::EnableADC(int pin) {
    if (!IsPinValid(pin)) {
        return false;
    }
    
    gpio_pins_[pin].adc_enabled = true;
    LogExecution("ADC enabled on pin " + std::to_string(pin));
    return true;
}

int VMEmulator::ReadADC(int pin) {
    if (!IsPinValid(pin)) {
        return 0;
    }
    
    if (!gpio_pins_[pin].adc_enabled) {
        LogExecution("ADC not enabled on pin " + std::to_string(pin));
        return 0;
    }
    
    // Return simulated ADC value
    return gpio_pins_[pin].adc_value;
}

bool VMEmulator::ConfigureWiFi(WiFiMode mode, const std::string& ssid, const std::string& password) {
    wifi_config_.mode = mode;
    wifi_config_.ssid = ssid;
    wifi_config_.password = password;
    wifi_config_.status = WiFiStatus::DISCONNECTED;
    wifi_config_.mac_address = GenerateMacAddress();
    
    LogExecution("WiFi configured - Mode: " + std::to_string(static_cast<int>(mode)) + 
                 ", SSID: " + ssid);
    return true;
}

VMEmulator::WiFiStatus VMEmulator::ConnectWiFi() {
    if (wifi_config_.mode == WiFiMode::OFF) {
        LogExecution("WiFi is OFF");
        return WiFiStatus::FAILED;
    }
    
    wifi_config_.status = WiFiStatus::CONNECTING;
    LogExecution("Connecting to WiFi: " + wifi_config_.ssid);
    
    // Simulate connection
    wifi_config_.status = WiFiStatus::CONNECTED;
    wifi_config_.ip_address = GenerateIPAddress();
    wifi_config_.signal_strength = -45; // Good signal
    
    LogExecution("WiFi connected - IP: " + wifi_config_.ip_address);
    return WiFiStatus::CONNECTED;
}

bool VMEmulator::DisconnectWiFi() {
    wifi_config_.status = WiFiStatus::DISCONNECTED;
    wifi_config_.ip_address = "";
    LogExecution("WiFi disconnected");
    return true;
}

VMEmulator::WiFiConfig VMEmulator::GetWiFiConfig() const {
    return wifi_config_;
}

void VMEmulator::SimulateWiFiData(const std::string& data) {
    if (wifi_config_.status == WiFiStatus::CONNECTED) {
        WriteSerial("WiFi Data: " + data);
        LogExecution("WiFi data received: " + data);
    }
}

bool VMEmulator::ConfigureBluetooth(BluetoothMode mode, const std::string& device_name) {
    bluetooth_config_.mode = mode;
    bluetooth_config_.device_name = device_name;
    bluetooth_config_.mac_address = GenerateMacAddress();
    bluetooth_config_.connected = false;
    
    LogExecution("Bluetooth configured - Mode: " + std::to_string(static_cast<int>(mode)) + 
                 ", Name: " + device_name);
    return true;
}

bool VMEmulator::EnableBluetooth() {
    if (bluetooth_config_.mode == BluetoothMode::OFF) {
        LogExecution("Bluetooth mode is OFF");
        return false;
    }
    
    bluetooth_config_.connected = true;
    LogExecution("Bluetooth enabled");
    return true;
}

bool VMEmulator::DisableBluetooth() {
    bluetooth_config_.connected = false;
    LogExecution("Bluetooth disabled");
    return true;
}

VMEmulator::BluetoothConfig VMEmulator::GetBluetoothConfig() const {
    return bluetooth_config_;
}

void VMEmulator::SimulateBluetoothData(const std::string& data) {
    if (bluetooth_config_.connected) {
        WriteSerial("BT Data: " + data);
        LogExecution("Bluetooth data received: " + data);
    }
}

VMEmulator::MemoryStatus VMEmulator::GetMemoryStatus() const {
    return memory_status_;
}

bool VMEmulator::AllocateMemory(size_t bytes) {
    if (bytes > memory_status_.free_heap) {
        LogExecution("Not enough free heap memory");
        return false;
    }
    
    allocated_heap_ += bytes;
    memory_status_.free_heap -= bytes;
    
    // Update fragmentation estimate
    memory_status_.fragmentation_percent = 
        (static_cast<float>(allocated_heap_) / memory_status_.total_heap) * 100.0f * 0.3f;
    
    LogExecution("Allocated " + std::to_string(bytes) + " bytes");
    return true;
}

bool VMEmulator::FreeMemory(size_t bytes) {
    if (bytes > allocated_heap_) {
        bytes = allocated_heap_;
    }
    
    allocated_heap_ -= bytes;
    memory_status_.free_heap += bytes;
    
    // Update fragmentation estimate
    memory_status_.fragmentation_percent = 
        (static_cast<float>(allocated_heap_) / memory_status_.total_heap) * 100.0f * 0.3f;
    
    LogExecution("Freed " + std::to_string(bytes) + " bytes");
    return true;
}

void VMEmulator::SimulateMemoryUsage(size_t heap_used, size_t psram_used) {
    allocated_heap_ = std::min(heap_used, memory_status_.total_heap);
    allocated_psram_ = std::min(psram_used, memory_status_.total_psram);
    
    memory_status_.free_heap = memory_status_.total_heap - allocated_heap_;
    memory_status_.free_psram = memory_status_.total_psram - allocated_psram_;
    
    memory_status_.fragmentation_percent = 
        (static_cast<float>(allocated_heap_) / memory_status_.total_heap) * 100.0f * 0.3f;
}

bool VMEmulator::EnablePeripheral(const std::string& peripheral) {
    if (peripheral == "SPI") {
        peripheral_status_.spi_enabled = true;
    } else if (peripheral == "I2C") {
        peripheral_status_.i2c_enabled = true;
    } else if (peripheral == "UART") {
        peripheral_status_.uart_enabled = true;
    } else if (peripheral == "DAC") {
        peripheral_status_.dac_enabled = true;
    } else if (peripheral == "ADC") {
        peripheral_status_.adc_enabled = true;
    } else {
        peripheral_status_.custom_peripherals[peripheral] = true;
    }
    
    LogExecution("Peripheral enabled: " + peripheral);
    return true;
}

bool VMEmulator::DisablePeripheral(const std::string& peripheral) {
    if (peripheral == "SPI") {
        peripheral_status_.spi_enabled = false;
    } else if (peripheral == "I2C") {
        peripheral_status_.i2c_enabled = false;
    } else if (peripheral == "UART") {
        peripheral_status_.uart_enabled = false;
    } else if (peripheral == "DAC") {
        peripheral_status_.dac_enabled = false;
    } else if (peripheral == "ADC") {
        peripheral_status_.adc_enabled = false;
    } else {
        peripheral_status_.custom_peripherals[peripheral] = false;
    }
    
    LogExecution("Peripheral disabled: " + peripheral);
    return true;
}

VMEmulator::PeripheralStatus VMEmulator::GetPeripheralStatus() const {
    return peripheral_status_;
}

void VMEmulator::WriteSerial(const std::string& data) {
    serial_output_.push_back(data);
    OutputMessage(data);
}

std::vector<std::string> VMEmulator::ReadSerialOutput() const {
    return serial_output_;
}

void VMEmulator::ClearSerialOutput() {
    serial_output_.clear();
}

VMEmulator::ExecutionResult VMEmulator::ExecuteCode(const std::string& code) {
    ExecutionResult result;
    result.success = true;
    result.execution_time_ms = 0;
    result.memory_used = 0;
    
    if (!running_) {
        result.success = false;
        result.errors.push_back("VM is not running");
        return result;
    }
    
    LogExecution("Executing code on virtual device");
    
    // Simulate code execution
    result.output = "Code executed successfully on " + GetBoardName();
    result.execution_time_ms = 150; // Simulated execution time
    result.memory_used = 1024; // Simulated memory usage
    
    // Simulate some basic checks
    if (code.find("setup()") == std::string::npos) {
        result.errors.push_back("Missing setup() function");
        result.success = false;
    }
    
    if (code.find("loop()") == std::string::npos) {
        result.errors.push_back("Missing loop() function");
        result.success = false;
    }
    
    return result;
}

void VMEmulator::SetOutputCallback(OutputCallback callback) {
    output_callback_ = callback;
}

void VMEmulator::SetPinChangeCallback(PinChangeCallback callback) {
    pin_change_callback_ = callback;
}

void VMEmulator::SimulateSensorData(int pin, int value) {
    if (!IsPinValid(pin)) {
        return;
    }
    
    gpio_pins_[pin].adc_value = std::max(0, std::min(4095, value));
    LogExecution("Sensor data simulated on pin " + std::to_string(pin) + 
                 ": " + std::to_string(value));
}

void VMEmulator::SimulateInterrupt(int pin) {
    if (!IsPinValid(pin)) {
        return;
    }
    
    LogExecution("Interrupt triggered on pin " + std::to_string(pin));
    WriteSerial("INTERRUPT: Pin " + std::to_string(pin));
}

void VMEmulator::SimulateDelay(size_t milliseconds) {
    LogExecution("Delay: " + std::to_string(milliseconds) + "ms");
}

std::vector<std::string> VMEmulator::GetExecutionLog() const {
    return execution_log_;
}

void VMEmulator::ClearExecutionLog() {
    execution_log_.clear();
}

VMEmulator::ValidationResult VMEmulator::ValidateConfiguration() const {
    ValidationResult result;
    result.valid = true;
    
    // Check if device configuration is valid
    if (device_config_.available_gpio_pins.empty()) {
        result.valid = false;
        result.errors.push_back("No GPIO pins available");
    }
    
    if (device_config_.sram_size_kb == 0) {
        result.valid = false;
        result.errors.push_back("No SRAM configured");
    }
    
    // Check memory status
    if (allocated_heap_ > memory_status_.total_heap) {
        result.valid = false;
        result.errors.push_back("Heap allocation exceeds total heap size");
    }
    
    // Warnings
    if (memory_status_.fragmentation_percent > 50.0f) {
        result.warnings.push_back("High memory fragmentation: " + 
                                   std::to_string(memory_status_.fragmentation_percent) + "%");
    }
    
    if (wifi_config_.mode != WiFiMode::OFF && wifi_config_.ssid.empty()) {
        result.warnings.push_back("WiFi enabled but no SSID configured");
    }
    
    return result;
}

void VMEmulator::LogExecution(const std::string& message) {
    execution_log_.push_back(message);
}

void VMEmulator::OutputMessage(const std::string& message) {
    if (output_callback_) {
        output_callback_(message);
    }
}

std::string VMEmulator::GenerateMacAddress() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    for (int i = 0; i < 6; ++i) {
        if (i > 0) oss << ":";
        oss << std::setw(2) << (std::rand() % 256);
    }
    
    return oss.str();
}

std::string VMEmulator::GenerateIPAddress() const {
    std::ostringstream oss;
    oss << "192.168." << (1 + std::rand() % 255) << "." << (1 + std::rand() % 254);
    return oss.str();
}

} // namespace esp32_ide
