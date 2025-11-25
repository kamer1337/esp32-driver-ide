#ifndef ADVANCED_VISUALIZATION_H
#define ADVANCED_VISUALIZATION_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>
#include <cstdint>

namespace esp32_ide {
namespace visualization {

// ============================================================================
// Signal Analyzer (Real-time waveform, Logic analyzer, Protocol decoder)
// ============================================================================

/**
 * @brief Signal type enumeration
 */
enum class SignalType {
    ANALOG,
    DIGITAL,
    PWM,
    I2C,
    SPI,
    UART,
    CUSTOM
};

/**
 * @brief Protocol type for decoding
 */
enum class ProtocolType {
    I2C,
    SPI,
    UART,
    ONE_WIRE,
    CAN,
    MODBUS,
    CUSTOM
};

/**
 * @brief Signal sample data point
 */
struct SignalSample {
    double timestamp_us;
    double value;
    bool is_digital_high;
};

/**
 * @brief Waveform configuration
 */
struct WaveformConfig {
    std::string channel_name;
    SignalType signal_type;
    double sample_rate_hz;
    double voltage_scale;
    double time_scale_us;
    std::string color;
    bool visible;
};

/**
 * @brief Decoded protocol message
 */
struct DecodedMessage {
    ProtocolType protocol;
    double start_time_us;
    double end_time_us;
    std::string raw_data;
    std::string decoded_data;
    std::string description;
    bool is_valid;
    std::vector<std::string> errors;
};

/**
 * @brief Signal analyzer for real-time waveform display
 */
class SignalAnalyzer {
public:
    using SampleCallback = std::function<void(const SignalSample&)>;
    
    SignalAnalyzer();
    ~SignalAnalyzer() = default;
    
    // Channel management
    int AddChannel(const WaveformConfig& config);
    bool RemoveChannel(int channel_id);
    void ClearAllChannels();
    WaveformConfig GetChannelConfig(int channel_id) const;
    std::vector<int> GetChannelIds() const;
    
    // Data acquisition
    void StartCapture();
    void StopCapture();
    bool IsCapturing() const { return capturing_; }
    void AddSample(int channel_id, const SignalSample& sample);
    std::vector<SignalSample> GetSamples(int channel_id, double start_time, double end_time) const;
    void ClearSamples(int channel_id);
    
    // Waveform display
    std::string GenerateWaveformASCII(int channel_id, int width = 80, int height = 10) const;
    std::string GenerateWaveformSVG(int channel_id, int width = 800, int height = 200) const;
    
    // Signal analysis
    /** @brief Get signal frequency in Hz. Returns 0 if insufficient data. */
    double GetFrequency(int channel_id) const;
    /** @brief Get duty cycle as percentage (0-100). Returns 0 if no data. */
    double GetDutyCycle(int channel_id) const;
    /** @brief Get RMS voltage value. Returns 0 if no samples. */
    double GetRMS(int channel_id) const;
    /** @brief Get peak-to-peak voltage. Returns 0 if no samples. */
    double GetPeakToPeak(int channel_id) const;
    /** @brief Get average voltage value. Returns 0 if no samples. */
    double GetAverage(int channel_id) const;
    
    // Trigger settings
    void SetTriggerLevel(double level);
    void SetTriggerEdge(bool rising);
    void SetTriggerChannel(int channel_id);
    double GetTriggerLevel() const { return trigger_level_; }
    
    // Callbacks
    void SetSampleCallback(int channel_id, SampleCallback callback);
    
private:
    bool capturing_;
    double trigger_level_;
    bool trigger_rising_;
    int trigger_channel_;
    std::map<int, WaveformConfig> channels_;
    std::map<int, std::vector<SignalSample>> samples_;
    std::map<int, SampleCallback> callbacks_;
    int next_channel_id_;
};

/**
 * @brief Logic analyzer for digital signal analysis
 */
class LogicAnalyzer {
public:
    struct LogicChannel {
        int channel_id;
        std::string name;
        std::string color;
        bool inverted;
        std::vector<SignalSample> samples;
    };
    
    struct TimingMeasurement {
        std::string name;
        double value_us;
        std::string description;
    };
    
    LogicAnalyzer();
    ~LogicAnalyzer() = default;
    
