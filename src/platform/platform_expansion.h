#ifndef PLATFORM_EXPANSION_H
#define PLATFORM_EXPANSION_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace esp32_ide {
namespace platform {

// ============================================================================
// Multi-Board Support (ESP8266, STM32, Arduino)
// ============================================================================

/**
 * @brief Board family enumeration
 */
enum class BoardFamily {
    ESP32,
    ESP8266,
    STM32,
    ARDUINO,
    CUSTOM
};

/**
 * @brief Board configuration structure
 */
struct BoardConfig {
    std::string id;
    std::string name;
    BoardFamily family;
    std::string architecture;     // e.g., "xtensa", "arm", "avr"
    std::string chip;             // e.g., "ESP8266", "STM32F103", "ATmega328P"
    size_t flash_size_kb;
    size_t ram_size_kb;
    size_t eeprom_size_bytes;
    int cpu_frequency_mhz;
    std::vector<int> available_pins;
    std::vector<std::string> supported_features;  // e.g., "wifi", "bluetooth", "usb"
    std::map<std::string, std::string> properties;
};

/**
 * @brief Multi-board support manager
 */
class MultiBoardSupport {
public:
    MultiBoardSupport();
    ~MultiBoardSupport() = default;
    
    // Board registration
    void RegisterBoard(const BoardConfig& config);
    void UnregisterBoard(const std::string& board_id);
    
    // Board selection
    bool SelectBoard(const std::string& board_id);
    const BoardConfig* GetSelectedBoard() const;
    std::string GetSelectedBoardId() const;
    
    // Board queries
    std::vector<BoardConfig> GetAllBoards() const;
    std::vector<BoardConfig> GetBoardsByFamily(BoardFamily family) const;
    const BoardConfig* GetBoardById(const std::string& board_id) const;
    
    // Board capabilities
    bool HasFeature(const std::string& feature) const;
    std::vector<std::string> GetSupportedFeatures() const;
    
    // Code generation helpers
    std::string GetBoardDefine() const;
    std::string GetCompilerFlags() const;
    std::string GetLinkerFlags() const;
    
private:
    std::map<std::string, BoardConfig> boards_;
    std::string selected_board_id_;
    
    void InitializeDefaultBoards();
};

// ============================================================================
// RTOS Integration (FreeRTOS support, Task visualization, Scheduler analysis)
// ============================================================================

/**
 * @brief Task state enumeration
 */
enum class TaskState {
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    DELETED
};

/**
 * @brief Task priority levels
 */
enum class TaskPriority {
    IDLE = 0,
    LOW = 1,
    BELOW_NORMAL = 2,
    NORMAL = 3,
    ABOVE_NORMAL = 4,
    HIGH = 5,
    REALTIME = 6
};

/**
 * @brief Task information structure
 */
struct TaskInfo {
    std::string name;
    uint32_t task_id;
    TaskState state;
    TaskPriority priority;
    size_t stack_size;
    size_t stack_high_water_mark;
    int cpu_core;  // -1 for any core
    float cpu_usage_percent;
    long long total_runtime_us;
    std::chrono::system_clock::time_point created_at;
};

/**
 * @brief Scheduler statistics
 */
struct SchedulerStats {
    int total_tasks;
    int running_tasks;
    int blocked_tasks;
    int suspended_tasks;
    long long context_switches;
    float total_cpu_usage_percent;
    std::chrono::system_clock::time_point uptime_start;
    std::map<std::string, float> task_cpu_usage;
};

/**
 * @brief Semaphore/Mutex information
 */
struct SyncPrimitiveInfo {
    std::string name;
    std::string type;  // "mutex", "semaphore", "queue"
    std::string owner_task;
    int count;
    int max_count;
    std::vector<std::string> waiting_tasks;
};

/**
 * @brief RTOS integration manager
 */
class RTOSIntegration {
public:
    using TaskCallback = std::function<void(const TaskInfo&)>;
    
    RTOSIntegration();
    ~RTOSIntegration() = default;
    
