#include "platform/platform_expansion.h"
#include <sstream>
#include <algorithm>

namespace esp32_ide {
namespace platform {

// ============================================================================
// MultiBoardSupport Implementation
// ============================================================================

MultiBoardSupport::MultiBoardSupport() {
    InitializeDefaultBoards();
}

void MultiBoardSupport::InitializeDefaultBoards() {
    // ESP32 family
    BoardConfig esp32;
    esp32.id = "esp32";
    esp32.name = "ESP32 Dev Module";
    esp32.family = BoardFamily::ESP32;
    esp32.architecture = "xtensa";
    esp32.chip = "ESP32";
    esp32.flash_size_kb = 4096;
    esp32.ram_size_kb = 520;
    esp32.eeprom_size_bytes = 4096;
    esp32.cpu_frequency_mhz = 240;
    esp32.available_pins = {0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};
    esp32.supported_features = {"wifi", "bluetooth", "ble", "adc", "dac", "i2c", "spi", "uart", "pwm", "touch"};
    RegisterBoard(esp32);
    
    // ESP8266
    BoardConfig esp8266;
    esp8266.id = "esp8266";
    esp8266.name = "ESP8266 Generic";
    esp8266.family = BoardFamily::ESP8266;
    esp8266.architecture = "xtensa";
    esp8266.chip = "ESP8266";
    esp8266.flash_size_kb = 4096;
    esp8266.ram_size_kb = 80;
    esp8266.eeprom_size_bytes = 4096;
    esp8266.cpu_frequency_mhz = 80;
    esp8266.available_pins = {0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16};
    esp8266.supported_features = {"wifi", "adc", "i2c", "spi", "uart", "pwm"};
    RegisterBoard(esp8266);
    
    BoardConfig nodemcu;
    nodemcu.id = "nodemcu";
    nodemcu.name = "NodeMCU 1.0";
    nodemcu.family = BoardFamily::ESP8266;
    nodemcu.architecture = "xtensa";
    nodemcu.chip = "ESP8266";
    nodemcu.flash_size_kb = 4096;
    nodemcu.ram_size_kb = 80;
    nodemcu.eeprom_size_bytes = 4096;
    nodemcu.cpu_frequency_mhz = 80;
    nodemcu.available_pins = {0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16};
    nodemcu.supported_features = {"wifi", "adc", "i2c", "spi", "uart", "pwm"};
    RegisterBoard(nodemcu);
    
    // STM32 family
    BoardConfig stm32f103;
    stm32f103.id = "stm32f103";
    stm32f103.name = "STM32F103C8 (Blue Pill)";
    stm32f103.family = BoardFamily::STM32;
    stm32f103.architecture = "arm";
    stm32f103.chip = "STM32F103C8T6";
    stm32f103.flash_size_kb = 64;
    stm32f103.ram_size_kb = 20;
    stm32f103.eeprom_size_bytes = 0;
    stm32f103.cpu_frequency_mhz = 72;
    stm32f103.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    stm32f103.supported_features = {"usb", "can", "i2c", "spi", "uart", "pwm", "adc", "dma"};
    RegisterBoard(stm32f103);
    
    BoardConfig stm32f401;
    stm32f401.id = "stm32f401";
    stm32f401.name = "STM32F401 (Black Pill)";
    stm32f401.family = BoardFamily::STM32;
    stm32f401.architecture = "arm";
    stm32f401.chip = "STM32F401CCU6";
    stm32f401.flash_size_kb = 256;
    stm32f401.ram_size_kb = 64;
    stm32f401.eeprom_size_bytes = 0;
    stm32f401.cpu_frequency_mhz = 84;
    stm32f401.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    stm32f401.supported_features = {"usb", "i2c", "spi", "uart", "pwm", "adc", "dma"};
    RegisterBoard(stm32f401);
    
    BoardConfig stm32f407;
    stm32f407.id = "stm32f407";
    stm32f407.name = "STM32F407 Discovery";
    stm32f407.family = BoardFamily::STM32;
    stm32f407.architecture = "arm";
    stm32f407.chip = "STM32F407VGT6";
    stm32f407.flash_size_kb = 1024;
    stm32f407.ram_size_kb = 192;
    stm32f407.eeprom_size_bytes = 0;
    stm32f407.cpu_frequency_mhz = 168;
    stm32f407.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    stm32f407.supported_features = {"usb", "ethernet", "can", "i2c", "spi", "uart", "pwm", "adc", "dac", "dma"};
    RegisterBoard(stm32f407);
    
    // Arduino family
    BoardConfig uno;
    uno.id = "arduino_uno";
    uno.name = "Arduino Uno";
    uno.family = BoardFamily::ARDUINO;
    uno.architecture = "avr";
    uno.chip = "ATmega328P";
    uno.flash_size_kb = 32;
    uno.ram_size_kb = 2;
    uno.eeprom_size_bytes = 1024;
    uno.cpu_frequency_mhz = 16;
    uno.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    uno.supported_features = {"adc", "i2c", "spi", "uart", "pwm"};
    RegisterBoard(uno);
    
    BoardConfig mega;
    mega.id = "arduino_mega";
    mega.name = "Arduino Mega 2560";
    mega.family = BoardFamily::ARDUINO;
    mega.architecture = "avr";
    mega.chip = "ATmega2560";
    mega.flash_size_kb = 256;
    mega.ram_size_kb = 8;
    mega.eeprom_size_bytes = 4096;
    mega.cpu_frequency_mhz = 16;
    mega.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
    mega.supported_features = {"adc", "i2c", "spi", "uart", "pwm"};
    RegisterBoard(mega);
    
    BoardConfig nano;
    nano.id = "arduino_nano";
    nano.name = "Arduino Nano";
    nano.family = BoardFamily::ARDUINO;
    nano.architecture = "avr";
    nano.chip = "ATmega328P";
    nano.flash_size_kb = 32;
    nano.ram_size_kb = 2;
    nano.eeprom_size_bytes = 1024;
    nano.cpu_frequency_mhz = 16;
    nano.available_pins = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    nano.supported_features = {"adc", "i2c", "spi", "uart", "pwm"};
    RegisterBoard(nano);
    
    // Default selection
    selected_board_id_ = "esp32";
}

void MultiBoardSupport::RegisterBoard(const BoardConfig& config) {
    boards_[config.id] = config;
}

void MultiBoardSupport::UnregisterBoard(const std::string& board_id) {
    boards_.erase(board_id);
    if (selected_board_id_ == board_id) {
        selected_board_id_.clear();
    }
}

bool MultiBoardSupport::SelectBoard(const std::string& board_id) {
    if (boards_.find(board_id) != boards_.end()) {
        selected_board_id_ = board_id;
        return true;
    }
    return false;
}

const BoardConfig* MultiBoardSupport::GetSelectedBoard() const {
    return GetBoardById(selected_board_id_);
}

std::string MultiBoardSupport::GetSelectedBoardId() const {
    return selected_board_id_;
}

std::vector<BoardConfig> MultiBoardSupport::GetAllBoards() const {
    std::vector<BoardConfig> result;
    for (const auto& pair : boards_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<BoardConfig> MultiBoardSupport::GetBoardsByFamily(BoardFamily family) const {
    std::vector<BoardConfig> result;
    for (const auto& pair : boards_) {
        if (pair.second.family == family) {
            result.push_back(pair.second);
        }
    }
    return result;
}

const BoardConfig* MultiBoardSupport::GetBoardById(const std::string& board_id) const {
    auto it = boards_.find(board_id);
    return (it != boards_.end()) ? &it->second : nullptr;
}

bool MultiBoardSupport::HasFeature(const std::string& feature) const {
    const BoardConfig* board = GetSelectedBoard();
    if (!board) return false;
    
    return std::find(board->supported_features.begin(), 
                     board->supported_features.end(), 
                     feature) != board->supported_features.end();
}

std::vector<std::string> MultiBoardSupport::GetSupportedFeatures() const {
    const BoardConfig* board = GetSelectedBoard();
    if (!board) return {};
    return board->supported_features;
}

std::string MultiBoardSupport::GetBoardDefine() const {
    const BoardConfig* board = GetSelectedBoard();
    if (!board) return "";
    
    switch (board->family) {
        case BoardFamily::ESP32: return "-DESP32";
        case BoardFamily::ESP8266: return "-DESP8266";
        case BoardFamily::STM32: return "-DSTM32 -D" + board->chip;
        case BoardFamily::ARDUINO: return "-DARDUINO -D" + board->chip;
        default: return "";
    }
}

std::string MultiBoardSupport::GetCompilerFlags() const {
    const BoardConfig* board = GetSelectedBoard();
    if (!board) return "";
    
    std::ostringstream flags;
    
    switch (board->family) {
        case BoardFamily::ESP32:
        case BoardFamily::ESP8266:
            flags << "-mlongcalls -mtext-section-literals ";
            break;
        case BoardFamily::STM32:
            flags << "-mthumb -mcpu=cortex-m3 ";
            break;
        case BoardFamily::ARDUINO:
            flags << "-mmcu=" << board->chip << " ";
            break;
        default:
            break;
    }
    
    flags << "-DF_CPU=" << board->cpu_frequency_mhz << "000000L";
    return flags.str();
}

std::string MultiBoardSupport::GetLinkerFlags() const {
    const BoardConfig* board = GetSelectedBoard();
    if (!board) return "";
    
    std::ostringstream flags;
    
    switch (board->family) {
        case BoardFamily::ESP32:
            flags << "-Tesp32.ld -Wl,--gc-sections";
            break;
        case BoardFamily::ESP8266:
            flags << "-Teagle.flash.4m.ld -Wl,--gc-sections";
            break;
        case BoardFamily::STM32:
            flags << "-TSTM32.ld -Wl,--gc-sections";
            break;
        case BoardFamily::ARDUINO:
            flags << "-Wl,--gc-sections";
            break;
        default:
            break;
    }
    
    return flags.str();
}

// ============================================================================
// RTOSIntegration Implementation
// ============================================================================

RTOSIntegration::RTOSIntegration()
    : initialized_(false), next_task_id_(1), scheduler_running_(false) {
    stats_.total_tasks = 0;
    stats_.running_tasks = 0;
    stats_.blocked_tasks = 0;
    stats_.suspended_tasks = 0;
    stats_.context_switches = 0;
    stats_.total_cpu_usage_percent = 0.0f;
}

bool RTOSIntegration::Initialize() {
    if (initialized_) return true;
    
    stats_.uptime_start = std::chrono::system_clock::now();
    initialized_ = true;
    
    // Create idle task
    CreateTask("IDLE", TaskPriority::IDLE, 1024, 0);
    
    return true;
}

void RTOSIntegration::Shutdown() {
    StopScheduler();
    tasks_.clear();
    sync_primitives_.clear();
    initialized_ = false;
}

uint32_t RTOSIntegration::CreateTask(const std::string& name, TaskPriority priority,
                                      size_t stack_size, int cpu_core) {
    if (!initialized_) return 0;
    
    TaskInfo task;
    task.name = name;
    task.task_id = next_task_id_++;
    task.state = TaskState::READY;
    task.priority = priority;
    task.stack_size = stack_size;
    task.stack_high_water_mark = stack_size;
    task.cpu_core = cpu_core;
    task.cpu_usage_percent = 0.0f;
    task.total_runtime_us = 0;
    task.created_at = std::chrono::system_clock::now();
    
    tasks_[task.task_id] = task;
    stats_.total_tasks++;
    
    NotifyTaskChange(task);
    return task.task_id;
}

bool RTOSIntegration::DeleteTask(uint32_t task_id) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) return false;
    
    it->second.state = TaskState::DELETED;
    NotifyTaskChange(it->second);
    tasks_.erase(it);
    stats_.total_tasks--;
    
    return true;
}

bool RTOSIntegration::SuspendTask(uint32_t task_id) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) return false;
    