    // Channel management
    int AddChannel(const std::string& name, const std::string& color = "#00FF00");
    bool RemoveChannel(int channel_id);
    void SetChannelInverted(int channel_id, bool inverted);
    
    // Data capture
    void StartCapture(double sample_rate_hz);
    void StopCapture();
    void RecordSample(int channel_id, bool value, double timestamp_us);
    std::vector<SignalSample> GetChannelData(int channel_id) const;
    
    // Timing analysis
    std::vector<TimingMeasurement> MeasureTiming(int channel_id) const;
    double MeasurePulseWidth(int channel_id, bool high_pulse) const;
    double MeasureFrequency(int channel_id) const;
    
    // Pattern detection
    std::vector<double> FindPattern(int channel_id, const std::vector<bool>& pattern) const;
    
    // Visualization
    std::string GenerateTimingDiagram(int width = 100) const;
    
private:
    std::map<int, LogicChannel> channels_;
    double sample_rate_hz_;
    bool capturing_;
    int next_channel_id_;
};

/**
 * @brief Protocol decoder for serial protocols
 */
class ProtocolDecoder {
public:
    struct I2CConfig {
        int sda_channel;
        int scl_channel;
        bool show_ack_nack;
    };
    
    struct SPIConfig {
        int mosi_channel;
        int miso_channel;
        int sck_channel;
        int cs_channel;
        bool cpol;  // Clock polarity
        bool cpha;  // Clock phase
        int bits_per_word;
    };
    
    struct UARTConfig {
        int rx_channel;
        int tx_channel;
        int baud_rate;
        int data_bits;
        int stop_bits;
        bool parity;
        bool parity_odd;
    };
    
    ProtocolDecoder();
    ~ProtocolDecoder() = default;
    
    // Protocol configuration
    void ConfigureI2C(const I2CConfig& config);
    void ConfigureSPI(const SPIConfig& config);
    void ConfigureUART(const UARTConfig& config);
    
    // Decoding
    std::vector<DecodedMessage> DecodeI2C(const std::vector<SignalSample>& sda,
                                           const std::vector<SignalSample>& scl) const;
    std::vector<DecodedMessage> DecodeSPI(const std::vector<SignalSample>& mosi,
                                           const std::vector<SignalSample>& miso,
                                           const std::vector<SignalSample>& sck,
                                           const std::vector<SignalSample>& cs) const;
    std::vector<DecodedMessage> DecodeUART(const std::vector<SignalSample>& data) const;
    
    // Auto-detection
    ProtocolType AutoDetectProtocol(const std::vector<SignalSample>& data) const;
    int AutoDetectBaudRate(const std::vector<SignalSample>& data) const;
    
    // Report generation
    std::string GenerateDecodedReport(const std::vector<DecodedMessage>& messages) const;
    
private:
    I2CConfig i2c_config_;
    SPIConfig spi_config_;
    UARTConfig uart_config_;
    
    bool DetectStartCondition(const std::vector<SignalSample>& sda,
                              const std::vector<SignalSample>& scl,
                              size_t index) const;
    bool DetectStopCondition(const std::vector<SignalSample>& sda,
                             const std::vector<SignalSample>& scl,
                             size_t index) const;
};

// ============================================================================
// 3D Visualization (PCB layout, Component placement, Wire routing)
// ============================================================================

/**
 * @brief 3D point structure
 */
struct Point3D {
    float x, y, z;
    
    Point3D() : x(0), y(0), z(0) {}
    Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

/**
 * @brief 3D bounding box
 */
struct BoundingBox3D {
    Point3D min;
    Point3D max;
    
