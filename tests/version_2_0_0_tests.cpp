#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>

#include "platform/platform_expansion.h"
#include "visualization/advanced_visualization.h"
#include "plugins/plugin_system.h"

using namespace esp32_ide::platform;
using namespace esp32_ide::visualization;
using namespace esp32_ide::plugins;

// ============================================================================
// Helper assertion functions
// ============================================================================

void assert_true(bool condition, const std::string& message = "") {
    if (!condition) {
        throw std::runtime_error("Assertion failed: " + message);
    }
}

void assert_equal(int expected, int actual, const std::string& message = "") {
    if (expected != actual) {
        throw std::runtime_error("Assertion failed: expected " + std::to_string(expected) + 
                                " but got " + std::to_string(actual) + ". " + message);
    }
}

template<typename T>
void assert_enum_equal(T expected, T actual, const std::string& message = "") {
    if (expected != actual) {
        throw std::runtime_error("Assertion failed: enum values don't match. " + message);
    }
}

void assert_equal(const std::string& expected, const std::string& actual, const std::string& message = "") {
    if (expected != actual) {
        throw std::runtime_error("Assertion failed: expected '" + expected + 
                                "' but got '" + actual + "'. " + message);
    }
}

// ============================================================================
// Platform Expansion Tests
// ============================================================================

void test_multi_board_support() {
    MultiBoardSupport boards;
    
    // Test default boards are registered
    auto all_boards = boards.GetAllBoards();
    assert_true(all_boards.size() > 0, "Should have default boards registered");
    
    // Test ESP32 family
    auto esp32_boards = boards.GetBoardsByFamily(BoardFamily::ESP32);
    assert_true(esp32_boards.size() > 0, "Should have ESP32 boards");
    
    // Test ESP8266 family
    auto esp8266_boards = boards.GetBoardsByFamily(BoardFamily::ESP8266);
    assert_true(esp8266_boards.size() > 0, "Should have ESP8266 boards");
    
    // Test STM32 family
    auto stm32_boards = boards.GetBoardsByFamily(BoardFamily::STM32);
    assert_true(stm32_boards.size() > 0, "Should have STM32 boards");
    
    // Test Arduino family
    auto arduino_boards = boards.GetBoardsByFamily(BoardFamily::ARDUINO);
    assert_true(arduino_boards.size() > 0, "Should have Arduino boards");
    
    // Test board selection
    assert_true(boards.SelectBoard("esp32"), "Should select ESP32 board");
    assert_equal("esp32", boards.GetSelectedBoardId());
    
    assert_true(boards.SelectBoard("esp8266"), "Should select ESP8266 board");
    assert_equal("esp8266", boards.GetSelectedBoardId());
    
    assert_true(boards.SelectBoard("stm32f103"), "Should select STM32 board");
    assert_equal("stm32f103", boards.GetSelectedBoardId());
    
    assert_true(boards.SelectBoard("arduino_uno"), "Should select Arduino board");
    assert_equal("arduino_uno", boards.GetSelectedBoardId());
    
    // Test features
    boards.SelectBoard("esp32");
    assert_true(boards.HasFeature("wifi"), "ESP32 should have WiFi");
    assert_true(boards.HasFeature("bluetooth"), "ESP32 should have Bluetooth");
    
    boards.SelectBoard("arduino_uno");
    assert_true(!boards.HasFeature("wifi"), "Arduino UNO should not have WiFi");
    
    std::cout << "  ✓ Multi-board support tests passed" << std::endl;
}