    it->second.state = TaskState::SUSPENDED;
    stats_.suspended_tasks++;
    NotifyTaskChange(it->second);
    
    return true;
}

bool RTOSIntegration::ResumeTask(uint32_t task_id) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) return false;
    
    if (it->second.state == TaskState::SUSPENDED) {
        it->second.state = TaskState::READY;
        stats_.suspended_tasks--;
        NotifyTaskChange(it->second);
    }
    
    return true;
}

bool RTOSIntegration::SetTaskPriority(uint32_t task_id, TaskPriority priority) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) return false;
    
    it->second.priority = priority;
    NotifyTaskChange(it->second);
    
    return true;
}

TaskInfo RTOSIntegration::GetTaskInfo(uint32_t task_id) const {
    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        return it->second;
    }
    return TaskInfo();
}

std::vector<TaskInfo> RTOSIntegration::GetAllTasks() const {
    std::vector<TaskInfo> result;
    for (const auto& pair : tasks_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<TaskInfo> RTOSIntegration::GetTasksByState(TaskState state) const {
    std::vector<TaskInfo> result;
    for (const auto& pair : tasks_) {
        if (pair.second.state == state) {
            result.push_back(pair.second);
        }
    }
    return result;
}

TaskInfo RTOSIntegration::GetCurrentTask() const {
    for (const auto& pair : tasks_) {
        if (pair.second.state == TaskState::RUNNING) {
            return pair.second;
        }
    }
    return TaskInfo();
}

bool RTOSIntegration::StartScheduler() {
    if (!initialized_) return false;
    scheduler_running_ = true;
    return true;
}

bool RTOSIntegration::StopScheduler() {
    scheduler_running_ = false;
    return true;
}

bool RTOSIntegration::SuspendAll() {
    for (auto& pair : tasks_) {
        if (pair.second.state != TaskState::DELETED) {
            pair.second.state = TaskState::SUSPENDED;
        }
    }
    return true;
}

bool RTOSIntegration::ResumeAll() {
    for (auto& pair : tasks_) {
        if (pair.second.state == TaskState::SUSPENDED) {
            pair.second.state = TaskState::READY;
        }
    }
    return true;
}

SchedulerStats RTOSIntegration::GetSchedulerStats() const {
    return stats_;
}

std::string RTOSIntegration::GenerateTaskDiagram() const {
    std::ostringstream diagram;
    diagram << "FreeRTOS Task Diagram\n";
    diagram << "=====================\n\n";
    
    for (const auto& pair : tasks_) {
        const TaskInfo& task = pair.second;
        diagram << "[" << task.name << "] ";
        
        switch (task.state) {
            case TaskState::READY: diagram << "(READY)"; break;
            case TaskState::RUNNING: diagram << "(RUNNING)"; break;
            case TaskState::BLOCKED: diagram << "(BLOCKED)"; break;
            case TaskState::SUSPENDED: diagram << "(SUSPENDED)"; break;
            case TaskState::DELETED: diagram << "(DELETED)"; break;
        }
        
        diagram << " Pri:" << static_cast<int>(task.priority);
        diagram << " CPU:" << task.cpu_usage_percent << "%\n";
    }
    
    return diagram.str();
}

std::string RTOSIntegration::GenerateGanttChart(int duration_ms) const {
    std::ostringstream chart;
    chart << "Task Gantt Chart (simulated " << duration_ms << "ms)\n";
    chart << std::string(50, '-') << "\n";
    
    for (const auto& pair : tasks_) {
        chart << pair.second.name << ": ";
        int bars = static_cast<int>(pair.second.cpu_usage_percent / 5);
        chart << std::string(bars, '#') << std::string(20 - bars, '.') << "\n";
    }
    
    return chart.str();
}

std::string RTOSIntegration::GenerateCPUUsageChart() const {
    std::ostringstream chart;
    chart << "CPU Usage by Task\n";
    chart << "=================\n\n";
    
    for (const auto& pair : tasks_) {
        chart << pair.second.name << ": " << pair.second.cpu_usage_percent << "%\n";
    }
    
    chart << "\nTotal CPU Usage: " << stats_.total_cpu_usage_percent << "%\n";
    
    return chart.str();
}

std::vector<std::string> RTOSIntegration::AnalyzeScheduling() const {
    std::vector<std::string> analysis;
    
    // Check for high-priority tasks
    int high_priority_count = 0;
    for (const auto& pair : tasks_) {
        if (pair.second.priority >= TaskPriority::HIGH) {
            high_priority_count++;
        }
    }
    
    if (high_priority_count > 2) {
        analysis.push_back("Warning: Multiple high-priority tasks detected. Consider priority optimization.");
    }
    
    // Check stack usage
    for (const auto& pair : tasks_) {
        float stack_usage = 1.0f - (static_cast<float>(pair.second.stack_high_water_mark) / 
                                    static_cast<float>(pair.second.stack_size));
        if (stack_usage > 0.8f) {
            analysis.push_back("Warning: Task '" + pair.second.name + "' has high stack usage.");
        }
    }
    
    if (analysis.empty()) {
        analysis.push_back("Scheduling analysis complete. No issues detected.");
    }
    
    return analysis;
}

std::vector<std::string> RTOSIntegration::DetectPriorityInversion() const {
    std::vector<std::string> inversions;
    
    // Simplified priority inversion detection
    for (const auto& sync : sync_primitives_) {
        if (sync.second.type == "mutex" && !sync.second.owner_task.empty()) {
            for (const auto& waiting : sync.second.waiting_tasks) {
                // Check if waiting task has higher priority than owner
                inversions.push_back("Potential priority inversion: " + waiting + 
                                   " waiting on mutex held by " + sync.second.owner_task);
            }
        }
    }
    
    return inversions;
}

std::vector<std::string> RTOSIntegration::DetectDeadlocks() const {
    std::vector<std::string> deadlocks;
    
    // Simplified deadlock detection - check for circular wait
    // In a real implementation, this would build and analyze a wait-for graph
    
    return deadlocks;
}

float RTOSIntegration::CalculateCPUUtilization() const {
    float total = 0.0f;
    for (const auto& pair : tasks_) {
        if (pair.second.name != "IDLE") {
            total += pair.second.cpu_usage_percent;
        }
    }
    return std::min(100.0f, total);
}

std::string RTOSIntegration::CreateMutex(const std::string& name) {
    SyncPrimitiveInfo mutex;
    mutex.name = name;
    mutex.type = "mutex";
    mutex.count = 1;
    mutex.max_count = 1;
    
    std::string id = "mutex_" + name;
    sync_primitives_[id] = mutex;
    return id;
}

std::string RTOSIntegration::CreateSemaphore(const std::string& name, int max_count) {
    SyncPrimitiveInfo sem;
    sem.name = name;
    sem.type = "semaphore";
    sem.count = max_count;
    sem.max_count = max_count;
    
    std::string id = "sem_" + name;
    sync_primitives_[id] = sem;
    return id;
}

std::string RTOSIntegration::CreateQueue(const std::string& name, int queue_length, size_t /*item_size*/) {
    SyncPrimitiveInfo queue;
    queue.name = name;
    queue.type = "queue";
    queue.count = 0;
    queue.max_count = queue_length;
    
    std::string id = "queue_" + name;
    sync_primitives_[id] = queue;
    return id;
}

bool RTOSIntegration::DeleteSyncPrimitive(const std::string& id) {
    return sync_primitives_.erase(id) > 0;
}

std::vector<SyncPrimitiveInfo> RTOSIntegration::GetAllSyncPrimitives() const {
    std::vector<SyncPrimitiveInfo> result;
    for (const auto& pair : sync_primitives_) {
        result.push_back(pair.second);
    }
    return result;
}

void RTOSIntegration::SetTaskStateChangeCallback(TaskCallback callback) {
    task_callback_ = callback;
}

void RTOSIntegration::UpdateTaskStats() {
    stats_.running_tasks = 0;
    stats_.blocked_tasks = 0;
    stats_.suspended_tasks = 0;
    
    for (const auto& pair : tasks_) {
        switch (pair.second.state) {
            case TaskState::RUNNING: stats_.running_tasks++; break;
            case TaskState::BLOCKED: stats_.blocked_tasks++; break;
            case TaskState::SUSPENDED: stats_.suspended_tasks++; break;
            default: break;
        }
    }
}

void RTOSIntegration::NotifyTaskChange(const TaskInfo& task) {
    if (task_callback_) {
        task_callback_(task);
    }
    UpdateTaskStats();
}

// ============================================================================
// IoTPlatformIntegration Implementation
// ============================================================================

IoTPlatformIntegration::IoTPlatformIntegration()
    : platform_(IoTPlatform::AWS_IOT), connection_status_(ConnectionStatus::DISCONNECTED) {
    shadow_.version = 0;
}

void IoTPlatformIntegration::SetCredentials(const IoTCredentials& credentials) {
    credentials_ = credentials;
}

void IoTPlatformIntegration::SetPlatform(IoTPlatform platform) {
    platform_ = platform;
}

bool IoTPlatformIntegration::Connect() {
    if (credentials_.endpoint.empty()) return false;
    
    connection_status_ = ConnectionStatus::CONNECTING;
    NotifyConnectionChange(connection_status_);
    
    // Simulate connection
    connection_status_ = ConnectionStatus::CONNECTED;
    NotifyConnectionChange(connection_status_);
    
    return true;
}

bool IoTPlatformIntegration::Disconnect() {
    connection_status_ = ConnectionStatus::DISCONNECTED;
    NotifyConnectionChange(connection_status_);
    subscribed_topics_.clear();
    return true;
}

bool IoTPlatformIntegration::Reconnect() {
    Disconnect();
    return Connect();
}

bool IoTPlatformIntegration::Publish(const std::string& topic, const std::string& payload, int qos) {
    if (connection_status_ != ConnectionStatus::CONNECTED) return false;
    
    IoTMessage msg;
    msg.topic = topic;
    msg.payload = payload;
    msg.qos = qos;
    msg.retained = false;
    msg.timestamp = std::chrono::system_clock::now();
    
    // In real implementation, would send to IoT platform
    return true;
}

bool IoTPlatformIntegration::Subscribe(const std::string& topic, int /*qos*/) {
    if (connection_status_ != ConnectionStatus::CONNECTED) return false;
    
    subscribed_topics_.push_back(topic);
    return true;
}

bool IoTPlatformIntegration::Unsubscribe(const std::string& topic) {
    auto it = std::find(subscribed_topics_.begin(), subscribed_topics_.end(), topic);
    if (it != subscribed_topics_.end()) {
        subscribed_topics_.erase(it);
        return true;
    }
    return false;
}

std::vector<std::string> IoTPlatformIntegration::GetSubscribedTopics() const {
    return subscribed_topics_;
}

bool IoTPlatformIntegration::UpdateShadow(const std::map<std::string, std::string>& reported_state) {
    if (connection_status_ != ConnectionStatus::CONNECTED) return false;
    
    shadow_.reported_state = reported_state;
    shadow_.version++;
    shadow_.last_updated = std::chrono::system_clock::now();
    
    return true;
}

DeviceShadow IoTPlatformIntegration::GetShadow() const {
    return shadow_;
}

bool IoTPlatformIntegration::DeleteShadow() {
    shadow_.reported_state.clear();
    shadow_.desired_state.clear();
    shadow_.version = 0;
    return true;
}

bool IoTPlatformIntegration::SendTelemetry(const std::vector<TelemetryData>& data) {
    if (connection_status_ != ConnectionStatus::CONNECTED) return false;
    
    std::ostringstream payload;
    payload << "{";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) payload << ",";
        payload << "\"" << data[i].name << "\":" << data[i].value;
    }
    payload << "}";
    
    return Publish("telemetry", payload.str(), 0);
}

