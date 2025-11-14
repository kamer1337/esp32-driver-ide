#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include <string>
#include <vector>
#include <map>
#include <set>

namespace esp32_ide {

/**
 * @brief Syntax highlighter for C/C++ and Arduino code
 * 
 * Provides syntax highlighting without external dependencies
 */
class SyntaxHighlighter {
public:
    enum class TokenType {
        KEYWORD,
        TYPE,
        FUNCTION,
        STRING,
        COMMENT,
        NUMBER,
        PREPROCESSOR,
        OPERATOR,
        IDENTIFIER,
        WHITESPACE,
        ERROR  // For syntax errors
    };
    
    struct Token {
        TokenType type;
        size_t start;
        size_t length;
        std::string text;
    };
    
    struct SyntaxError {
        size_t line;
        size_t column;
        size_t position;
        std::string message;
        std::string severity;  // "error", "warning", "info"
    };
    
    SyntaxHighlighter();
    
    // Tokenize code
    std::vector<Token> Tokenize(const std::string& code) const;
    
    // Syntax error detection
    std::vector<SyntaxError> CheckSyntax(const std::string& code) const;
    bool HasSyntaxErrors(const std::string& code) const;
    
    // Get ANSI color code for token type (for terminal output)
    static std::string GetColorCode(TokenType type);
    
    // Check if identifier is a keyword
    bool IsKeyword(const std::string& word) const;
    bool IsType(const std::string& word) const;
    
private:
    std::set<std::string> keywords_;
    std::set<std::string> types_;
    std::set<std::string> arduino_functions_;
    
    void InitializeKeywords();
    bool IsOperator(char c) const;
    bool IsWhitespace(char c) const;
};

} // namespace esp32_ide

#endif // SYNTAX_HIGHLIGHTER_H
