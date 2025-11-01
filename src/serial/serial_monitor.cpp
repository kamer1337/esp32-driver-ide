#include "serial/serial_monitor.h"
#include <chrono>

namespace esp32_ide {

SerialMonitor::SerialMonitor() 
    : connected_(false), current_port_(""), baud_rate_(115200) {}

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

void SerialMonitor::NotifyMessage(const SerialMessage& message) {
    if (message_callback_) {
        message_callback_(message);
    }
}

} // namespace esp32_ide