bool IoTPlatformIntegration::SendTelemetry(const std::string& name, const std::string& value) {
    TelemetryData data;
    data.name = name;
    data.value = value;
    data.timestamp = std::chrono::system_clock::now();
    return SendTelemetry({data});
}

std::string IoTPlatformIntegration::GenerateConnectionCode() const {
    std::ostringstream code;
    
    switch (platform_) {
        case IoTPlatform::AWS_IOT:
            code << "#include <WiFiClientSecure.h>\n";
            code << "#include <PubSubClient.h>\n\n";
            code << "WiFiClientSecure espClient;\n";
            code << "PubSubClient client(espClient);\n\n";
            code << "void setupAWS() {\n";
            code << "    espClient.setCACert(root_ca);\n";
            code << "    espClient.setCertificate(device_cert);\n";
            code << "    espClient.setPrivateKey(private_key);\n";
            code << "    client.setServer(\"" << credentials_.endpoint << "\", 8883);\n";
            code << "}\n";
            break;
            
        case IoTPlatform::AZURE_IOT:
            code << "#include <AzureIoTHub.h>\n\n";
            code << "void setupAzure() {\n";
            code << "    IoTHubClient_LL_CreateFromConnectionString(\n";
            code << "        \"" << credentials_.endpoint << "\",\n";
            code << "        MQTT_Protocol);\n";
            code << "}\n";
            break;
            
        case IoTPlatform::GOOGLE_CLOUD_IOT:
            code << "#include <CloudIoTCore.h>\n\n";
            code << "CloudIoTCoreDevice device;\n\n";
            code << "void setupGCP() {\n";
            code << "    device.connect();\n";
            code << "}\n";
            break;
            
        default:
            code << "// Custom IoT platform setup\n";
            break;
    }
    
    return code.str();
}