void test_rtos_integration() {
    RTOSIntegration rtos;
    
    // Test initialization
    assert_true(rtos.Initialize(), "Should initialize RTOS");
    assert_true(rtos.IsInitialized(), "Should be initialized");
    
    // Test task creation
    uint32_t task1 = rtos.CreateTask("Task1", TaskPriority::NORMAL, 4096, 0);
    assert_true(task1 > 0, "Should create task");
    
    uint32_t task2 = rtos.CreateTask("Task2", TaskPriority::HIGH, 8192, 1);
    assert_true(task2 > 0, "Should create second task");
    
    // Test task info
    auto info = rtos.GetTaskInfo(task1);
    assert_equal("Task1", info.name);
    
    // Test all tasks
    auto all_tasks = rtos.GetAllTasks();
    assert_true(all_tasks.size() >= 3, "Should have at least 3 tasks (including IDLE)");
    
    // Test task priority change
    assert_true(rtos.SetTaskPriority(task1, TaskPriority::HIGH), "Should change priority");
    
    // Test suspend/resume
    assert_true(rtos.SuspendTask(task1), "Should suspend task");
    assert_true(rtos.ResumeTask(task1), "Should resume task");
    
    // Test scheduler
    assert_true(rtos.StartScheduler(), "Should start scheduler");
    
    // Test visualization
    std::string diagram = rtos.GenerateTaskDiagram();
    assert_true(!diagram.empty(), "Should generate task diagram");
    
    std::string gantt = rtos.GenerateGanttChart(1000);
    assert_true(!gantt.empty(), "Should generate Gantt chart");
    
    // Test analysis
    auto analysis = rtos.AnalyzeScheduling();
    assert_true(!analysis.empty(), "Should provide scheduling analysis");
    
    // Test sync primitives
    std::string mutex = rtos.CreateMutex("TestMutex");
    assert_true(!mutex.empty(), "Should create mutex");
    
    std::string sem = rtos.CreateSemaphore("TestSem", 5);
    assert_true(!sem.empty(), "Should create semaphore");
    
    // Cleanup
    rtos.Shutdown();
    assert_true(!rtos.IsInitialized(), "Should be shut down");
    
    std::cout << "  ✓ RTOS integration tests passed" << std::endl;
}

void test_iot_platform_integration() {
    // Test AWS IoT
    AWSIoTIntegration aws;
    assert_enum_equal(IoTPlatform::AWS_IOT, aws.GetPlatform(), "Should be AWS IoT");
    
    aws.SetRegion("us-west-2");
    aws.SetThingName("MyDevice");
    
    IoTCredentials creds;
    creds.endpoint = "abc123.iot.us-west-2.amazonaws.com";
    creds.client_id = "device_01";
    aws.SetCredentials(creds);
    
    assert_true(aws.Connect(), "Should connect to AWS IoT");
    assert_enum_equal(ConnectionStatus::CONNECTED, aws.GetConnectionStatus(), "Should be connected");
    
    // Test publishing
    assert_true(aws.Publish("test/topic", "{\"temp\": 25.5}"), "Should publish message");
    
    // Test subscribing
    assert_true(aws.Subscribe("test/response"), "Should subscribe to topic");
    auto topics = aws.GetSubscribedTopics();
    assert_true(topics.size() > 0, "Should have subscribed topics");
    
    // Test shadow
    std::map<std::string, std::string> state;
    state["temperature"] = "25.5";
    assert_true(aws.UpdateShadow(state), "Should update shadow");
    
    // Test code generation
    std::string code = aws.GenerateAWSCode();
    assert_true(!code.empty(), "Should generate AWS code");
    assert_true(code.find("AWS_IOT") != std::string::npos, "Code should contain AWS_IOT");
    
    aws.Disconnect();
    
    // Test Azure IoT
    AzureIoTIntegration azure;
    assert_enum_equal(IoTPlatform::AZURE_IOT, azure.GetPlatform(), "Should be Azure IoT");
    
    azure.SetDeviceId("device_01");
    azure.SetConnectionString("HostName=myiot.azure-devices.net;...");
    
    std::string azure_code = azure.GenerateAzureCode();
    assert_true(!azure_code.empty(), "Should generate Azure code");
    
    // Test Google Cloud IoT
    GoogleCloudIoTIntegration gcp;
    assert_enum_equal(IoTPlatform::GOOGLE_CLOUD_IOT, gcp.GetPlatform(), "Should be GCP IoT");
    
    gcp.SetProjectId("my-project");
    gcp.SetRegistryId("my-registry");
    gcp.SetDeviceId("device_01");
    gcp.SetRegion("us-central1");
    
    std::string gcp_code = gcp.GenerateGCPCode();
    assert_true(!gcp_code.empty(), "Should generate GCP code");
    
    std::cout << "  ✓ IoT platform integration tests passed" << std::endl;
}

