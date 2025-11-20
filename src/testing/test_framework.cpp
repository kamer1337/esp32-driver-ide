#include "testing/test_framework.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>

namespace esp32_ide {
namespace testing {

// ============================================================================
// TestSuite Implementation
// ============================================================================

TestSuite::TestSuite(const std::string& name) : name_(name) {}

void TestSuite::AddTest(const std::string& name, TestFunction test_func) {
    tests_.push_back({name, test_func});
}

void TestSuite::AddSetup(SetupFunction setup_func) {
    setup_ = setup_func;
}

void TestSuite::AddTeardown(TeardownFunction teardown_func) {
    teardown_ = teardown_func;
}

std::vector<TestResult> TestSuite::Run() {
    std::vector<TestResult> results;
    
    for (const auto& test : tests_) {
        results.push_back(RunSingleTest(test.first));
    }
    
    return results;
}

TestResult TestSuite::RunSingleTest(const std::string& name) {
    TestResult result;
    result.test_name = name;
    result.timestamp = std::chrono::system_clock::now();
    
    // Check if test is skipped
    if (std::find(skipped_tests_.begin(), skipped_tests_.end(), name) != skipped_tests_.end()) {
        result.status = TestStatus::SKIPPED;
        result.message = "Test skipped";
        result.duration_ms = 0;
        return result;
    }
    
    // Find the test
    auto it = std::find_if(tests_.begin(), tests_.end(),
                          [&name](const auto& t) { return t.first == name; });
    
    if (it == tests_.end()) {
        result.status = TestStatus::ERROR;
        result.message = "Test not found";
        result.duration_ms = 0;
        return result;
    }
    
    // Run setup
    if (setup_) {
        try {
            setup_();
        } catch (const std::exception& e) {
            result.status = TestStatus::ERROR;
            result.message = std::string("Setup failed: ") + e.what();
            result.duration_ms = 0;
            return result;
        }
    }
    
    // Run test
    auto start = std::chrono::high_resolution_clock::now();
    try {
        it->second();  // Execute test function
        result.status = TestStatus::PASSED;
        result.message = "Test passed";
    } catch (const std::exception& e) {
        result.status = TestStatus::FAILED;
        result.message = e.what();
    } catch (...) {
        result.status = TestStatus::ERROR;
        result.message = "Unknown error occurred";
    }
    auto end = std::chrono::high_resolution_clock::now();
    result.duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Run teardown
    if (teardown_) {
        try {
            teardown_();
        } catch (...) {
            // Teardown errors are logged but don't affect test result
        }
    }
    
    return result;
}

void TestSuite::Skip(const std::string& test_name) {
    skipped_tests_.push_back(test_name);
}

void TestSuite::ClearTests() {
    tests_.clear();
    skipped_tests_.clear();
}

std::vector<std::string> TestSuite::GetTestNames() const {
    std::vector<std::string> names;
    for (const auto& test : tests_) {
        names.push_back(test.first);
    }
    return names;
}

// ============================================================================
// TestRunner Implementation
// ============================================================================

TestRunner::TestRunner() : verbose_(false), stop_on_failure_(false) {}

void TestRunner::AddSuite(TestSuite* suite) {
    suites_.push_back(suite);
}

void TestRunner::RemoveSuite(const std::string& suite_name) {
    suites_.erase(std::remove_if(suites_.begin(), suites_.end(),
                                 [&](TestSuite* s) { return s->GetName() == suite_name; }),
                  suites_.end());
}

void TestRunner::ClearSuites() {
    suites_.clear();
    all_results_.clear();
}

std::vector<TestResult> TestRunner::RunAll() {
    all_results_.clear();
    
    for (auto* suite : suites_) {
        if (verbose_) {
            std::cout << "Running suite: " << suite->GetName() << std::endl;
        }
        
        auto results = suite->Run();
        all_results_.insert(all_results_.end(), results.begin(), results.end());
        
        if (stop_on_failure_) {
            for (const auto& result : results) {
                if (result.status == TestStatus::FAILED || result.status == TestStatus::ERROR) {
                    return all_results_;
                }
            }
        }
    }
    
    return all_results_;
}

std::vector<TestResult> TestRunner::RunSuite(const std::string& suite_name) {
    for (auto* suite : suites_) {
        if (suite->GetName() == suite_name) {
            return suite->Run();
        }
    }
    return {};
}

std::vector<TestResult> TestRunner::RunTest(const std::string& suite_name, const std::string& test_name) {
    for (auto* suite : suites_) {
        if (suite->GetName() == suite_name) {
            return {suite->RunSingleTest(test_name)};
        }
    }
    return {};
}

int TestRunner::GetPassedCount() const {
    return std::count_if(all_results_.begin(), all_results_.end(),
                        [](const TestResult& r) { return r.status == TestStatus::PASSED; });
}

int TestRunner::GetFailedCount() const {
    return std::count_if(all_results_.begin(), all_results_.end(),
                        [](const TestResult& r) { return r.status == TestStatus::FAILED; });
}

int TestRunner::GetSkippedCount() const {
    return std::count_if(all_results_.begin(), all_results_.end(),
                        [](const TestResult& r) { return r.status == TestStatus::SKIPPED; });
}

std::string TestRunner::GenerateReport() const {
    std::ostringstream report;
    report << "====================================\n";
    report << "Test Execution Report\n";
    report << "====================================\n\n";
    
    int passed = GetPassedCount();
    int failed = GetFailedCount();
    int skipped = GetSkippedCount();
    int total = all_results_.size();
    
    report << "Total Tests: " << total << "\n";
    report << "✓ Passed: " << passed << "\n";
    report << "✗ Failed: " << failed << "\n";
    report << "⊘ Skipped: " << skipped << "\n\n";
    
    if (failed > 0) {
        report << "Failed Tests:\n";
        report << "-------------\n";
        for (const auto& result : all_results_) {
            if (result.status == TestStatus::FAILED || result.status == TestStatus::ERROR) {
                report << "  " << result.test_name << "\n";
                report << "    Message: " << result.message << "\n";
                report << "    Duration: " << result.duration_ms << " ms\n\n";
            }
        }
    }
    
    long long total_duration = 0;
    for (const auto& result : all_results_) {
        total_duration += result.duration_ms;
    }
    report << "Total Duration: " << total_duration << " ms\n";
    
    return report.str();
}

// ============================================================================
// CoverageAnalyzer Implementation
// ============================================================================

CoverageAnalyzer::CoverageAnalyzer() : is_tracking_(false) {}

void CoverageAnalyzer::StartTracking() {
    is_tracking_ = true;
}

void CoverageAnalyzer::StopTracking() {
    is_tracking_ = false;
}

void CoverageAnalyzer::RecordLineCoverage(const std::string& file, int line) {
    if (!is_tracking_) return;
    
    auto& lines = covered_lines_[file];
    if (std::find(lines.begin(), lines.end(), line) == lines.end()) {
        lines.push_back(line);
    }
}

void CoverageAnalyzer::RecordFunctionCoverage(const std::string& file, const std::string& function) {
    if (!is_tracking_) return;
    
    auto& functions = covered_functions_[file];
    if (std::find(functions.begin(), functions.end(), function) == functions.end()) {
        functions.push_back(function);
    }
}

CoverageInfo CoverageAnalyzer::GetCoverageInfo() const {
    CoverageInfo info;
    info.total_lines = 0;
    info.covered_lines = 0;
    info.total_functions = 0;
    info.covered_functions = 0;
    
    for (const auto& pair : total_lines_per_file_) {
        info.total_lines += pair.second;
        
        auto it = covered_lines_.find(pair.first);
        if (it != covered_lines_.end()) {
            info.covered_lines += it->second.size();
        }
    }
    
    for (const auto& pair : total_functions_) {
        info.total_functions += pair.second.size();
        
        auto it = covered_functions_.find(pair.first);
        if (it != covered_functions_.end()) {
            info.covered_functions += it->second.size();
        }
    }
    
    return info;
}

std::string CoverageAnalyzer::GenerateCoverageReport() const {
    auto info = GetCoverageInfo();
    
    std::ostringstream report;
    report << "====================================\n";
    report << "Code Coverage Report\n";
    report << "====================================\n\n";
    
    report << "Line Coverage: " << info.GetLineCoverage() << "% ";
    report << "(" << info.covered_lines << "/" << info.total_lines << ")\n";
    
    report << "Function Coverage: " << info.GetFunctionCoverage() << "% ";
    report << "(" << info.covered_functions << "/" << info.total_functions << ")\n\n";
    
    report << "Per-File Coverage:\n";
    report << "------------------\n";
    
    for (const auto& pair : total_lines_per_file_) {
        const std::string& file = pair.first;
        int total = pair.second;
        int covered = 0;
        
        auto it = covered_lines_.find(file);
        if (it != covered_lines_.end()) {
            covered = it->second.size();
        }
        
        float percentage = total > 0 ? (float)covered / total * 100.0f : 0.0f;
        report << "  " << file << ": " << percentage << "% ";
        report << "(" << covered << "/" << total << ")\n";
    }
    
    return report.str();
}

std::map<std::string, float> CoverageAnalyzer::GetFileCoverage() const {
    std::map<std::string, float> coverage;
    
    for (const auto& pair : total_lines_per_file_) {
        const std::string& file = pair.first;
        int total = pair.second;
        int covered = 0;
        
        auto it = covered_lines_.find(file);
        if (it != covered_lines_.end()) {
            covered = it->second.size();
        }
        
        coverage[file] = total > 0 ? (float)covered / total * 100.0f : 0.0f;
    }
    
    return coverage;
}

void CoverageAnalyzer::AddSourceFile(const std::string& file, int total_lines) {
    total_lines_per_file_[file] = total_lines;
}

void CoverageAnalyzer::AddFunction(const std::string& file, const std::string& function) {
    total_functions_[file].push_back(function);
}

void CoverageAnalyzer::Reset() {
    covered_lines_.clear();
    covered_functions_.clear();
    is_tracking_ = false;
}

// ============================================================================
// MockFramework Implementation
// ============================================================================

MockFramework::MockFramework() {}

void MockFramework::RegisterMock(const std::string& function_name) {
    recorded_calls_[function_name] = {};
    expected_calls_[function_name] = -1;  // -1 means no expectation
}

void MockFramework::SetReturnValue(const std::string& function_name, const std::string& value) {
    return_values_[function_name] = value;
}

void MockFramework::SetExpectedCalls(const std::string& function_name, int count) {
    expected_calls_[function_name] = count;
}

void MockFramework::RecordCall(const std::string& function_name, const std::vector<std::string>& args) {
    MockCall call;
    call.function_name = function_name;
    call.arguments = args;
    call.timestamp = std::chrono::system_clock::now();
    
    auto it = return_values_.find(function_name);
    if (it != return_values_.end()) {
        call.return_value = it->second;
    }
    
    recorded_calls_[function_name].push_back(call);
}

int MockFramework::GetCallCount(const std::string& function_name) const {
    auto it = recorded_calls_.find(function_name);
    if (it != recorded_calls_.end()) {
        return it->second.size();
    }
    return 0;
}

std::vector<MockFramework::MockCall> MockFramework::GetCalls(const std::string& function_name) const {
    auto it = recorded_calls_.find(function_name);
    if (it != recorded_calls_.end()) {
        return it->second;
    }
    return {};
}

bool MockFramework::VerifyExpectedCalls() const {
    for (const auto& pair : expected_calls_) {
        if (pair.second == -1) continue;  // No expectation set
        
        int actual_calls = GetCallCount(pair.first);
        if (actual_calls != pair.second) {
            return false;
        }
    }
    return true;
}

std::string MockFramework::GetReturnValue(const std::string& function_name) const {
    auto it = return_values_.find(function_name);
    if (it != return_values_.end()) {
        return it->second;
    }
    return "";
}

void MockFramework::Reset() {
    recorded_calls_.clear();
    return_values_.clear();
    expected_calls_.clear();
}

void MockFramework::ResetFunction(const std::string& function_name) {
    recorded_calls_[function_name].clear();
    expected_calls_[function_name] = -1;
}

// ============================================================================
// HardwareInLoopSimulator Implementation
// ============================================================================

HardwareInLoopSimulator::HardwareInLoopSimulator() : state_(HardwareState::IDLE) {}

void HardwareInLoopSimulator::AddScenario(const TestScenario& scenario) {
    scenarios_[scenario.name] = scenario;
}

void HardwareInLoopSimulator::RemoveScenario(const std::string& name) {
    scenarios_.erase(name);
}

std::vector<std::string> HardwareInLoopSimulator::GetScenarios() const {
    std::vector<std::string> names;
    for (const auto& pair : scenarios_) {
        names.push_back(pair.first);
    }
    return names;
}

HardwareInLoopSimulator::HardwareTestResult HardwareInLoopSimulator::RunScenario(const std::string& name) {
    HardwareTestResult result;
    result.scenario_name = name;
    
    auto it = scenarios_.find(name);
    if (it == scenarios_.end()) {
        result.status = TestStatus::ERROR;
        result.message = "Scenario not found";
        result.duration_ms = 0;
        return result;
    }
    
    const auto& scenario = it->second;
    state_ = HardwareState::RUNNING;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        // Execute scenario steps (simulated)
        for (const auto& step : scenario.steps) {
            if (state_ == HardwareState::PAUSED) {
                result.status = TestStatus::SKIPPED;
                result.message = "Scenario paused";
                break;
            }
            
            // Simulate step execution
            // In real implementation, this would interact with hardware
        }
        
        // Verify expected outputs
        bool all_matched = true;
        for (const auto& expected : scenario.expected_outputs) {
            // Simulate checking actual vs expected
            result.actual_outputs[expected.first] = expected.second;  // Simulated match
        }
        
        if (all_matched && state_ == HardwareState::RUNNING) {
            result.status = TestStatus::PASSED;
            result.message = "All checks passed";
        } else if (state_ != HardwareState::RUNNING) {
            result.status = TestStatus::SKIPPED;
        } else {
            result.status = TestStatus::FAILED;
            result.message = "Output verification failed";
        }
        
    } catch (const std::exception& e) {
        result.status = TestStatus::ERROR;
        result.message = e.what();
        state_ = HardwareState::FAILED;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    state_ = HardwareState::IDLE;
    return result;
}

std::vector<HardwareInLoopSimulator::HardwareTestResult> HardwareInLoopSimulator::RunAllScenarios() {
    std::vector<HardwareTestResult> results;
    
    for (const auto& pair : scenarios_) {
        results.push_back(RunScenario(pair.first));
    }
    
    return results;
}

void HardwareInLoopSimulator::SimulateGPIOInput(int pin, bool state) {
    gpio_states_[pin] = state;
}

void HardwareInLoopSimulator::SimulateSensorData(int pin, int value) {
    sensor_values_[pin] = value;
}

void HardwareInLoopSimulator::SimulateSerialInput(const std::string& data) {
    serial_buffer_ += data;
}

void HardwareInLoopSimulator::Pause() {
    if (state_ == HardwareState::RUNNING) {
        state_ = HardwareState::PAUSED;
    }
}

void HardwareInLoopSimulator::Resume() {
    if (state_ == HardwareState::PAUSED) {
        state_ = HardwareState::RUNNING;
    }
}

void HardwareInLoopSimulator::Stop() {
    state_ = HardwareState::IDLE;
}

// ============================================================================
// Assert Implementation
// ============================================================================

void Assert::IsTrue(bool condition, const std::string& message) {
    if (!condition) {
        Fail(message.empty() ? "Expected true but got false" : message);
    }
}

void Assert::IsFalse(bool condition, const std::string& message) {
    if (condition) {
        Fail(message.empty() ? "Expected false but got true" : message);
    }
}

void Assert::AreEqual(int expected, int actual, const std::string& message) {
    if (expected != actual) {
        std::ostringstream msg;
        msg << (message.empty() ? "Values not equal" : message);
        msg << " (expected: " << expected << ", actual: " << actual << ")";
        Fail(msg.str());
    }
}

void Assert::AreEqual(const std::string& expected, const std::string& actual, const std::string& message) {
    if (expected != actual) {
        std::ostringstream msg;
        msg << (message.empty() ? "Strings not equal" : message);
        msg << " (expected: \"" << expected << "\", actual: \"" << actual << "\")";
        Fail(msg.str());
    }
}

void Assert::AreNotEqual(int a, int b, const std::string& message) {
    if (a == b) {
        std::ostringstream msg;
        msg << (message.empty() ? "Values should not be equal" : message);
        msg << " (both are: " << a << ")";
        Fail(msg.str());
    }
}

void Assert::IsNull(void* ptr, const std::string& message) {
    if (ptr != nullptr) {
        Fail(message.empty() ? "Expected null pointer" : message);
    }
}

void Assert::IsNotNull(void* ptr, const std::string& message) {
    if (ptr == nullptr) {
        Fail(message.empty() ? "Expected non-null pointer" : message);
    }
}

void Assert::Throws(std::function<void()> func, const std::string& message) {
    bool thrown = false;
    try {
        func();
    } catch (...) {
        thrown = true;
    }
    
    if (!thrown) {
        Fail(message.empty() ? "Expected exception to be thrown" : message);
    }
}

void Assert::DoesNotThrow(std::function<void()> func, const std::string& message) {
    try {
        func();
    } catch (...) {
        Fail(message.empty() ? "Expected no exception to be thrown" : message);
    }
}

void Assert::Fail(const std::string& message) {
    throw std::runtime_error(message);
}

} // namespace testing
} // namespace esp32_ide