std::string IoTPlatformIntegration::GeneratePublishCode(const std::string& topic) const {
    std::ostringstream code;
    
    code << "void publishMessage(const char* payload) {\n";
    code << "    client.publish(\"" << topic << "\", payload);\n";
    code << "}\n";
    
    return code.str();
}

std::string IoTPlatformIntegration::GenerateSubscribeCode(const std::string& topic) const {
    std::ostringstream code;
    
    code << "void subscribeToTopic() {\n";
    code << "    client.subscribe(\"" << topic << "\");\n";
    code << "}\n";
    
    return code.str();
}

std::string IoTPlatformIntegration::GenerateTelemetryCode() const {
    std::ostringstream code;
    
    code << "void sendTelemetry(float temperature, float humidity) {\n";
    code << "    char payload[256];\n";
    code << "    snprintf(payload, sizeof(payload),\n";
    code << "        \"{\\\"temperature\\\":%.2f,\\\"humidity\\\":%.2f}\",\n";
    code << "        temperature, humidity);\n";
    code << "    publishMessage(payload);\n";
    code << "}\n";
    
    return code.str();
}

std::string IoTPlatformIntegration::GetPlatformName() const {
    switch (platform_) {
        case IoTPlatform::AWS_IOT: return "AWS IoT Core";
        case IoTPlatform::AZURE_IOT: return "Azure IoT Hub";
        case IoTPlatform::GOOGLE_CLOUD_IOT: return "Google Cloud IoT Core";
        case IoTPlatform::CUSTOM: return "Custom IoT Platform";
        default: return "Unknown";
    }
}