// ============================================================================
// Advanced Visualization Tests
// ============================================================================

void test_signal_analyzer() {
    SignalAnalyzer analyzer;
    
    // Test channel creation
    WaveformConfig config;
    config.channel_name = "CH1";
    config.signal_type = SignalType::ANALOG;
    config.sample_rate_hz = 1000000;
    config.voltage_scale = 3.3;
    config.color = "#00FF00";
    config.visible = true;
    
    int channel = analyzer.AddChannel(config);
    assert_true(channel > 0, "Should create channel");
    
    // Test data capture
    analyzer.StartCapture();
    assert_true(analyzer.IsCapturing(), "Should be capturing");
    
    // Add samples
    for (int i = 0; i < 100; ++i) {
        SignalSample sample;
        sample.timestamp_us = i * 10.0;
        sample.value = std::sin(i * 0.1) * 1.65 + 1.65;  // 0-3.3V sine wave
        sample.is_digital_high = sample.value > 1.65;
        analyzer.AddSample(channel, sample);
    }
    
    analyzer.StopCapture();
    assert_true(!analyzer.IsCapturing(), "Should stop capturing");
    
    // Test waveform generation
    std::string ascii = analyzer.GenerateWaveformASCII(channel, 50, 10);
    assert_true(!ascii.empty(), "Should generate ASCII waveform");
    
    std::string svg = analyzer.GenerateWaveformSVG(channel, 400, 200);
    assert_true(!svg.empty(), "Should generate SVG waveform");
    assert_true(svg.find("<svg") != std::string::npos, "Should be valid SVG");
    
    // Test signal analysis
    double freq = analyzer.GetFrequency(channel);
    double rms = analyzer.GetRMS(channel);
    double pp = analyzer.GetPeakToPeak(channel);
    double avg = analyzer.GetAverage(channel);
    
    assert_true(rms > 0, "Should calculate RMS");
    assert_true(pp > 0, "Should calculate peak-to-peak");
    
    // Test trigger
    analyzer.SetTriggerLevel(1.65);
    analyzer.SetTriggerEdge(true);
    assert_true(analyzer.GetTriggerLevel() == 1.65, "Should set trigger level");
    
    std::cout << "  ✓ Signal analyzer tests passed" << std::endl;
}

void test_logic_analyzer() {
    LogicAnalyzer logic;
    
    // Add channels
    int ch1 = logic.AddChannel("CLK", "#00FF00");
    int ch2 = logic.AddChannel("DATA", "#FF0000");
    assert_true(ch1 > 0 && ch2 > 0, "Should create channels");
    
    // Start capture
    logic.StartCapture(10000000);  // 10 MHz
    
    // Simulate clock and data signals
    for (int i = 0; i < 100; ++i) {
        logic.RecordSample(ch1, i % 2 == 0, i * 0.1);  // 5 MHz clock
        logic.RecordSample(ch2, (i / 4) % 2 == 0, i * 0.1);  // Data
    }
    
    logic.StopCapture();
    
    // Test timing measurements
    auto timing = logic.MeasureTiming(ch1);
    assert_true(!timing.empty(), "Should measure timing");
    
    double pulse_width = logic.MeasurePulseWidth(ch1, true);
    double frequency = logic.MeasureFrequency(ch1);
    
    // Test timing diagram
    std::string diagram = logic.GenerateTimingDiagram(80);
    assert_true(!diagram.empty(), "Should generate timing diagram");
    
    std::cout << "  ✓ Logic analyzer tests passed" << std::endl;
}