    float Width() const { return max.x - min.x; }
    float Height() const { return max.y - min.y; }
    float Depth() const { return max.z - min.z; }
};

/**
 * @brief PCB layer type
 */
enum class PCBLayer {
    TOP_COPPER,
    BOTTOM_COPPER,
    TOP_SILKSCREEN,
    BOTTOM_SILKSCREEN,
    TOP_SOLDERMASK,
    BOTTOM_SOLDERMASK,
    SUBSTRATE
};

/**
 * @brief Component footprint
 */
struct ComponentFootprint {
    std::string id;
    std::string name;
    std::string package;
    BoundingBox3D bounds;
    std::vector<Point3D> pins;
    float height_mm;
    std::string color;
};

/**
 * @brief PCB trace
 */
struct PCBTrace {
    std::string net_name;
    PCBLayer layer;
    std::vector<Point3D> path;
    float width_mm;
    std::string color;
};

/**
 * @brief Via structure
 */
struct Via {
    Point3D position;
    float drill_diameter_mm;
    float outer_diameter_mm;
    PCBLayer start_layer;
    PCBLayer end_layer;
};

/**
 * @brief 3D visualization engine
 */
class Visualization3D {
public:
    Visualization3D();
    ~Visualization3D() = default;
    
    // PCB board setup
    void SetBoardSize(float width_mm, float height_mm, float thickness_mm);
    void SetBoardColor(const std::string& color);
    BoundingBox3D GetBoardBounds() const;
    
    // Component placement
    void AddComponent(const ComponentFootprint& component);
    void RemoveComponent(const std::string& id);
    void MoveComponent(const std::string& id, const Point3D& new_position);
    void RotateComponent(const std::string& id, float angle_degrees);
    std::vector<ComponentFootprint> GetAllComponents() const;
    ComponentFootprint* GetComponent(const std::string& id);
    
    // Trace routing
    void AddTrace(const PCBTrace& trace);
    void RemoveTrace(const std::string& net_name);
    std::vector<PCBTrace> GetTraces() const;
    
    // Via management
    void AddVia(const Via& via);
    std::vector<Via> GetVias() const;
    
    // Auto-placement
    void AutoPlaceComponents();
    std::vector<std::string> ValidatePlacement() const;
    float CalculateRoutingDensity() const;
    
    // Wire routing
    std::vector<Point3D> AutoRouteNet(const std::string& net_name,
                                       const std::vector<std::string>& pin_ids);
    float CalculateWireLength(const PCBTrace& trace) const;
    
    // Visualization
    std::string GenerateSVG(PCBLayer layer, int width = 800, int height = 600) const;
    std::string Generate3DOBJ() const;  // Wavefront OBJ format
    std::string GenerateGerber(PCBLayer layer) const;
    
    // View control
    void SetViewAngle(float pitch, float yaw, float roll);
    void SetZoom(float zoom_level);
    Point3D GetCameraPosition() const;
    
    // Analysis
    std::vector<std::string> CheckDesignRules(float min_trace_width_mm,
                                               float min_clearance_mm) const;
    float EstimateBoardCost() const;
    
private:
    float board_width_mm_;
    float board_height_mm_;
    float board_thickness_mm_;
    std::string board_color_;
    
    std::map<std::string, ComponentFootprint> components_;
    std::vector<PCBTrace> traces_;
    std::vector<Via> vias_;
    
    float view_pitch_;
    float view_yaw_;
    float view_roll_;
    float zoom_level_;
    
    bool CheckCollision(const ComponentFootprint& a, const ComponentFootprint& b) const;
    Point3D Project2D(const Point3D& point3d) const;
};

// ============================================================================
// Network Visualization (WiFi map, Bluetooth discovery, MQTT topic tree)
// ============================================================================

/**
 * @brief Network node type
 */
enum class NetworkNodeType {
    WIFI_ACCESS_POINT,
    WIFI_STATION,
    BLUETOOTH_DEVICE,
    BLE_DEVICE,
    MQTT_BROKER,
    MQTT_CLIENT,
    MQTT_TOPIC,
    GATEWAY,
    SENSOR,
    ACTUATOR
};

/**
 * @brief Network connection type
 */
enum class ConnectionType {
    WIFI,
    BLUETOOTH_CLASSIC,
    BLE,
    MQTT,
    TCP,
    UDP,
    HTTP,
    WEBSOCKET
};

/**
 * @brief Network node structure
 */
struct NetworkNode {
    std::string id;
    std::string name;
    NetworkNodeType type;
    std::string mac_address;
    std::string ip_address;
    int signal_strength_dbm;
    bool is_connected;
    Point3D position;  // For visualization
    std::map<std::string, std::string> properties;
};

/**
 * @brief Network connection
 */
struct NetworkConnection {
    std::string source_id;
    std::string target_id;
    ConnectionType type;
    int bandwidth_kbps;
    int latency_ms;
    float packet_loss_percent;
    bool is_encrypted;
};

/**
 * @brief MQTT topic node
 */
struct MQTTTopicNode {
    std::string topic;
    std::string parent_topic;
    std::vector<std::string> child_topics;
    int subscriber_count;
    int message_rate_per_minute;
    std::string last_message;
    std::chrono::system_clock::time_point last_activity;
};

/**
 * @brief WiFi network information
 */
struct WiFiNetwork {
    std::string ssid;
    std::string bssid;
    int channel;
    int signal_strength_dbm;
    std::string security_type;
    bool is_connected;
    std::vector<std::string> connected_devices;
};

/**
 * @brief Bluetooth device information
 */
struct BluetoothDevice {
    std::string name;
    std::string address;
    int rssi;
    bool is_ble;
    bool is_paired;
    bool is_connected;
    std::vector<std::string> services;
};

/**
 * @brief Network visualizer
 */
class NetworkVisualizer {
public:
    using NodeCallback = std::function<void(const NetworkNode&)>;
    