    // Initialization
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    
    // Task management
    uint32_t CreateTask(const std::string& name, TaskPriority priority, 
                        size_t stack_size = 4096, int cpu_core = -1);
    bool DeleteTask(uint32_t task_id);
    bool SuspendTask(uint32_t task_id);
    bool ResumeTask(uint32_t task_id);
    bool SetTaskPriority(uint32_t task_id, TaskPriority priority);
    
    // Task queries
    TaskInfo GetTaskInfo(uint32_t task_id) const;
    std::vector<TaskInfo> GetAllTasks() const;
    std::vector<TaskInfo> GetTasksByState(TaskState state) const;
    TaskInfo GetCurrentTask() const;
    
    // Scheduler control
    bool StartScheduler();
    bool StopScheduler();
    bool SuspendAll();
    bool ResumeAll();
    SchedulerStats GetSchedulerStats() const;
    
    // Task visualization
    std::string GenerateTaskDiagram() const;
    std::string GenerateGanttChart(int duration_ms) const;
    std::string GenerateCPUUsageChart() const;
    
    // Scheduler analysis
    std::vector<std::string> AnalyzeScheduling() const;
    std::vector<std::string> DetectPriorityInversion() const;
    std::vector<std::string> DetectDeadlocks() const;
    float CalculateCPUUtilization() const;
    
    // Synchronization primitives
    std::string CreateMutex(const std::string& name);
    std::string CreateSemaphore(const std::string& name, int max_count);
    std::string CreateQueue(const std::string& name, int queue_length, size_t item_size);
    bool DeleteSyncPrimitive(const std::string& id);
    std::vector<SyncPrimitiveInfo> GetAllSyncPrimitives() const;
    
    // Callbacks
    void SetTaskStateChangeCallback(TaskCallback callback);
    
private:
    bool initialized_;
    uint32_t next_task_id_;
    std::map<uint32_t, TaskInfo> tasks_;
    std::map<std::string, SyncPrimitiveInfo> sync_primitives_;
    TaskCallback task_callback_;
    SchedulerStats stats_;
    bool scheduler_running_;
    
    void UpdateTaskStats();
    void NotifyTaskChange(const TaskInfo& task);
};

// ============================================================================
// IoT Platform Integration (AWS IoT, Azure IoT, Google Cloud IoT)
// ============================================================================

/**
 * @brief IoT platform type
 */
enum class IoTPlatform {
    AWS_IOT,
    AZURE_IOT,
    GOOGLE_CLOUD_IOT,
    CUSTOM
};

/**
 * @brief Connection status
 */
enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING,
    ERROR
};

/**
 * @brief IoT credentials structure
 * 
 * @warning Security Note: This structure contains sensitive authentication data.
 * - Never log or print credential values
 * - Clear credentials from memory when no longer needed
 * - Use secure storage mechanisms in production
 * - Consider using hardware security modules (HSM) for private keys
 */
struct IoTCredentials {
    std::string client_id;
    std::string endpoint;
    std::string certificate;
    std::string private_key;
    std::string root_ca;
    std::string username;
    std::string password;
    std::map<std::string, std::string> extra_params;
};

/**
 * @brief Message structure for IoT communication
 */
struct IoTMessage {
    std::string topic;
    std::string payload;
    int qos;
    bool retained;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Device shadow/twin structure
 */
struct DeviceShadow {
    std::string device_id;
    std::map<std::string, std::string> reported_state;
    std::map<std::string, std::string> desired_state;
    long long version;
    std::chrono::system_clock::time_point last_updated;
};

/**
 * @brief Telemetry data point
 */
struct TelemetryData {
    std::string name;
    std::string value;
    std::string unit;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief IoT platform integration base class
 */
class IoTPlatformIntegration {
public:
    using MessageCallback = std::function<void(const IoTMessage&)>;
    using ConnectionCallback = std::function<void(ConnectionStatus)>;
    
    IoTPlatformIntegration();
    virtual ~IoTPlatformIntegration() = default;
    