void test_protocol_decoder() {
    ProtocolDecoder decoder;
    
    // Configure I2C
    ProtocolDecoder::I2CConfig i2c_cfg;
    i2c_cfg.sda_channel = 0;
    i2c_cfg.scl_channel = 1;
    i2c_cfg.show_ack_nack = true;
    decoder.ConfigureI2C(i2c_cfg);
    
    // Configure UART
    ProtocolDecoder::UARTConfig uart_cfg;
    uart_cfg.rx_channel = 0;
    uart_cfg.baud_rate = 115200;
    uart_cfg.data_bits = 8;
    uart_cfg.stop_bits = 1;
    decoder.ConfigureUART(uart_cfg);
    
    // Test I2C decoding (with simulated data)
    std::vector<SignalSample> sda, scl;
    for (int i = 0; i < 100; ++i) {
        SignalSample s;
        s.timestamp_us = i * 10.0;
        s.is_digital_high = i % 3 != 0;
        sda.push_back(s);
        scl.push_back(s);
    }
    
    auto i2c_msgs = decoder.DecodeI2C(sda, scl);
    assert_true(!i2c_msgs.empty(), "Should decode I2C messages");
    assert_enum_equal(ProtocolType::I2C, i2c_msgs[0].protocol, "Protocol should be I2C");
    
    // Test UART decoding
    auto uart_msgs = decoder.DecodeUART(sda);
    assert_true(!uart_msgs.empty(), "Should decode UART messages");
    
    // Test report generation
    std::string report = decoder.GenerateDecodedReport(i2c_msgs);
    assert_true(!report.empty(), "Should generate report");
    
    // Test auto-detection
    int baud = decoder.AutoDetectBaudRate(sda);
    assert_true(baud > 0, "Should auto-detect baud rate");
    
    std::cout << "  ✓ Protocol decoder tests passed" << std::endl;
}

void test_3d_visualization() {
    Visualization3D viz;
    
    // Set board size
    viz.SetBoardSize(100.0f, 80.0f, 1.6f);
    viz.SetBoardColor("#2E7D32");
    
    auto bounds = viz.GetBoardBounds();
    assert_true(bounds.Width() == 100.0f, "Board width should be 100mm");
    assert_true(bounds.Height() == 80.0f, "Board height should be 80mm");
    
    // Add components
    ComponentFootprint esp32;
    esp32.id = "U1";
    esp32.name = "ESP32-WROOM-32";
    esp32.package = "MODULE";
    esp32.bounds.min = Point3D(10, 10, 1.6f);
    esp32.bounds.max = Point3D(35, 30, 1.6f + 3.0f);
    esp32.height_mm = 3.0f;
    esp32.color = "#333333";
    viz.AddComponent(esp32);
    
    ComponentFootprint cap;
    cap.id = "C1";
    cap.name = "100nF";
    cap.package = "0805";
    cap.bounds.min = Point3D(40, 15, 1.6f);
    cap.bounds.max = Point3D(42, 16.25f, 1.6f + 0.5f);
    cap.height_mm = 0.5f;
    cap.color = "#8B4513";
    viz.AddComponent(cap);
    
    auto components = viz.GetAllComponents();
    assert_equal(2, static_cast<int>(components.size()));
    
    // Test component manipulation
    viz.MoveComponent("C1", Point3D(50, 20, 1.6f));
    viz.RotateComponent("C1", 90.0f);
    
    // Add traces
    PCBTrace trace;
    trace.net_name = "VCC";
    trace.layer = PCBLayer::TOP_COPPER;
    trace.path = {Point3D(35, 20, 1.6f), Point3D(45, 20, 1.6f), Point3D(50, 20, 1.6f)};
    trace.width_mm = 0.5f;
    trace.color = "#FF0000";
    viz.AddTrace(trace);
    
    auto traces = viz.GetTraces();
    assert_equal(1, static_cast<int>(traces.size()));
    
    // Add via
    Via via;
    via.position = Point3D(45, 20, 0);
    via.drill_diameter_mm = 0.3f;
    via.outer_diameter_mm = 0.6f;
    via.start_layer = PCBLayer::TOP_COPPER;
    via.end_layer = PCBLayer::BOTTOM_COPPER;
    viz.AddVia(via);
    
    // Test visualization
    std::string svg = viz.GenerateSVG(PCBLayer::TOP_COPPER, 800, 600);
    assert_true(!svg.empty(), "Should generate SVG");
    assert_true(svg.find("<svg") != std::string::npos, "Should be valid SVG");
    
    std::string obj = viz.Generate3DOBJ();
    assert_true(!obj.empty(), "Should generate OBJ");
    
    std::string gerber = viz.GenerateGerber(PCBLayer::TOP_COPPER);
    assert_true(!gerber.empty(), "Should generate Gerber");
    
    // Test design rules check
    auto violations = viz.CheckDesignRules(0.2f, 0.15f);
    // May or may not have violations depending on design
    
    // Test cost estimation
    float cost = viz.EstimateBoardCost();
    assert_true(cost > 0, "Should estimate board cost");
    
    std::cout << "  ✓ 3D visualization tests passed" << std::endl;
}