    NetworkVisualizer();
    ~NetworkVisualizer() = default;
    
    // Node management
    void AddNode(const NetworkNode& node);
    void RemoveNode(const std::string& node_id);
    void UpdateNode(const NetworkNode& node);
    NetworkNode* GetNode(const std::string& node_id);
    std::vector<NetworkNode> GetAllNodes() const;
    std::vector<NetworkNode> GetNodesByType(NetworkNodeType type) const;
    
    // Connection management
    void AddConnection(const NetworkConnection& connection);
    void RemoveConnection(const std::string& source_id, const std::string& target_id);
    std::vector<NetworkConnection> GetConnections() const;
    std::vector<NetworkConnection> GetNodeConnections(const std::string& node_id) const;
    
    // WiFi network mapping
    void ScanWiFiNetworks();
    void AddWiFiNetwork(const WiFiNetwork& network);
    std::vector<WiFiNetwork> GetWiFiNetworks() const;
    std::string GenerateWiFiMap() const;
    
    // Bluetooth discovery
    void StartBluetoothScan();
    void StopBluetoothScan();
    void AddBluetoothDevice(const BluetoothDevice& device);
    std::vector<BluetoothDevice> GetBluetoothDevices() const;
    std::string GenerateBluetoothMap() const;
    
    // MQTT topic tree
    void AddMQTTTopic(const MQTTTopicNode& topic);
    void RemoveMQTTTopic(const std::string& topic);
    MQTTTopicNode* GetMQTTTopic(const std::string& topic);
    std::vector<MQTTTopicNode> GetMQTTTopics() const;
    std::string GenerateMQTTTopicTree() const;
    void UpdateMQTTActivity(const std::string& topic, const std::string& message);
    
    // Visualization
    std::string GenerateNetworkDiagram() const;
    std::string GenerateNetworkSVG(int width = 800, int height = 600) const;
    std::string GenerateTopologyJSON() const;
    
    // Layout algorithms
    void ApplyForceDirectedLayout();
    void ApplyHierarchicalLayout();
    void ApplyCircularLayout();
    
    // Network analysis
    std::vector<std::string> FindPath(const std::string& from_id, const std::string& to_id) const;
    float CalculateNetworkDensity() const;
    std::vector<std::string> FindBottlenecks() const;
    
    // Callbacks
    void SetNodeDiscoveredCallback(NodeCallback callback) { node_callback_ = callback; }
    
private:
    std::map<std::string, NetworkNode> nodes_;
    std::vector<NetworkConnection> connections_;
    std::vector<WiFiNetwork> wifi_networks_;
    std::vector<BluetoothDevice> bluetooth_devices_;
    std::map<std::string, MQTTTopicNode> mqtt_topics_;
    bool bluetooth_scanning_;
    NodeCallback node_callback_;
    
    void NotifyNodeDiscovered(const NetworkNode& node);
    Point3D CalculateNodePosition(const std::string& node_id) const;
};

} // namespace visualization
} // namespace esp32_ide

#endif // ADVANCED_VISUALIZATION_H
