#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include <string>
#include <vector>

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
    
private:
    std::vector<Message> history_;
    
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
};

} // namespace esp32_ide

#endif // AI_ASSISTANT_H