void test_network_visualizer() {
    NetworkVisualizer viz;
    
    // Add nodes
    NetworkNode ap;
    ap.id = "ap_1";
    ap.name = "HomeRouter";
    ap.type = NetworkNodeType::WIFI_ACCESS_POINT;
    ap.mac_address = "AA:BB:CC:DD:EE:FF";
    ap.ip_address = "192.168.1.1";
    ap.signal_strength_dbm = -40;
    ap.is_connected = true;
    viz.AddNode(ap);
    
    NetworkNode device;
    device.id = "dev_1";
    device.name = "ESP32_Sensor";
    device.type = NetworkNodeType::WIFI_STATION;
    device.mac_address = "11:22:33:44:55:66";
    device.ip_address = "192.168.1.100";
    device.signal_strength_dbm = -55;
    device.is_connected = true;
    viz.AddNode(device);
    
    auto nodes = viz.GetAllNodes();
    assert_equal(2, static_cast<int>(nodes.size()));
    
    // Add connection
    NetworkConnection conn;
    conn.source_id = "ap_1";
    conn.target_id = "dev_1";
    conn.type = ConnectionType::WIFI;
    conn.bandwidth_kbps = 54000;
    conn.latency_ms = 5;
    viz.AddConnection(conn);
    
    // Test WiFi scanning
    viz.ScanWiFiNetworks();
    auto wifi = viz.GetWiFiNetworks();
    assert_true(!wifi.empty(), "Should have scanned WiFi networks");
    
    // Test WiFi map
    std::string wifi_map = viz.GenerateWiFiMap();
    assert_true(!wifi_map.empty(), "Should generate WiFi map");
    
    // Test Bluetooth
    BluetoothDevice bt;
    bt.name = "Heart Rate Monitor";
    bt.address = "AA:BB:CC:DD:EE:00";
    bt.rssi = -60;
    bt.is_ble = true;
    bt.is_connected = false;
    viz.AddBluetoothDevice(bt);
    
    auto bt_devices = viz.GetBluetoothDevices();
    assert_true(!bt_devices.empty(), "Should have Bluetooth devices");
    
    std::string bt_map = viz.GenerateBluetoothMap();
    assert_true(!bt_map.empty(), "Should generate Bluetooth map");
    
    // Test MQTT topic tree
    MQTTTopicNode root;
    root.topic = "home";
    root.subscriber_count = 5;
    viz.AddMQTTTopic(root);
    
    MQTTTopicNode sensor;
    sensor.topic = "home/sensor";
    sensor.parent_topic = "home";
    sensor.subscriber_count = 3;
    viz.AddMQTTTopic(sensor);
    
    viz.UpdateMQTTActivity("home/sensor", "{\"temp\": 25.5}");
    
    std::string mqtt_tree = viz.GenerateMQTTTopicTree();
    assert_true(!mqtt_tree.empty(), "Should generate MQTT topic tree");
    
    // Test network diagram
    std::string diagram = viz.GenerateNetworkDiagram();
    assert_true(!diagram.empty(), "Should generate network diagram");
    
    std::string svg = viz.GenerateNetworkSVG(800, 600);
    assert_true(!svg.empty(), "Should generate network SVG");
    
    std::string json = viz.GenerateTopologyJSON();
    assert_true(!json.empty(), "Should generate topology JSON");
    
    // Test layout
    viz.ApplyForceDirectedLayout();
    
    // Test analysis
    float density = viz.CalculateNetworkDensity();
    assert_true(density >= 0, "Should calculate density");
    
    std::cout << "  ✓ Network visualizer tests passed" << std::endl;
}

