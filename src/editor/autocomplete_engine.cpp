#include "editor/autocomplete_engine.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace esp32_ide {

// AutocompleteEngine implementation

AutocompleteEngine::AutocompleteEngine()
    : min_prefix_length_(1), max_suggestions_(50), case_sensitive_(false), fuzzy_matching_(true) {
}

bool AutocompleteEngine::Initialize() {
    InitializeKeywords();
    InitializeESP32API();
    InitializeArduinoAPI();
    InitializeSnippets();
    return true;
}

void AutocompleteEngine::Shutdown() {
    symbols_.clear();
    snippets_.clear();
    keywords_.clear();
}

std::vector<CompletionItem> AutocompleteEngine::GetCompletions(const CompletionContext& context) const {
    if (context.prefix.length() < static_cast<size_t>(min_prefix_length_)) {
        return {};
    }

    if (context.is_inside_string || context.is_inside_comment) {
        return {};
    }

    std::vector<CompletionItem> completions;

    // Get different types of completions
    if (context.is_after_dot || context.is_after_arrow) {
        // Member access - show member functions
        auto members = GetFunctionCompletions(context.prefix, true);
        completions.insert(completions.end(), members.begin(), members.end());
    } else {
        // Regular completions
        auto keywords = GetKeywordCompletions(context.prefix);
        auto functions = GetFunctionCompletions(context.prefix, false);
        auto variables = GetVariableCompletions(context.prefix);
        auto snippets = GetSnippets(context.prefix);

        completions.insert(completions.end(), keywords.begin(), keywords.end());
        completions.insert(completions.end(), functions.begin(), functions.end());
        completions.insert(completions.end(), variables.begin(), variables.end());
        completions.insert(completions.end(), snippets.begin(), snippets.end());
    }

    // Sort and limit results
    SortCompletions(completions, context.prefix);
    
    if (completions.size() > static_cast<size_t>(max_suggestions_)) {
        completions.resize(max_suggestions_);
    }

    return completions;
}

std::vector<CompletionItem> AutocompleteEngine::GetCompletions(const std::string& code, int cursor_position) const {
    CompletionContext context = AnalyzeContext(code, cursor_position);
    return GetCompletions(context);
}

void AutocompleteEngine::AddSymbol(const std::string& name, CompletionItem::Type type,
                                   const std::string& detail, const std::string& documentation) {
    CompletionItem item;
    item.label = name;
    item.insert_text = name;
    item.type = type;
    item.detail = detail;
    item.documentation = documentation;
    item.priority = 5;
    symbols_[name] = item;
}

void AutocompleteEngine::RemoveSymbol(const std::string& name) {
    symbols_.erase(name);
}

void AutocompleteEngine::ClearSymbols() {
    symbols_.clear();
}

void AutocompleteEngine::ParseCodeForSymbols(const std::string& code) {
    // Simple parsing for function and variable declarations
    // This is a simplified implementation
    std::istringstream iss(code);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Look for function definitions
        if (line.find("void ") != std::string::npos || 
            line.find("int ") != std::string::npos ||
            line.find("bool ") != std::string::npos) {
            // Extract function name (simplified)
            size_t paren = line.find('(');
            if (paren != std::string::npos) {
                size_t space = line.rfind(' ', paren);
                if (space != std::string::npos) {
                    std::string name = line.substr(space + 1, paren - space - 1);
                    if (!name.empty() && std::isalpha(name[0])) {
                        AddSymbol(name, CompletionItem::Type::FUNCTION);
                    }
                }
            }
        }
    }
}

void AutocompleteEngine::AddSnippet(const std::string& trigger, const std::string& code,
                                    const std::string& description) {
    CompletionItem item;
    item.label = trigger;
    item.insert_text = code;
    item.type = CompletionItem::Type::SNIPPET;
    item.detail = description;
    item.priority = 10;
    snippets_[trigger] = item;
}

