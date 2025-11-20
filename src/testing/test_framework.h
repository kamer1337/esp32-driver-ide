#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <map>

namespace esp32_ide {
namespace testing {

/**
 * @brief Test result status
 */
enum class TestStatus {
    PASSED,
    FAILED,
    SKIPPED,
    ERROR
};

/**
 * @brief Test case result
 */
struct TestResult {
    std::string test_name;
    TestStatus status;
    std::string message;
    long long duration_ms;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Code coverage information
 */
struct CoverageInfo {
    int total_lines;
    int covered_lines;
    int total_functions;
    int covered_functions;
    std::map<std::string, std::vector<int>> uncovered_lines;  // file -> line numbers
    
    float GetLineCoverage() const {
        return total_lines > 0 ? (float)covered_lines / total_lines * 100.0f : 0.0f;
    }
    
    float GetFunctionCoverage() const {
        return total_functions > 0 ? (float)covered_functions / total_functions * 100.0f : 0.0f;
    }
};

/**
 * @brief Test suite for unit testing
 */
class TestSuite {
public:
    using TestFunction = std::function<void()>;
    using SetupFunction = std::function<void()>;
    using TeardownFunction = std::function<void()>;
    
    TestSuite(const std::string& name);
    ~TestSuite() = default;
    
    // Test registration
    void AddTest(const std::string& name, TestFunction test_func);
    void AddSetup(SetupFunction setup_func);
    void AddTeardown(TeardownFunction teardown_func);
    
    // Test execution
    std::vector<TestResult> Run();
    TestResult RunSingleTest(const std::string& name);
    
    // Test management
    void Skip(const std::string& test_name);
    void ClearTests();
    
    // Getters
    const std::string& GetName() const { return name_; }
    int GetTestCount() const { return tests_.size(); }
    std::vector<std::string> GetTestNames() const;
    
private:
    std::string name_;
    std::vector<std::pair<std::string, TestFunction>> tests_;
    SetupFunction setup_;
    TeardownFunction teardown_;
    std::vector<std::string> skipped_tests_;
};

/**
 * @brief Test runner that executes test suites
 */
class TestRunner {
public:
    TestRunner();
    ~TestRunner() = default;
    
    // Suite management
    void AddSuite(TestSuite* suite);
    void RemoveSuite(const std::string& suite_name);
    void ClearSuites();
    
    // Test execution
    std::vector<TestResult> RunAll();
    std::vector<TestResult> RunSuite(const std::string& suite_name);
    std::vector<TestResult> RunTest(const std::string& suite_name, const std::string& test_name);
    
    // Results
    std::vector<TestResult> GetResults() const { return all_results_; }
    int GetPassedCount() const;
    int GetFailedCount() const;
    int GetSkippedCount() const;
    std::string GenerateReport() const;
    
    // Configuration
    void SetVerbose(bool verbose) { verbose_ = verbose; }
    void SetStopOnFailure(bool stop) { stop_on_failure_ = stop; }
    
private:
    std::vector<TestSuite*> suites_;
    std::vector<TestResult> all_results_;
    bool verbose_;
    bool stop_on_failure_;
};

/**
 * @brief Coverage analyzer for code coverage analysis
 */
class CoverageAnalyzer {
public:
    CoverageAnalyzer();
    ~CoverageAnalyzer() = default;
    
    // Coverage tracking
    void StartTracking();
    void StopTracking();
    void RecordLineCoverage(const std::string& file, int line);
    void RecordFunctionCoverage(const std::string& file, const std::string& function);
    
    // Coverage analysis
    CoverageInfo GetCoverageInfo() const;
    std::string GenerateCoverageReport() const;
    std::map<std::string, float> GetFileCoverage() const;
    
    // Configuration
    void AddSourceFile(const std::string& file, int total_lines);
    void AddFunction(const std::string& file, const std::string& function);
    void Reset();
    
private:
    bool is_tracking_;
    std::map<std::string, std::vector<int>> covered_lines_;      // file -> covered line numbers
    std::map<std::string, int> total_lines_per_file_;            // file -> total lines
    std::map<std::string, std::vector<std::string>> covered_functions_;  // file -> covered functions
    std::map<std::string, std::vector<std::string>> total_functions_;    // file -> all functions
};

/**
 * @brief Mock framework for creating test mocks
 */
class MockFramework {
public:
    struct MockCall {
        std::string function_name;
        std::vector<std::string> arguments;
        std::string return_value;
        std::chrono::system_clock::time_point timestamp;
    };
    
    MockFramework();
    ~MockFramework() = default;
    
    // Mock registration
    void RegisterMock(const std::string& function_name);
    void SetReturnValue(const std::string& function_name, const std::string& value);
    void SetExpectedCalls(const std::string& function_name, int count);
    
    // Mock verification
    void RecordCall(const std::string& function_name, const std::vector<std::string>& args);
    int GetCallCount(const std::string& function_name) const;
    std::vector<MockCall> GetCalls(const std::string& function_name) const;
    bool VerifyExpectedCalls() const;
    std::string GetReturnValue(const std::string& function_name) const;
    
    // Mock management
    void Reset();
    void ResetFunction(const std::string& function_name);
    
private:
    std::map<std::string, std::vector<MockCall>> recorded_calls_;
    std::map<std::string, std::string> return_values_;
    std::map<std::string, int> expected_calls_;
};

/**
 * @brief Hardware-in-loop testing simulator
 */
class HardwareInLoopSimulator {
public:
    enum class HardwareState {
        IDLE,
        RUNNING,
        PAUSED,
        FAILED
    };
    
    struct TestScenario {
        std::string name;
        std::string description;
        std::vector<std::string> steps;
        std::map<std::string, std::string> expected_outputs;
    };
    
    struct HardwareTestResult {
        std::string scenario_name;
        TestStatus status;
        std::string message;
        std::map<std::string, std::string> actual_outputs;
        long long duration_ms;
    };
    
    HardwareInLoopSimulator();
    ~HardwareInLoopSimulator() = default;
    
    // Scenario management
    void AddScenario(const TestScenario& scenario);
    void RemoveScenario(const std::string& name);
    std::vector<std::string> GetScenarios() const;
    
    // Test execution
    HardwareTestResult RunScenario(const std::string& name);
    std::vector<HardwareTestResult> RunAllScenarios();
    
    // Hardware simulation
    void SimulateGPIOInput(int pin, bool state);
    void SimulateSensorData(int pin, int value);
    void SimulateSerialInput(const std::string& data);
    
    // State management
    HardwareState GetState() const { return state_; }
    void Pause();
    void Resume();
    void Stop();
    
private:
    HardwareState state_;
    std::map<std::string, TestScenario> scenarios_;
    std::map<int, bool> gpio_states_;
    std::map<int, int> sensor_values_;
    std::string serial_buffer_;
};

/**
 * @brief Assertion helper functions
 */
class Assert {
public:
    static void IsTrue(bool condition, const std::string& message = "");
    static void IsFalse(bool condition, const std::string& message = "");
    static void AreEqual(int expected, int actual, const std::string& message = "");
    static void AreEqual(const std::string& expected, const std::string& actual, const std::string& message = "");
    static void AreNotEqual(int a, int b, const std::string& message = "");
    static void IsNull(void* ptr, const std::string& message = "");
    static void IsNotNull(void* ptr, const std::string& message = "");
    static void Throws(std::function<void()> func, const std::string& message = "");
    static void DoesNotThrow(std::function<void()> func, const std::string& message = "");
    
private:
    static void Fail(const std::string& message);
};

} // namespace testing
} // namespace esp32_ide

#endif // TEST_FRAMEWORK_H