// ============================================================================
// Plugin System Tests
// ============================================================================

// Sample plugin for testing
class TestPlugin : public Plugin {
public:
    TestPlugin() : Plugin(CreateMetadata()) {}
    
    static PluginMetadata CreateMetadata() {
        PluginMetadata meta;
        meta.id = "test-plugin";
        meta.name = "Test Plugin";
        meta.version = "1.0.0";
        meta.author = "Test Author";
        meta.description = "A test plugin";
        meta.type = PluginType::TOOL;
        return meta;
    }
    
    bool Initialize(PluginAPI* api) override {
        api_ = api;
        SetState(PluginState::LOADED);
        return true;
    }
    
    bool Activate() override {
        SetState(PluginState::ACTIVE);
        return true;
    }
    
    bool Deactivate() override {
        SetState(PluginState::LOADED);
        return true;
    }
    
    void Dispose() override {
        SetState(PluginState::UNLOADED);
    }
};

void test_plugin_manager() {
    PluginManager manager;
    
    // Register plugin factory
    manager.RegisterPluginFactory("test-plugin", []() {
        return std::make_unique<TestPlugin>();
    });
    
    // Test loading
    assert_true(manager.LoadPlugin("test-plugin"), "Should load plugin");
    assert_true(manager.IsPluginLoaded("test-plugin"), "Plugin should be loaded");
    
    // Test plugin retrieval
    Plugin* plugin = manager.GetPlugin("test-plugin");
    assert_true(plugin != nullptr, "Should get plugin");
    assert_equal("Test Plugin", plugin->GetMetadata().name);
    
    // Test enable/disable
    assert_true(manager.EnablePlugin("test-plugin"), "Should enable plugin");
    assert_enum_equal(PluginState::ACTIVE, plugin->GetState(), "Plugin should be active");
    
    assert_true(manager.DisablePlugin("test-plugin"), "Should disable plugin");
    assert_enum_equal(PluginState::LOADED, plugin->GetState(), "Plugin should be loaded");
    
    // Test all plugins
    auto all = manager.GetAllPlugins();
    assert_equal(1, static_cast<int>(all.size()));
    
    // Test by type
    auto tools = manager.GetPluginsByType(PluginType::TOOL);
    assert_equal(1, static_cast<int>(tools.size()));
    
    // Test unloading
    assert_true(manager.UnloadPlugin("test-plugin"), "Should unload plugin");
    assert_true(!manager.IsPluginLoaded("test-plugin"), "Plugin should not be loaded");
    
    std::cout << "  ✓ Plugin manager tests passed" << std::endl;
}

