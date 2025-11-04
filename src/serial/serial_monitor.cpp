#include "serial/serial_monitor.h"
#include <chrono>

namespace esp32_ide {

SerialMonitor::SerialMonitor() 
    : connected_(false), current_port_(""), baud_rate_(115200), 
      realtime_reading_(false), memory_profiling_(false) {}

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

// Memory profiling implementation
SerialMonitor::MemoryProfile SerialMonitor::GetMemoryProfile() const {
    MemoryProfile profile;
    
    // Simulate memory profile data
    profile.total_heap = 327680; // ESP32 typical heap size
    profile.free_heap = 280000;
    profile.free_psram = 0;
    profile.largest_free_block = 110000;
    profile.fragmentation_percent = ((profile.total_heap - profile.free_heap - profile.largest_free_block) * 100.0f) / profile.total_heap;
    
    // Generate warnings based on memory state
    if (profile.free_heap < 20000) {
        profile.warnings.push_back("CRITICAL: Low free heap (<20KB)");
    } else if (profile.free_heap < 50000) {
        profile.warnings.push_back("WARNING: Free heap getting low (<50KB)");
    }
    
    if (profile.fragmentation_percent > 30.0f) {
        profile.warnings.push_back("High memory fragmentation detected");
    }
    
    return profile;
}

void SerialMonitor::StartMemoryProfiling() {
    memory_profiling_ = true;
    memory_history_.clear();
    SimulateMemoryProfiling();
}

void SerialMonitor::StopMemoryProfiling() {
    memory_profiling_ = false;
}

bool SerialMonitor::IsMemoryProfiling() const {
    return memory_profiling_;
}

std::vector<SerialMonitor::MemoryProfile> SerialMonitor::GetMemoryHistory() const {
    return memory_history_;
}

void SerialMonitor::SimulateMemoryProfiling() {
    if (!memory_profiling_) {
        return;
    }
    
    // Simulate memory snapshots at different points
    for (int i = 0; i < 5; i++) {
        MemoryProfile profile;
        profile.total_heap = 327680;
        profile.free_heap = 280000 - (i * 10000); // Simulate decreasing free heap
        profile.free_psram = 0;
        profile.largest_free_block = 110000 - (i * 5000);
        profile.fragmentation_percent = ((profile.total_heap - profile.free_heap - profile.largest_free_block) * 100.0f) / profile.total_heap;
        
        if (profile.free_heap < 20000) {
            profile.warnings.push_back("CRITICAL: Low free heap");
        }
        
        memory_history_.push_back(profile);
    }
}

// Variable watching implementation
void SerialMonitor::AddWatchVariable(const std::string& name, const std::string& type) {
    WatchVariable var;
    var.name = name;
    var.type = type;
    var.value = "N/A";
    var.last_update = std::chrono::system_clock::now().time_since_epoch().count();
    watch_variables_.push_back(var);
}

void SerialMonitor::RemoveWatchVariable(const std::string& name) {
    watch_variables_.erase(
        std::remove_if(watch_variables_.begin(), watch_variables_.end(),
                      [&name](const WatchVariable& var) { return var.name == name; }),
        watch_variables_.end()
    );
}

std::vector<SerialMonitor::WatchVariable> SerialMonitor::GetWatchVariables() const {
    return watch_variables_;
}

void SerialMonitor::UpdateWatchVariable(const std::string& name, const std::string& value) {
    for (auto& var : watch_variables_) {
        if (var.name == name) {
            var.value = value;
            var.last_update = std::chrono::system_clock::now().time_since_epoch().count();
            break;
        }
    }
}

} // namespace esp32_ide