std::string IoTPlatformIntegration::GetEndpointUrl() const {
    return credentials_.endpoint;
}

std::vector<std::string> IoTPlatformIntegration::GetRequiredLibraries() const {
    std::vector<std::string> libs;
    
    switch (platform_) {
        case IoTPlatform::AWS_IOT:
            libs = {"WiFiClientSecure", "PubSubClient", "ArduinoJson"};
            break;
        case IoTPlatform::AZURE_IOT:
            libs = {"AzureIoTHub", "AzureIoTUtility", "AzureIoTProtocol_MQTT"};
            break;
        case IoTPlatform::GOOGLE_CLOUD_IOT:
            libs = {"CloudIoTCore", "MQTT", "jwt"};
            break;
        default:
            break;
    }
    
    return libs;
}

void IoTPlatformIntegration::NotifyConnectionChange(ConnectionStatus status) {
    if (connection_callback_) {
        connection_callback_(status);
    }
}

void IoTPlatformIntegration::NotifyMessage(const IoTMessage& message) {
    if (message_callback_) {
        message_callback_(message);
    }
}

// ============================================================================
// AWSIoTIntegration Implementation
// ============================================================================

AWSIoTIntegration::AWSIoTIntegration() : region_("us-east-1") {
    SetPlatform(IoTPlatform::AWS_IOT);
}

