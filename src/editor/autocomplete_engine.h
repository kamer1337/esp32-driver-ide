#ifndef AUTOCOMPLETE_ENGINE_H
#define AUTOCOMPLETE_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

namespace esp32_ide {

/**
 * @brief Completion item representing a single autocomplete suggestion
 */
struct CompletionItem {
    enum class Type {
        KEYWORD,
        FUNCTION,
        VARIABLE,
        CLASS,
        STRUCT,
        ENUM,
        MACRO,
        CONSTANT,
        SNIPPET,
        FILE,
        FOLDER
    };

    std::string label;          // Display text
    std::string insert_text;    // Text to insert
    std::string detail;         // Additional info (e.g., function signature)
    std::string documentation;  // Full documentation
    Type type;
    int priority;               // Higher priority appears first

    CompletionItem()
        : type(Type::KEYWORD), priority(0) {}

    CompletionItem(const std::string& label, Type type, int priority = 0)
        : label(label), insert_text(label), type(type), priority(priority) {}
};

/**
 * @brief Context for autocomplete suggestions
 */
struct CompletionContext {
    std::string current_line;
    std::string prefix;              // Text before cursor
    std::string trigger_character;   // Character that triggered completion (e.g., ".", "->")
    int cursor_position;
    int line_number;
    bool is_inside_string;
    bool is_inside_comment;
    bool is_after_dot;
    bool is_after_arrow;
};

/**
 * @brief Autocomplete engine for code completion
 */
class AutocompleteEngine {
public:
    AutocompleteEngine();
    ~AutocompleteEngine() = default;

    // Initialization
    bool Initialize();
    void Shutdown();

    // Get completions
    std::vector<CompletionItem> GetCompletions(const CompletionContext& context) const;
    std::vector<CompletionItem> GetCompletions(const std::string& code, int cursor_position) const;

    // Symbol management
    void AddSymbol(const std::string& name, CompletionItem::Type type, 
                   const std::string& detail = "", const std::string& documentation = "");
    void RemoveSymbol(const std::string& name);
    void ClearSymbols();

    // Parse code for symbols
    void ParseCodeForSymbols(const std::string& code);

    // Configuration
    void SetMinimumPrefixLength(int length) { min_prefix_length_ = length; }
    void SetMaxSuggestions(int max) { max_suggestions_ = max; }
    void SetCaseSensitive(bool sensitive) { case_sensitive_ = sensitive; }
    void EnableFuzzyMatching(bool enable) { fuzzy_matching_ = enable; }

    // Snippets
    void AddSnippet(const std::string& trigger, const std::string& code, 
                    const std::string& description = "");
    std::vector<CompletionItem> GetSnippets(const std::string& prefix) const;

private:
    std::map<std::string, CompletionItem> symbols_;
    std::map<std::string, CompletionItem> snippets_;
    std::set<std::string> keywords_;
    
    int min_prefix_length_;
    int max_suggestions_;
    bool case_sensitive_;
    bool fuzzy_matching_;

    // Completion helpers
    void InitializeKeywords();
    void InitializeESP32API();
    void InitializeArduinoAPI();
    void InitializeSnippets();
    
    std::vector<CompletionItem> FilterByPrefix(const std::string& prefix) const;
    bool MatchesPrefix(const std::string& text, const std::string& prefix) const;
    bool FuzzyMatch(const std::string& text, const std::string& pattern) const;
    
    CompletionContext AnalyzeContext(const std::string& code, int cursor_position) const;
    std::vector<CompletionItem> GetKeywordCompletions(const std::string& prefix) const;
    std::vector<CompletionItem> GetFunctionCompletions(const std::string& prefix, bool is_member_access) const;
    std::vector<CompletionItem> GetVariableCompletions(const std::string& prefix) const;
    
    void SortCompletions(std::vector<CompletionItem>& items, const std::string& prefix) const;
    int CalculateScore(const CompletionItem& item, const std::string& prefix) const;
};

/**
 * @brief Signature help for function parameters
 */
struct ParameterInfo {
    std::string label;
    std::string documentation;
};

struct SignatureInfo {
    std::string label;
    std::string documentation;
    std::vector<ParameterInfo> parameters;
    int active_parameter;
};

/**
 * @brief Signature help provider
 */
class SignatureHelpProvider {
public:
    SignatureHelpProvider();
    ~SignatureHelpProvider() = default;

    bool Initialize();
    void Shutdown();

    // Get signature help
    std::vector<SignatureInfo> GetSignatureHelp(const std::string& code, int cursor_position) const;
    
    // Add function signature
    void AddSignature(const std::string& function_name, const std::string& signature,
                     const std::vector<std::string>& param_names,
                     const std::vector<std::string>& param_docs = {});

private:
    std::map<std::string, std::vector<SignatureInfo>> signatures_;
    
    void InitializeStandardSignatures();
    void InitializeESP32Signatures();
    
    std::string GetFunctionName(const std::string& code, int position) const;
    int GetActiveParameter(const std::string& code, int position) const;
};

} // namespace esp32_ide

#endif // AUTOCOMPLETE_ENGINE_H
