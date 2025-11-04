#ifndef SERIAL_MONITOR_H
#define SERIAL_MONITOR_H

#include <string>
#include <vector>
#include <functional>

namespace esp32_ide {

/**
 * @brief Serial monitor for ESP32 communication
 * 
 * Handles serial communication with ESP32 devices
 */
class SerialMonitor {
public:
    enum class MessageType {
        NORMAL,
        ERROR,
        WARNING,
        SUCCESS,
        INFO
    };
    
    struct SerialMessage {
        std::string content;
        MessageType type;
        long long timestamp;
    };
    
    using MessageCallback = std::function<void(const SerialMessage&)>;
    
    SerialMonitor();
    ~SerialMonitor();
    
    // Connection
    bool Connect(const std::string& port, int baud_rate);
    bool Disconnect();
    bool IsConnected() const;
    
    // Communication
    bool SendData(const std::string& data);
    std::string ReceiveData();
    
    // Message handling
    void AddMessage(const std::string& content, MessageType type = MessageType::NORMAL);
    std::vector<SerialMessage> GetMessages() const;
    void ClearMessages();
    
    // Callbacks
    void SetMessageCallback(MessageCallback callback);
    
    // Port management
    static std::vector<std::string> GetAvailablePorts();
    std::string GetCurrentPort() const;
    int GetBaudRate() const;
    
    // Realtime data reading
    void StartRealtimeReading();
    void StopRealtimeReading();
    bool IsRealtimeReading() const;
    std::vector<std::string> GetRealtimeData() const;
    void ClearRealtimeData();
    
    // Memory profiling
    struct MemoryProfile {
        size_t free_heap;
        size_t total_heap;
        size_t free_psram;
        size_t largest_free_block;
        float fragmentation_percent;
        std::vector<std::string> warnings;
    };
    
    MemoryProfile GetMemoryProfile() const;
    void StartMemoryProfiling();
    void StopMemoryProfiling();
    bool IsMemoryProfiling() const;
    std::vector<MemoryProfile> GetMemoryHistory() const;
    
    // Variable watching for debugging
    struct WatchVariable {
        std::string name;
        std::string value;
        std::string type;
        long long last_update;
    };
    
    void AddWatchVariable(const std::string& name, const std::string& type);
    void RemoveWatchVariable(const std::string& name);
    std::vector<WatchVariable> GetWatchVariables() const;
    void UpdateWatchVariable(const std::string& name, const std::string& value);
    
private:
    bool connected_;
    std::string current_port_;
    int baud_rate_;
    std::vector<SerialMessage> messages_;
    MessageCallback message_callback_;
    bool realtime_reading_;
    std::vector<std::string> realtime_data_;
    bool memory_profiling_;
    std::vector<MemoryProfile> memory_history_;
    std::vector<WatchVariable> watch_variables_;
    
    void NotifyMessage(const SerialMessage& message);
    void SimulateDataReading();
    void SimulateMemoryProfiling();
};

} // namespace esp32_ide

#endif // SERIAL_MONITOR_H
