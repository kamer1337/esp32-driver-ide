#ifndef VM_EMULATOR_H
#define VM_EMULATOR_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace esp32_ide {

/**
 * @brief Virtual Machine Emulator for ESP32 devices
 * 
 * Provides a virtual environment for testing device configurations
 * without requiring physical hardware. Simulates GPIO, WiFi, Bluetooth,
 * memory, and other peripherals.
 */
class VMEmulator {
public:
    enum class BoardType {
        ESP32,
        ESP32_S2,
        ESP32_S3,
        ESP32_C3
    };
    
    enum class PinMode {
        INPUT,
        OUTPUT,
        INPUT_PULLUP,
        INPUT_PULLDOWN
    };
    
    enum class PinState {
        LOW,
        HIGH
    };
    
    enum class WiFiMode {
        OFF,
        STATION,
        ACCESS_POINT,
        STATION_AP
    };
    
    enum class WiFiStatus {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED
    };
    
    enum class BluetoothMode {
        OFF,
        CLASSIC,
        BLE,
        DUAL
    };
    
    struct DeviceConfig {
        BoardType board_type;
        size_t flash_size_mb;
        size_t psram_size_mb;
        size_t sram_size_kb;
        int cpu_frequency_mhz;
        std::string chip_revision;
        std::vector<int> available_gpio_pins;
    };
    
    struct GPIOPin {
        int pin_number;
        PinMode mode;
        PinState state;
        bool pwm_enabled;
        int pwm_value;  // 0-255
        bool adc_enabled;
        int adc_value;  // 0-4095
    };
    
    struct WiFiConfig {
        WiFiMode mode;
        WiFiStatus status;
        std::string ssid;
        std::string password;
        std::string ip_address;
        std::string mac_address;
        int signal_strength;  // -100 to 0 dBm
    };
    
    struct BluetoothConfig {
        BluetoothMode mode;
        bool connected;
        std::string device_name;
        std::string mac_address;
        std::vector<std::string> paired_devices;
    };
    
    struct MemoryStatus {
        size_t free_heap;
        size_t total_heap;
        size_t free_psram;
        size_t total_psram;
        size_t largest_free_block;
        float fragmentation_percent;
    };
    
    struct PeripheralStatus {
        bool spi_enabled;
        bool i2c_enabled;
        bool uart_enabled;
        bool dac_enabled;
        bool adc_enabled;
        std::map<std::string, bool> custom_peripherals;
    };
    
    using OutputCallback = std::function<void(const std::string&)>;
    using PinChangeCallback = std::function<void(int, PinState)>;
    
    VMEmulator();
    explicit VMEmulator(BoardType board);
    ~VMEmulator();
    
    // Device configuration
    void SetBoardType(BoardType board);
    BoardType GetBoardType() const;
    DeviceConfig GetDeviceConfig() const;
    std::string GetBoardName() const;
    
    // Virtual machine lifecycle
    bool Start();
    bool Stop();
    bool Reset();
    bool IsRunning() const;
    
    // GPIO operations
    bool SetPinMode(int pin, PinMode mode);
    PinMode GetPinMode(int pin) const;
    bool SetPinState(int pin, PinState state);
    PinState GetPinState(int pin) const;
    bool IsPinValid(int pin) const;
    std::vector<GPIOPin> GetAllPins() const;
    
    // PWM operations
    bool EnablePWM(int pin, int frequency);
    bool SetPWMValue(int pin, int value);  // 0-255
    int GetPWMValue(int pin) const;
    
    // ADC operations
    bool EnableADC(int pin);
    int ReadADC(int pin);
    
    // WiFi simulation
    bool ConfigureWiFi(WiFiMode mode, const std::string& ssid, const std::string& password);
    WiFiStatus ConnectWiFi();
    bool DisconnectWiFi();
    WiFiConfig GetWiFiConfig() const;
    void SimulateWiFiData(const std::string& data);
    
    // Bluetooth simulation
    bool ConfigureBluetooth(BluetoothMode mode, const std::string& device_name);
    bool EnableBluetooth();
    bool DisableBluetooth();
    BluetoothConfig GetBluetoothConfig() const;
    void SimulateBluetoothData(const std::string& data);
    
    // Memory management
    MemoryStatus GetMemoryStatus() const;
    bool AllocateMemory(size_t bytes);
    bool FreeMemory(size_t bytes);
    void SimulateMemoryUsage(size_t heap_used, size_t psram_used);
    
    // Peripheral management
    bool EnablePeripheral(const std::string& peripheral);
    bool DisablePeripheral(const std::string& peripheral);
    PeripheralStatus GetPeripheralStatus() const;
    
    // Serial output simulation
    void WriteSerial(const std::string& data);
    std::vector<std::string> ReadSerialOutput() const;
    void ClearSerialOutput();
    
    // Code execution simulation
    struct ExecutionResult {
        bool success;
        std::string output;
        std::vector<std::string> errors;
        size_t execution_time_ms;
        size_t memory_used;
    };
    
    ExecutionResult ExecuteCode(const std::string& code);
    
    // Callbacks
    void SetOutputCallback(OutputCallback callback);
    void SetPinChangeCallback(PinChangeCallback callback);
    
    // Testing utilities
    void SimulateSensorData(int pin, int value);
    void SimulateInterrupt(int pin);
    void SimulateDelay(size_t milliseconds);
    std::vector<std::string> GetExecutionLog() const;
    void ClearExecutionLog();
    
    // Configuration validation
    struct ValidationResult {
        bool valid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    ValidationResult ValidateConfiguration() const;
    
private:
    BoardType board_type_;
    DeviceConfig device_config_;
    bool running_;
    
    std::map<int, GPIOPin> gpio_pins_;
    WiFiConfig wifi_config_;
    BluetoothConfig bluetooth_config_;
    MemoryStatus memory_status_;
    PeripheralStatus peripheral_status_;
    
    std::vector<std::string> serial_output_;
    std::vector<std::string> execution_log_;
    
    OutputCallback output_callback_;
    PinChangeCallback pin_change_callback_;
    
    size_t allocated_heap_;
    size_t allocated_psram_;
    
    void InitializeDeviceConfig();
    void InitializeGPIOPins();
    void InitializeMemory();
    void LogExecution(const std::string& message);
    void OutputMessage(const std::string& message);
    std::string GenerateMacAddress() const;
    std::string GenerateIPAddress() const;
};

} // namespace esp32_ide

#endif // VM_EMULATOR_H