    // Configuration
    virtual void SetCredentials(const IoTCredentials& credentials);
    virtual void SetPlatform(IoTPlatform platform);
    IoTPlatform GetPlatform() const { return platform_; }
    
    // Connection management
    virtual bool Connect();
    virtual bool Disconnect();
    virtual bool Reconnect();
    ConnectionStatus GetConnectionStatus() const { return connection_status_; }
    
    // Messaging
    virtual bool Publish(const std::string& topic, const std::string& payload, int qos = 0);
    virtual bool Subscribe(const std::string& topic, int qos = 0);
    virtual bool Unsubscribe(const std::string& topic);
    std::vector<std::string> GetSubscribedTopics() const;
    
    // Device shadow/twin
    virtual bool UpdateShadow(const std::map<std::string, std::string>& reported_state);
    virtual DeviceShadow GetShadow() const;
    virtual bool DeleteShadow();
    
    // Telemetry
    virtual bool SendTelemetry(const std::vector<TelemetryData>& data);
    virtual bool SendTelemetry(const std::string& name, const std::string& value);
    
    // Callbacks
    void SetMessageCallback(MessageCallback callback) { message_callback_ = callback; }
    void SetConnectionCallback(ConnectionCallback callback) { connection_callback_ = callback; }
    
    // Code generation
    std::string GenerateConnectionCode() const;
    std::string GeneratePublishCode(const std::string& topic) const;
    std::string GenerateSubscribeCode(const std::string& topic) const;
    std::string GenerateTelemetryCode() const;
    
    // Platform-specific helpers
    std::string GetPlatformName() const;
    std::string GetEndpointUrl() const;
    std::vector<std::string> GetRequiredLibraries() const;
    
protected:
    IoTPlatform platform_;
    IoTCredentials credentials_;
    ConnectionStatus connection_status_;
    std::vector<std::string> subscribed_topics_;
    DeviceShadow shadow_;
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    
    void NotifyConnectionChange(ConnectionStatus status);
    void NotifyMessage(const IoTMessage& message);
};

/**
 * @brief AWS IoT specific integration
 */
class AWSIoTIntegration : public IoTPlatformIntegration {
public:
    AWSIoTIntegration();
    
    // AWS-specific methods
    void SetRegion(const std::string& region);
    void SetThingName(const std::string& thing_name);
    bool CreateThing(const std::string& thing_name, const std::string& thing_type = "");
    bool RegisterCertificate(const std::string& certificate);
    
    std::string GenerateAWSCode() const;
    
private:
    std::string region_;
    std::string thing_name_;
};

/**
 * @brief Azure IoT specific integration
 */
class AzureIoTIntegration : public IoTPlatformIntegration {
public:
    AzureIoTIntegration();
    
    // Azure-specific methods
    void SetConnectionString(const std::string& connection_string);
    void SetDeviceId(const std::string& device_id);
    bool UpdateDeviceTwin(const std::map<std::string, std::string>& properties);
    bool InvokeDirectMethod(const std::string& method_name, const std::string& payload);
    
    std::string GenerateAzureCode() const;
    
private:
    std::string connection_string_;
    std::string device_id_;
    std::string iot_hub_name_;
};

/**
 * @brief Google Cloud IoT specific integration
 */
class GoogleCloudIoTIntegration : public IoTPlatformIntegration {
public:
    GoogleCloudIoTIntegration();
    
    // GCP-specific methods
    void SetProjectId(const std::string& project_id);
    void SetRegistryId(const std::string& registry_id);
    void SetDeviceId(const std::string& device_id);
    void SetRegion(const std::string& region);
    bool ConfigureDevice(const std::map<std::string, std::string>& config);
    
    std::string GenerateGCPCode() const;
    
private:
    std::string project_id_;
    std::string registry_id_;
    std::string device_id_;
    std::string region_;
};

} // namespace platform
} // namespace esp32_ide

#endif // PLATFORM_EXPANSION_H
