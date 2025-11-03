#include "serial/serial_monitor.h"
#include <chrono>

namespace esp32_ide {

SerialMonitor::SerialMonitor() 
    : connected_(false), current_port_(""), baud_rate_(115200), realtime_reading_(false) {}

SerialMonitor::~SerialMonitor() {
    Disconnect();
}

bool SerialMonitor::Connect(const std::string& port, int baud_rate) {
    if (connected_) {
        Disconnect();
    }
    
    current_port_ = port;
    baud_rate_ = baud_rate;
    
    // In a real implementation, this would open the serial port
    // For now, simulate connection
    connected_ = true;
    
    AddMessage("Connected to " + port + " at " + std::to_string(baud_rate) + " baud", 
               MessageType::SUCCESS);
    
    return true;
}

bool SerialMonitor::Disconnect() {
    if (!connected_) {
        return false;
    }
    
    // In a real implementation, this would close the serial port
    connected_ = false;
    
    AddMessage("Disconnected from " + current_port_, MessageType::INFO);
    current_port_ = "";
    
    return true;
}

bool SerialMonitor::IsConnected() const {
    return connected_;
}

bool SerialMonitor::SendData(const std::string& data) {
    if (!connected_) {
        return false;
    }
    
    // In a real implementation, this would send data to the serial port
    AddMessage("Sent: " + data, MessageType::INFO);
    return true;
}

std::string SerialMonitor::ReceiveData() {
    if (!connected_) {
        return "";
    }
    
    // In a real implementation, this would read from the serial port
    return "";
}

void SerialMonitor::AddMessage(const std::string& content, MessageType type) {
    SerialMessage msg;
    msg.content = content;
    msg.type = type;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    messages_.push_back(msg);
    NotifyMessage(msg);
}

std::vector<SerialMonitor::SerialMessage> SerialMonitor::GetMessages() const {
    return messages_;
}

void SerialMonitor::ClearMessages() {
    messages_.clear();
}

void SerialMonitor::SetMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

std::vector<std::string> SerialMonitor::GetAvailablePorts() {
    // In a real implementation, this would scan for available serial ports
    // For simulation, return common port names
    std::vector<std::string> ports;
    
#ifdef _WIN32
    ports.push_back("COM1");
    ports.push_back("COM3");
    ports.push_back("COM4");
#else
    ports.push_back("/dev/ttyUSB0");
    ports.push_back("/dev/ttyUSB1");
    ports.push_back("/dev/ttyACM0");
#endif
    
    return ports;
}

std::string SerialMonitor::GetCurrentPort() const {
    return current_port_;
}

int SerialMonitor::GetBaudRate() const {
    return baud_rate_;
}

void SerialMonitor::StartRealtimeReading() {
    if (!connected_) {
        AddMessage("Cannot start realtime reading: Not connected", MessageType::ERROR);
        return;
    }
    
    realtime_reading_ = true;
    realtime_data_.clear();
    AddMessage("Started realtime data reading", MessageType::SUCCESS);
    
    // Simulate initial data
    SimulateDataReading();
}

void SerialMonitor::StopRealtimeReading() {
    realtime_reading_ = false;
    AddMessage("Stopped realtime data reading", MessageType::INFO);
}

bool SerialMonitor::IsRealtimeReading() const {
    return realtime_reading_;
}

std::vector<std::string> SerialMonitor::GetRealtimeData() const {
    return realtime_data_;
}

void SerialMonitor::ClearRealtimeData() {
    realtime_data_.clear();
}

void SerialMonitor::SimulateDataReading() {
    // Simulate receiving data from device
    // In a real implementation, this would read from the actual serial port
    if (!realtime_reading_) {
        return;
    }
    
    // Simulate various types of ESP32 data
    realtime_data_.push_back("[0.000] ESP32 Boot");
    realtime_data_.push_back("[0.100] WiFi: Connecting...");
    realtime_data_.push_back("[0.500] WiFi: Connected");
    realtime_data_.push_back("[1.000] IP Address: 192.168.1.100");
    realtime_data_.push_back("[2.000] Temperature: 25.3°C");
    realtime_data_.push_back("[2.500] Humidity: 60.2%");
    realtime_data_.push_back("[3.000] GPIO2: HIGH");
    realtime_data_.push_back("[3.500] Sensor Reading: 1023");
    realtime_data_.push_back("[4.000] Free Heap: 280000 bytes");
    realtime_data_.push_back("[4.500] Loop iteration: 100");
}

void SerialMonitor::NotifyMessage(const SerialMessage& message) {
    if (message_callback_) {
        message_callback_(message);
    }
}

} // namespace esp32_ide