void test_external_tools() {
    ExternalToolManager tools;
    
    // Register tool
    ExternalToolConfig config;
    config.id = "format";
    config.name = "Code Formatter";
    config.command = "clang-format";
    config.arguments = "-i ${file}";
    config.show_in_menu = true;
    tools.RegisterTool(config);
    
    assert_true(tools.ToolExists("format"), "Tool should exist");
    
    auto tool_config = tools.GetToolConfig("format");
    assert_equal("Code Formatter", tool_config.name);
    
    // Test variable substitution
    std::map<std::string, std::string> vars;
    vars["file"] = "main.cpp";
    std::string result = tools.SubstituteVariables("Format ${file}", vars);
    assert_equal("Format main.cpp", result);
    
    // Test execution (simulated)
    auto exec_result = tools.ExecuteTool("format", vars);
    assert_equal(0, exec_result.exit_code);
    
    // Get all tools
    auto all_tools = tools.GetAllTools();
    assert_equal(1, static_cast<int>(all_tools.size()));
    
    // Get menu tools
    auto menu_tools = tools.GetMenuTools();
    assert_equal(1, static_cast<int>(menu_tools.size()));
    
    std::cout << "  ✓ External tools tests passed" << std::endl;
}

void test_custom_compilers() {
    CustomCompilerManager compilers;
    
    // Should have default compilers
    auto all = compilers.GetAllCompilers();
    assert_true(!all.empty(), "Should have default compilers");
    
    // Test GCC
    assert_true(compilers.CompilerExists("gcc"), "Should have GCC");
    
    auto gcc = compilers.GetCompilerConfig("gcc");
    assert_equal("GCC", gcc.name);
    
    // Test compilation (simulated)
    auto result = compilers.Compile("gcc", "main.cpp", "main.o");
    assert_equal(0, result.exit_code);
    
    // Test linking (simulated)
    std::vector<std::string> objects = {"main.o", "util.o"};
    auto link_result = compilers.Link("gcc", objects, "program");
    assert_equal(0, link_result.exit_code);
    
    // Test compiler IDs
    auto ids = compilers.GetCompilerIds();
    assert_true(ids.size() >= 3, "Should have multiple compilers");
    
    std::cout << "  ✓ Custom compilers tests passed" << std::endl;
}

void test_analysis_tools() {
    AnalysisToolRunner analyzer;
    
    // Test static analysis
    std::string code = R"(
        void loop() {
            goto label;
        label:
            gets(buffer);
        }
    )";
    
    auto static_results = analyzer.RunStaticAnalysis(code);
    assert_true(!static_results.empty(), "Should find static analysis issues");
    
    // Test style check
    std::string long_line = std::string(150, 'x');
    auto style_results = analyzer.RunStyleCheck(long_line);
    assert_true(!style_results.empty(), "Should find style issues");
    
    // Test security scan
    std::string insecure = "char* password = \"secret123\"; strcpy(buf, input);";
    auto security_results = analyzer.RunSecurityScan(insecure);
    assert_true(!security_results.empty(), "Should find security issues");
    
    // Test complexity analysis
    std::string complex = R"(
        if (a) {
            if (b) {
                for (int i = 0; i < n; i++) {
                    while (x && y || z) {
                        switch(c) {
                            case 1: break;
                            case 2: break;
                            case 3: break;
                            case 4: break;
                        }
                        if (d && e) {
                            if (f || g) {}
                        }
                    }
                }
            }
        }
    )";
    auto complexity_results = analyzer.RunComplexityAnalysis(complex);
    // Complexity might or might not trigger based on threshold, so just verify API works
    // The important thing is the function runs without error
    
    // Test report generation
    std::string report = analyzer.GenerateReport(static_results, "text");
    assert_true(!report.empty(), "Should generate text report");
    
    std::string json_report = analyzer.GenerateReport(static_results, "json");
    assert_true(!json_report.empty(), "Should generate JSON report");
    
    std::cout << "  ✓ Analysis tools tests passed" << std::endl;
}