void AWSIoTIntegration::SetRegion(const std::string& region) {
    region_ = region;
}

void AWSIoTIntegration::SetThingName(const std::string& thing_name) {
    thing_name_ = thing_name;
}

bool AWSIoTIntegration::CreateThing(const std::string& thing_name, const std::string& /*thing_type*/) {
    thing_name_ = thing_name;
    return true;
}

bool AWSIoTIntegration::RegisterCertificate(const std::string& certificate) {
    credentials_.certificate = certificate;
    return true;
}

std::string AWSIoTIntegration::GenerateAWSCode() const {
    std::ostringstream code;
    
    code << "// AWS IoT Core Configuration\n";
    code << "#include <WiFiClientSecure.h>\n";
    code << "#include <PubSubClient.h>\n\n";
    code << "const char* AWS_IOT_ENDPOINT = \"" << credentials_.endpoint << "\";\n";
    code << "const char* THING_NAME = \"" << thing_name_ << "\";\n\n";
    code << GenerateConnectionCode();
    
    return code.str();
}

// ============================================================================
// AzureIoTIntegration Implementation
// ============================================================================

AzureIoTIntegration::AzureIoTIntegration() {
    SetPlatform(IoTPlatform::AZURE_IOT);
}

void AzureIoTIntegration::SetConnectionString(const std::string& connection_string) {
    connection_string_ = connection_string;
}

