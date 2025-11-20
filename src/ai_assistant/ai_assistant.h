#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace esp32_ide {

/**
 * @brief AI Assistant for ESP32 development help
 * 
 * Provides context-aware responses to user queries about ESP32 development
 */
class AIAssistant {
public:
    struct Message {
        enum class Sender { USER, ASSISTANT };
        Sender sender;
        std::string content;
        long long timestamp;
    };
    
    AIAssistant();
    ~AIAssistant();
    
    // Query the assistant
    std::string Query(const std::string& user_message);
    
    // Chat history
    std::vector<Message> GetHistory() const;
    void ClearHistory();
    
    // Code analysis
    std::string AnalyzeCode(const std::string& code);
    std::string SuggestFix(const std::string& error_message);
    
    // Code generation
    std::string GenerateCode(const std::string& request);
    std::string GenerateGPIOCode(const std::string& request) const;
    std::string GenerateWiFiCode() const;
    std::string GenerateBluetoothCode() const;
    std::string GenerateSerialCode() const;
    std::string GenerateSensorCode(const std::string& sensor_type) const;
    
    // AI-powered code refactoring
    std::string RefactorCode(const std::string& code, const std::string& refactor_type);
    std::string OptimizeCode(const std::string& code);
    std::string ImproveReadability(const std::string& code);
    
    // Automatic bug detection
    struct BugReport {
        std::string severity; // "critical", "warning", "suggestion"
        int line_number;
        std::string description;
        std::string suggested_fix;
    };
    std::vector<BugReport> DetectBugs(const std::string& code);
    std::string AutoFixBugs(const std::string& code);
    
    // Code completion
    struct CompletionSuggestion {
        std::string text;
        std::string description;
        int priority; // Higher = more relevant
    };
    std::vector<CompletionSuggestion> GetCompletionSuggestions(
        const std::string& code, 
        int cursor_position,
        const std::string& current_line
    );
    
    // Enhanced code generation
    std::string GenerateCompleteSketch(const std::string& description);
    std::string GenerateWebServerCode(const std::string& endpoint_description);
    std::string GenerateMQTTCode(const std::string& topic);
    std::string GenerateOTAUpdateCode();
    std::string GenerateDeepSleepCode(int sleep_seconds);
    
    // Natural language command interpretation (Version 1.3.0)
    struct CommandInterpretation {
        std::string action;        // Interpreted action (e.g., "create_file", "generate_code", "refactor")
        std::string target;        // Target of the action (e.g., "led_blink", "wifi_connection")
        std::map<std::string, std::string> parameters;  // Extracted parameters
        float confidence;          // Confidence score (0.0-1.0)
        std::string raw_command;   // Original command
    };
    CommandInterpretation InterpretNaturalLanguage(const std::string& command);
    std::string ExecuteNaturalLanguageCommand(const std::string& command);
    
    // Advanced code analysis (Version 1.3.0)
    struct SecurityIssue {
        std::string type;          // Type of issue (e.g., "buffer_overflow", "hardcoded_credentials")
        std::string severity;      // "critical", "high", "medium", "low"
        int line_number;
        std::string description;
        std::string recommendation;
    };
    
    struct PerformanceIssue {
        std::string type;          // Type of issue (e.g., "blocking_delay", "inefficient_loop")
        int line_number;
        std::string description;
        std::string optimization;
        int impact_score;          // 1-10 (10 = highest impact)
    };
    
    struct CodeSmell {
        std::string type;          // Type of smell (e.g., "magic_number", "long_function")
        int line_number;
        std::string description;
        std::string refactoring_suggestion;
    };
    
    std::vector<SecurityIssue> ScanSecurityVulnerabilities(const std::string& code);
    std::vector<PerformanceIssue> SuggestPerformanceOptimizations(const std::string& code);
    std::vector<CodeSmell> DetectCodeSmells(const std::string& code);
    std::string GenerateSecurityReport(const std::string& code);
    std::string GeneratePerformanceReport(const std::string& code);
    
    // Learning mode (Version 1.3.0)
    struct UsagePattern {
        std::string feature;       // Feature being used (e.g., "wifi_connection", "gpio_operations")
        int frequency;             // Number of times used
        std::chrono::system_clock::time_point last_used;
        std::vector<std::string> common_parameters;
    };
    
    struct PersonalizedSuggestion {
        std::string suggestion;
        std::string reasoning;     // Why this suggestion is relevant
        float relevance_score;     // 0.0-1.0
        std::string category;      // "code_pattern", "optimization", "feature"
    };
    
    void EnableLearningMode(bool enabled);
    bool IsLearningModeEnabled() const;
    void RecordUsagePattern(const std::string& feature, const std::map<std::string, std::string>& params);
    std::vector<PersonalizedSuggestion> GetPersonalizedSuggestions(const std::string& context);
    std::vector<UsagePattern> GetUsagePatterns() const;
    void ClearUsageHistory();
    
private:
    std::vector<Message> history_;
    bool learning_mode_enabled_;
    std::map<std::string, UsagePattern> usage_patterns_;
    
    // Response generators
    std::string GenerateResponse(const std::string& query) const;
    std::string HandleGPIOQuery() const;
    std::string HandleWiFiQuery() const;
    std::string HandleBluetoothQuery() const;
    std::string HandleSerialQuery() const;
    std::string HandleDebugQuery() const;
    std::string HandleSensorQuery() const;
    std::string HandleTimingQuery() const;
    std::string GetDefaultResponse() const;
    
    void AddMessage(Message::Sender sender, const std::string& content);
    bool ContainsKeywords(const std::string& text, const std::vector<std::string>& keywords) const;
    
    // Helper methods for Version 1.3.0 features
    std::vector<std::string> ExtractCodeLines(const std::string& code) const;
    bool IsHardcodedCredential(const std::string& line) const;
    bool HasBufferOverflowRisk(const std::string& line) const;
    int CalculateComplexity(const std::string& code) const;
};

} // namespace esp32_ide

#endif // AI_ASSISTANT_H