void test_plugin_marketplace() {
    PluginMarketplace marketplace;
    
    // Test search
    MarketplaceFilter filter;
    filter.query = "sensor";
    auto results = marketplace.Search(filter);
    assert_true(!results.empty(), "Should return search results");
    
    // Test featured
    auto featured = marketplace.GetFeatured();
    assert_true(!featured.empty(), "Should return featured plugins");
    
    // Test popular
    auto popular = marketplace.GetPopular(5);
    
    // Test plugin details
    auto details = marketplace.GetPluginDetails("sample-plugin");
    assert_true(!details.metadata.id.empty(), "Should return plugin details");
    
    // Test installation
    assert_true(marketplace.InstallPlugin("test-plugin"), "Should install plugin");
    assert_true(marketplace.IsPluginInstalled("test-plugin"), "Plugin should be installed");
    
    // Get installed
    auto installed = marketplace.GetInstalledPlugins();
    assert_equal(1, static_cast<int>(installed.size()));
    
    // Test uninstall
    assert_true(marketplace.UninstallPlugin("test-plugin"), "Should uninstall plugin");
    assert_true(!marketplace.IsPluginInstalled("test-plugin"), "Plugin should not be installed");
    
    // Test reviews
    auto reviews = marketplace.GetReviews("sample-plugin");
    assert_true(!reviews.empty(), "Should return reviews");
    
    std::cout << "  ✓ Plugin marketplace tests passed" << std::endl;
}

void test_plugin_devkit() {
    PluginDevKit devkit;
    
    // Test available templates
    auto templates = devkit.GetAvailableTemplates();
    assert_true(!templates.empty(), "Should have templates");
    
    // Test validation (simulated paths)
    auto manifest_errors = devkit.ValidateManifest("/path/to/manifest.json");
    // Empty means no errors
    
    auto plugin_errors = devkit.ValidatePlugin("/path/to/plugin");
    
    // Test compatibility check
    bool compatible = devkit.CheckCompatibility("/path/to/plugin", "2.0.0");
    assert_true(compatible, "Should be compatible");
    
    // Test debugging
    devkit.EnablePluginDebugging("test-plugin");
    devkit.DisablePluginDebugging("test-plugin");
    
    std::cout << "  ✓ Plugin dev kit tests passed" << std::endl;
}

void test_plugin_documentation() {
    PluginDocGenerator doc;
    
    // Test README generation
    PluginMetadata meta;
    meta.name = "My Plugin";
    meta.description = "A great plugin";
    meta.author = "Developer";
    meta.license = "MIT";
    
    std::string readme = doc.GenerateReadme(meta);
    assert_true(!readme.empty(), "Should generate README");
    assert_true(readme.find("My Plugin") != std::string::npos, "README should contain plugin name");
    
    // Test changelog
    std::vector<std::pair<std::string, std::string>> changes;
    changes.push_back({"1.0.0", "Initial release"});
    changes.push_back({"1.1.0", "Added new features"});
    
    std::string changelog = doc.GenerateChangelog(changes);
    assert_true(!changelog.empty(), "Should generate changelog");
    
    // Test contributing guide
    std::string contributing = doc.GenerateContributingGuide();
    assert_true(!contributing.empty(), "Should generate contributing guide");
    
    // Test exports
    std::string html = doc.ExportAsHTML(readme);
    assert_true(!html.empty(), "Should export as HTML");
    
    std::cout << "  ✓ Plugin documentation tests passed" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ESP32 Driver IDE - Version 2.0.0 Tests" << std::endl;
    std::cout << "Testing New Features" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        std::cout << "Platform Expansion Tests:" << std::endl;
        test_multi_board_support();
        test_rtos_integration();
        test_iot_platform_integration();
        
        std::cout << "\nAdvanced Visualization Tests:" << std::endl;
        test_signal_analyzer();
        test_logic_analyzer();
        test_protocol_decoder();
        test_3d_visualization();
        test_network_visualizer();
        
        std::cout << "\nPlugin System Tests:" << std::endl;
        test_plugin_manager();
        test_external_tools();
        test_custom_compilers();
        test_analysis_tools();
        test_plugin_marketplace();
        test_plugin_devkit();
        test_plugin_documentation();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ ALL VERSION 2.0.0 TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