void AzureIoTIntegration::SetDeviceId(const std::string& device_id) {
    device_id_ = device_id;
}

bool AzureIoTIntegration::UpdateDeviceTwin(const std::map<std::string, std::string>& properties) {
    return UpdateShadow(properties);
}

bool AzureIoTIntegration::InvokeDirectMethod(const std::string& /*method_name*/, 
                                               const std::string& /*payload*/) {
    // Simulate direct method invocation
    return true;
}

std::string AzureIoTIntegration::GenerateAzureCode() const {
    std::ostringstream code;
    
    code << "// Azure IoT Hub Configuration\n";
    code << "#include <AzureIoTHub.h>\n\n";
    code << "const char* DEVICE_ID = \"" << device_id_ << "\";\n";
    code << "const char* IOT_HUB_NAME = \"" << iot_hub_name_ << "\";\n\n";
    code << GenerateConnectionCode();
    
    return code.str();
}

// ============================================================================
// GoogleCloudIoTIntegration Implementation
// ============================================================================

GoogleCloudIoTIntegration::GoogleCloudIoTIntegration() : region_("us-central1") {
    SetPlatform(IoTPlatform::GOOGLE_CLOUD_IOT);
}

void GoogleCloudIoTIntegration::SetProjectId(const std::string& project_id) {
    project_id_ = project_id;
}

void GoogleCloudIoTIntegration::SetRegistryId(const std::string& registry_id) {
    registry_id_ = registry_id;
}

void GoogleCloudIoTIntegration::SetDeviceId(const std::string& device_id) {
    device_id_ = device_id;
}

void GoogleCloudIoTIntegration::SetRegion(const std::string& region) {
    region_ = region;
}

bool GoogleCloudIoTIntegration::ConfigureDevice(const std::map<std::string, std::string>& /*config*/) {
    // Simulate device configuration
    return true;
}

std::string GoogleCloudIoTIntegration::GenerateGCPCode() const {
    std::ostringstream code;
    
    code << "// Google Cloud IoT Core Configuration\n";
    code << "#include <CloudIoTCore.h>\n\n";
    code << "const char* PROJECT_ID = \"" << project_id_ << "\";\n";
    code << "const char* REGION = \"" << region_ << "\";\n";
    code << "const char* REGISTRY_ID = \"" << registry_id_ << "\";\n";
    code << "const char* DEVICE_ID = \"" << device_id_ << "\";\n\n";
    code << GenerateConnectionCode();
    
    return code.str();
}

} // namespace platform
} // namespace esp32_ide
