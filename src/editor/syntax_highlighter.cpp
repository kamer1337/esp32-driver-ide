#include "editor/syntax_highlighter.h"
#include <cctype>

namespace esp32_ide {

SyntaxHighlighter::SyntaxHighlighter() {
    InitializeKeywords();
}

void SyntaxHighlighter::InitializeKeywords() {
    // C/C++ Keywords
    keywords_ = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
        "class", "namespace", "template", "typename", "public", "private", "protected",
        "virtual", "override", "final", "const", "constexpr", "nullptr", "true", "false",
        "try", "catch", "throw", "new", "delete", "this", "using"
    };
    
    // C/C++ Types
    types_ = {
        "bool", "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
        "int64_t", "uint64_t", "size_t", "String", "byte", "word", "boolean"
    };
    
    // Arduino/ESP32 Functions
    arduino_functions_ = {
        "pinMode", "digitalWrite", "digitalRead", "analogRead", "analogWrite",
        "delay", "delayMicroseconds", "millis", "micros", "setup", "loop",
        "Serial", "begin", "print", "println", "available", "read", "write",
        "WiFi", "connect", "disconnect", "status"
    };
}

std::vector<SyntaxHighlighter::Token> SyntaxHighlighter::Tokenize(const std::string& code) const {
    std::vector<Token> tokens;
    size_t i = 0;
    
    while (i < code.length()) {
        // Skip whitespace
        if (IsWhitespace(code[i])) {
            size_t start = i;
            while (i < code.length() && IsWhitespace(code[i])) i++;
            tokens.push_back({TokenType::WHITESPACE, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // Preprocessor directives
        if (code[i] == '#') {
            size_t start = i;
            while (i < code.length() && code[i] != '\n') i++;
            tokens.push_back({TokenType::PREPROCESSOR, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // Single-line comments
        if (i + 1 < code.length() && code[i] == '/' && code[i + 1] == '/') {
            size_t start = i;
            while (i < code.length() && code[i] != '\n') i++;
            tokens.push_back({TokenType::COMMENT, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // Multi-line comments
        if (i + 1 < code.length() && code[i] == '/' && code[i + 1] == '*') {
            size_t start = i;
            i += 2;
            while (i + 1 < code.length() && !(code[i] == '*' && code[i + 1] == '/')) i++;
            if (i + 1 < code.length()) i += 2;
            tokens.push_back({TokenType::COMMENT, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // String literals
        if (code[i] == '"' || code[i] == '\'') {
            char quote = code[i];
            size_t start = i++;
            while (i < code.length() && code[i] != quote) {
                if (code[i] == '\\' && i + 1 < code.length()) i++; // Skip escaped characters
                i++;
            }
            if (i < code.length()) i++; // Include closing quote
            tokens.push_back({TokenType::STRING, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // Numbers
        if (std::isdigit(code[i])) {
            size_t start = i;
            while (i < code.length() && (std::isdigit(code[i]) || code[i] == '.' || 
                   code[i] == 'x' || code[i] == 'X' || std::isxdigit(code[i]) ||
                   code[i] == 'u' || code[i] == 'U' || code[i] == 'l' || code[i] == 'L' ||
                   code[i] == 'f' || code[i] == 'F')) {
                i++;
            }
            tokens.push_back({TokenType::NUMBER, start, i - start, code.substr(start, i - start)});
            continue;
        }
        
        // Identifiers and keywords
        if (std::isalpha(code[i]) || code[i] == '_') {
            size_t start = i;
            while (i < code.length() && (std::isalnum(code[i]) || code[i] == '_')) i++;
            std::string word = code.substr(start, i - start);
            
            TokenType type = TokenType::IDENTIFIER;
            if (IsKeyword(word)) {
                type = TokenType::KEYWORD;
            } else if (IsType(word)) {
                type = TokenType::TYPE;
            } else if (arduino_functions_.count(word)) {
                type = TokenType::FUNCTION;
            }
            
            tokens.push_back({type, start, i - start, word});
            continue;
        }
        
        // Operators
        if (IsOperator(code[i])) {
            size_t start = i;
            // Handle multi-character operators
            if (i + 1 < code.length()) {
                std::string op = code.substr(i, 2);
                if (op == "++" || op == "--" || op == "==" || op == "!=" || 
                    op == "<=" || op == ">=" || op == "&&" || op == "||" ||
                    op == "<<" || op == ">>" || op == "+=" || op == "-=" ||
                    op == "*=" || op == "/=" || op == "%=" || op == "&=" ||
                    op == "|=" || op == "^=" || op == "->") {
                    i += 2;
                    tokens.push_back({TokenType::OPERATOR, start, 2, op});
                    continue;
                }
            }
            tokens.push_back({TokenType::OPERATOR, start, 1, std::string(1, code[i])});
            i++;
            continue;
        }
        
        // Unknown character, skip it
        i++;
    }
    
    return tokens;
}

std::string SyntaxHighlighter::GetColorCode(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD:      return "\033[35m"; // Magenta
        case TokenType::TYPE:         return "\033[36m"; // Cyan
        case TokenType::FUNCTION:     return "\033[33m"; // Yellow
        case TokenType::STRING:       return "\033[32m"; // Green
        case TokenType::COMMENT:      return "\033[90m"; // Gray
        case TokenType::NUMBER:       return "\033[94m"; // Light Blue
        case TokenType::PREPROCESSOR: return "\033[95m"; // Light Magenta
        case TokenType::OPERATOR:     return "\033[37m"; // White
        default:                      return "\033[0m";  // Reset
    }
}

bool SyntaxHighlighter::IsKeyword(const std::string& word) const {
    return keywords_.count(word) > 0;
}

bool SyntaxHighlighter::IsType(const std::string& word) const {
    return types_.count(word) > 0;
}

bool SyntaxHighlighter::IsOperator(char c) const {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '=' || c == '<' || c == '>' || c == '!' || c == '&' ||
           c == '|' || c == '^' || c == '~' || c == '?' || c == ':' ||
           c == '(' || c == ')' || c == '[' || c == ']' || c == '{' ||
           c == '}' || c == ',' || c == ';' || c == '.';
}

bool SyntaxHighlighter::IsWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace esp32_ide
