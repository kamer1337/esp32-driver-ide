#ifndef ESP32_IDE_SCRIPTING_ENGINE_H
#define ESP32_IDE_SCRIPTING_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace esp32_ide {
namespace scripting {

// Forward declarations
class ScriptContext;
class ScriptValue;
class ScriptFunction;

/**
 * ScriptValue - Represents a value in the scripting system
 */
class ScriptValue {
public:
    enum class Type {
        NIL,
        BOOLEAN,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT,
        FUNCTION
    };

    ScriptValue();
    explicit ScriptValue(bool value);
    explicit ScriptValue(double value);
    explicit ScriptValue(const std::string& value);
    ~ScriptValue() = default;

    Type GetType() const { return type_; }
    bool IsNil() const { return type_ == Type::NIL; }
    bool IsBoolean() const { return type_ == Type::BOOLEAN; }
    bool IsNumber() const { return type_ == Type::NUMBER; }
    bool IsString() const { return type_ == Type::STRING; }
    
    bool AsBoolean() const;
    double AsNumber() const;
    std::string AsString() const;
    
    std::string ToString() const;

private:
    Type type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
};

/**
 * ScriptContext - Execution context for scripts
 */
class ScriptContext {
public:
    ScriptContext();
    ~ScriptContext() = default;

    // Variable management
    void SetVariable(const std::string& name, const ScriptValue& value);
    ScriptValue GetVariable(const std::string& name) const;
    bool HasVariable(const std::string& name) const;
    void ClearVariables();
    
    // Function registration
    using NativeFunction = std::function<ScriptValue(const std::vector<ScriptValue>&)>;
    void RegisterFunction(const std::string& name, NativeFunction func);
    bool HasFunction(const std::string& name) const;
    ScriptValue CallFunction(const std::string& name, const std::vector<ScriptValue>& args);
    
    // Output
    void Print(const std::string& message);
    std::string GetOutput() const { return output_; }
    void ClearOutput() { output_.clear(); }

private:
    std::map<std::string, ScriptValue> variables_;
    std::map<std::string, NativeFunction> functions_;
    std::string output_;
};

/**
 * ScriptEngine - Main scripting engine
 * 
 * A simple scripting engine for automation and device control.
 * Supports basic expressions, variables, functions, and control flow.
 * 
 * Example scripts:
 *   // Set GPIO pin
 *   gpio.set(13, HIGH)
 *   
 *   // Read sensor
 *   temp = sensor.read("temperature")
 *   print("Temperature: " + temp)
 *   
 *   // Control flow
 *   if temp > 30:
 *     led.on(13)
 *   else:
 *     led.off(13)
 *   end
 */
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine() = default;

    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Execution
    bool Execute(const std::string& script);
    bool ExecuteFile(const std::string& filename);
    
    // Context access
    ScriptContext& GetContext() { return context_; }
    const ScriptContext& GetContext() const { return context_; }
    
    // Error handling
    bool HasError() const { return !error_message_.empty(); }
    const std::string& GetErrorMessage() const { return error_message_; }
    void ClearError() { error_message_.clear(); }
    
    // Register standard library functions
    void RegisterStandardLibrary();
    
    // Register device control functions
    void RegisterDeviceFunctions();

private:
    ScriptContext context_;
    std::string error_message_;
    
    // Parser and interpreter
    struct Token {
        enum class Type {
            END,
            IDENTIFIER,
            NUMBER,
            STRING,
            LPAREN,
            RPAREN,
            LBRACE,
            RBRACE,
            COMMA,
            DOT,
            PLUS,
            MINUS,
            STAR,
            SLASH,
            EQUAL,
            EQUAL_EQUAL,
            NOT_EQUAL,
            LESS,
            LESS_EQUAL,
            GREATER,
            GREATER_EQUAL,
            IF,
            ELSE,
            WHILE,
            FOR,
            FUNCTION,
            RETURN,
            END,
            NEWLINE,
            COLON
        };
        
        Type type;
        std::string value;
        int line;
    };
    
    struct Expression;
    struct Statement;
    
    // Lexer
    std::vector<Token> Tokenize(const std::string& source);
    
    // Parser
    std::vector<std::unique_ptr<Statement>> Parse(const std::vector<Token>& tokens);
    
    // Interpreter
    void ExecuteStatements(const std::vector<std::unique_ptr<Statement>>& statements);
    ScriptValue EvaluateExpression(const Expression& expr);
    
    // Helper functions
    void SetError(const std::string& message);
};

/**
 * ScriptLibrary - Collection of example scripts
 */
class ScriptLibrary {
public:
    struct ScriptInfo {
        std::string name;
        std::string description;
        std::string category;
        std::string code;
    };
    
    static std::vector<ScriptInfo> GetExampleScripts();
    static std::string GetScript(const std::string& name);
    
    // Categories
    static std::vector<std::string> GetCategories();
    static std::vector<ScriptInfo> GetScriptsInCategory(const std::string& category);
};

} // namespace scripting
} // namespace esp32_ide

#endif // ESP32_IDE_SCRIPTING_ENGINE_H