std::vector<CompletionItem> AutocompleteEngine::GetSnippets(const std::string& prefix) const {
    std::vector<CompletionItem> result;
    for (const auto& pair : snippets_) {
        if (MatchesPrefix(pair.first, prefix)) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void AutocompleteEngine::InitializeKeywords() {
    // C/C++ keywords
    keywords_ = {
        "if", "else", "for", "while", "do", "switch", "case", "default",
        "break", "continue", "return", "void", "int", "char", "float", "double",
        "bool", "true", "false", "const", "static", "extern", "volatile",
        "struct", "class", "enum", "typedef", "sizeof", "nullptr",
        "public", "private", "protected", "virtual", "override"
    };
}

void AutocompleteEngine::InitializeESP32API() {
    // ESP32-specific functions
    AddSymbol("esp_restart", CompletionItem::Type::FUNCTION, "void esp_restart()", "Restart ESP32");
    AddSymbol("esp_get_free_heap_size", CompletionItem::Type::FUNCTION, "uint32_t esp_get_free_heap_size()", "Get free heap size");
    AddSymbol("esp_chip_info", CompletionItem::Type::FUNCTION, "void esp_chip_info(esp_chip_info_t* info)", "Get chip information");
    
    // GPIO functions
    AddSymbol("gpio_set_direction", CompletionItem::Type::FUNCTION, "esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode)");
    AddSymbol("gpio_set_level", CompletionItem::Type::FUNCTION, "esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level)");
    AddSymbol("gpio_get_level", CompletionItem::Type::FUNCTION, "int gpio_get_level(gpio_num_t gpio_num)");
}

void AutocompleteEngine::InitializeArduinoAPI() {
    // Arduino core functions
    AddSymbol("pinMode", CompletionItem::Type::FUNCTION, "void pinMode(uint8_t pin, uint8_t mode)", "Set pin mode");
    AddSymbol("digitalWrite", CompletionItem::Type::FUNCTION, "void digitalWrite(uint8_t pin, uint8_t val)", "Write digital value");
    AddSymbol("digitalRead", CompletionItem::Type::FUNCTION, "int digitalRead(uint8_t pin)", "Read digital value");
    AddSymbol("analogRead", CompletionItem::Type::FUNCTION, "int analogRead(uint8_t pin)", "Read analog value");
    AddSymbol("analogWrite", CompletionItem::Type::FUNCTION, "void analogWrite(uint8_t pin, int val)", "Write analog value (PWM)");
    AddSymbol("delay", CompletionItem::Type::FUNCTION, "void delay(unsigned long ms)", "Delay in milliseconds");
    AddSymbol("delayMicroseconds", CompletionItem::Type::FUNCTION, "void delayMicroseconds(unsigned int us)", "Delay in microseconds");
    AddSymbol("millis", CompletionItem::Type::FUNCTION, "unsigned long millis()", "Get milliseconds since boot");
    AddSymbol("micros", CompletionItem::Type::FUNCTION, "unsigned long micros()", "Get microseconds since boot");
    
    // Serial functions
    AddSymbol("Serial.begin", CompletionItem::Type::FUNCTION, "void Serial.begin(unsigned long baud)", "Initialize serial");
    AddSymbol("Serial.print", CompletionItem::Type::FUNCTION, "size_t Serial.print(const String &s)", "Print to serial");
    AddSymbol("Serial.println", CompletionItem::Type::FUNCTION, "size_t Serial.println(const String &s)", "Print line to serial");
    AddSymbol("Serial.available", CompletionItem::Type::FUNCTION, "int Serial.available()", "Check if data available");
    AddSymbol("Serial.read", CompletionItem::Type::FUNCTION, "int Serial.read()", "Read byte from serial");
    
    // Constants
    AddSymbol("HIGH", CompletionItem::Type::CONSTANT, "1", "Logic high level");
    AddSymbol("LOW", CompletionItem::Type::CONSTANT, "0", "Logic low level");
    AddSymbol("INPUT", CompletionItem::Type::CONSTANT, "0x0", "Pin input mode");
    AddSymbol("OUTPUT", CompletionItem::Type::CONSTANT, "0x1", "Pin output mode");
    AddSymbol("INPUT_PULLUP", CompletionItem::Type::CONSTANT, "0x2", "Pin input with pullup");
}

void AutocompleteEngine::InitializeSnippets() {
    AddSnippet("setup", "void setup() {\n  $0\n}", "Arduino setup function");
    AddSnippet("loop", "void loop() {\n  $0\n}", "Arduino loop function");
    AddSnippet("for", "for (int i = 0; i < $1; i++) {\n  $0\n}", "For loop");
    AddSnippet("if", "if ($1) {\n  $0\n}", "If statement");
    AddSnippet("while", "while ($1) {\n  $0\n}", "While loop");
    AddSnippet("switch", "switch ($1) {\n  case $2:\n    $0\n    break;\n  default:\n    break;\n}", "Switch statement");
}

std::vector<CompletionItem> AutocompleteEngine::FilterByPrefix(const std::string& prefix) const {
    std::vector<CompletionItem> result;
    for (const auto& pair : symbols_) {
        if (MatchesPrefix(pair.first, prefix)) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool AutocompleteEngine::MatchesPrefix(const std::string& text, const std::string& prefix) const {
    if (prefix.empty()) return true;
    
    if (case_sensitive_) {
        return text.substr(0, prefix.length()) == prefix;
    } else {
        if (text.length() < prefix.length()) return false;
        for (size_t i = 0; i < prefix.length(); ++i) {
            if (std::tolower(text[i]) != std::tolower(prefix[i])) {
                return false;
            }
        }
        return true;
    }
}

bool AutocompleteEngine::FuzzyMatch(const std::string& text, const std::string& pattern) const {
    size_t text_idx = 0;
    for (char c : pattern) {
        bool found = false;
        while (text_idx < text.length()) {
            if (std::tolower(text[text_idx]) == std::tolower(c)) {
                found = true;
                text_idx++;
                break;
            }
            text_idx++;
        }
        if (!found) return false;
    }
    return true;
}

CompletionContext AutocompleteEngine::AnalyzeContext(const std::string& code, int cursor_position) const {
    CompletionContext context;
    context.cursor_position = cursor_position;
    
    // Extract prefix
    int start = cursor_position - 1;
    while (start >= 0 && (std::isalnum(code[start]) || code[start] == '_')) {
        start--;
    }
    context.prefix = code.substr(start + 1, cursor_position - start - 1);
    
    // Check for member access
    if (start >= 0 && code[start] == '.') {
        context.is_after_dot = true;
    } else if (start >= 1 && code[start - 1] == '-' && code[start] == '>') {
        context.is_after_arrow = true;
    }
    
    // Find current line
    int line_start = cursor_position;
    while (line_start > 0 && code[line_start - 1] != '\n') {
        line_start--;
    }
    int line_end = cursor_position;
    while (line_end < static_cast<int>(code.length()) && code[line_end] != '\n') {
        line_end++;
    }
    context.current_line = code.substr(line_start, line_end - line_start);
    
    return context;
}

std::vector<CompletionItem> AutocompleteEngine::GetKeywordCompletions(const std::string& prefix) const {
    std::vector<CompletionItem> result;
    for (const auto& keyword : keywords_) {
        if (MatchesPrefix(keyword, prefix)) {
            CompletionItem item(keyword, CompletionItem::Type::KEYWORD, 8);
            result.push_back(item);
        }
    }
    return result;
}

std::vector<CompletionItem> AutocompleteEngine::GetFunctionCompletions(const std::string& prefix, bool is_member_access) const {
    std::vector<CompletionItem> result;
    for (const auto& pair : symbols_) {
        if (pair.second.type == CompletionItem::Type::FUNCTION) {
            if (MatchesPrefix(pair.first, prefix)) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

std::vector<CompletionItem> AutocompleteEngine::GetVariableCompletions(const std::string& prefix) const {
    std::vector<CompletionItem> result;
    for (const auto& pair : symbols_) {
        if (pair.second.type == CompletionItem::Type::VARIABLE) {
            if (MatchesPrefix(pair.first, prefix)) {
                result.push_back(pair.second);
            }
        }
    }
    return result;
}

void AutocompleteEngine::SortCompletions(std::vector<CompletionItem>& items, const std::string& prefix) const {
    std::sort(items.begin(), items.end(),
        [this, &prefix](const CompletionItem& a, const CompletionItem& b) {
            int score_a = CalculateScore(a, prefix);
            int score_b = CalculateScore(b, prefix);
            if (score_a != score_b) {
                return score_a > score_b;
            }
            return a.label < b.label;
        });
}

int AutocompleteEngine::CalculateScore(const CompletionItem& item, const std::string& prefix) const {
    int score = item.priority * 100;
    
    // Exact match gets bonus
    if (item.label == prefix) {
        score += 1000;
    }
    // Starts with prefix gets bonus
    else if (item.label.find(prefix) == 0) {
        score += 500;
    }
    // Contains prefix gets smaller bonus
    else if (item.label.find(prefix) != std::string::npos) {
        score += 100;
    }
    
    // Shorter completions preferred
    score -= static_cast<int>(item.label.length());
    
    return score;
}

// SignatureHelpProvider implementation

SignatureHelpProvider::SignatureHelpProvider() {
}

bool SignatureHelpProvider::Initialize() {
    InitializeStandardSignatures();
    InitializeESP32Signatures();
    return true;
}

void SignatureHelpProvider::Shutdown() {
    signatures_.clear();
}

std::vector<SignatureInfo> SignatureHelpProvider::GetSignatureHelp(const std::string& code, int cursor_position) const {
    std::string function_name = GetFunctionName(code, cursor_position);
    if (function_name.empty()) {
        return {};
    }
    
    auto it = signatures_.find(function_name);
    if (it == signatures_.end()) {
        return {};
    }
    
    int active_param = GetActiveParameter(code, cursor_position);
    
    std::vector<SignatureInfo> result = it->second;
    for (auto& sig : result) {
        sig.active_parameter = active_param;
    }
    
    return result;
}

void SignatureHelpProvider::AddSignature(const std::string& function_name, const std::string& signature,
                                        const std::vector<std::string>& param_names,
                                        const std::vector<std::string>& param_docs) {
    SignatureInfo info;
    info.label = signature;
    info.active_parameter = 0;
    
    for (size_t i = 0; i < param_names.size(); ++i) {
        ParameterInfo param;
        param.label = param_names[i];
        if (i < param_docs.size()) {
            param.documentation = param_docs[i];
        }
        info.parameters.push_back(param);
    }
    
    signatures_[function_name].push_back(info);
}

void SignatureHelpProvider::InitializeStandardSignatures() {
    AddSignature("pinMode", "void pinMode(uint8_t pin, uint8_t mode)",
                {"pin", "mode"}, {"Pin number", "INPUT, OUTPUT, or INPUT_PULLUP"});
    AddSignature("digitalWrite", "void digitalWrite(uint8_t pin, uint8_t val)",
                {"pin", "val"}, {"Pin number", "HIGH or LOW"});
    AddSignature("delay", "void delay(unsigned long ms)",
                {"ms"}, {"Delay in milliseconds"});
}

void SignatureHelpProvider::InitializeESP32Signatures() {
    AddSignature("gpio_set_level", "esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level)",
                {"gpio_num", "level"}, {"GPIO pin number", "0 or 1"});
}

std::string SignatureHelpProvider::GetFunctionName(const std::string& code, int position) const {
    // Find the opening parenthesis before cursor
    int paren_pos = position - 1;
    while (paren_pos >= 0 && code[paren_pos] != '(') {
        paren_pos--;
    }
    if (paren_pos < 0) return "";
    
    // Extract function name
    int name_end = paren_pos - 1;
    while (name_end >= 0 && std::isspace(code[name_end])) {
        name_end--;
    }
    int name_start = name_end;
    while (name_start >= 0 && (std::isalnum(code[name_start]) || code[name_start] == '_')) {
        name_start--;
    }
    
    return code.substr(name_start + 1, name_end - name_start);
}

int SignatureHelpProvider::GetActiveParameter(const std::string& code, int position) const {
    // Count commas between opening paren and cursor
    int paren_pos = position - 1;
    while (paren_pos >= 0 && code[paren_pos] != '(') {
        paren_pos--;
    }
    if (paren_pos < 0) return 0;
    
    int param = 0;
    for (int i = paren_pos + 1; i < position; ++i) {
        if (code[i] == ',') {
            param++;
        }
    }
    
    return param;
}

} // namespace esp32_ide
