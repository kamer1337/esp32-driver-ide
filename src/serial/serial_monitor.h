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
    
private:
    bool connected_;
    std::string current_port_;
    int baud_rate_;
    std::vector<SerialMessage> messages_;
    MessageCallback message_callback_;
    bool realtime_reading_;
    std::vector<std::string> realtime_data_;
    
    void NotifyMessage(const SerialMessage& message);
    void SimulateDataReading();
};

} // namespace esp32_ide

#endif // SERIAL_MONITOR_H
