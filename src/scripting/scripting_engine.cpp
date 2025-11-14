#include "scripting_engine.h"
#include <fstream>
#include <sstream>
#include <cmath>

namespace esp32_ide {
namespace scripting {

// ScriptValue implementation
ScriptValue::ScriptValue() 
    : type_(Type::NIL), bool_value_(false), number_value_(0.0) {
}

ScriptValue::ScriptValue(bool value)
    : type_(Type::BOOLEAN), bool_value_(value), number_value_(0.0) {
}

ScriptValue::ScriptValue(double value)
    : type_(Type::NUMBER), bool_value_(false), number_value_(value) {
}

ScriptValue::ScriptValue(const std::string& value)
    : type_(Type::STRING), bool_value_(false), number_value_(0.0), string_value_(value) {
}

bool ScriptValue::AsBoolean() const {
    if (type_ == Type::BOOLEAN) return bool_value_;
    if (type_ == Type::NUMBER) return number_value_ != 0.0;
    if (type_ == Type::STRING) return !string_value_.empty();
    return false;
}

double ScriptValue::AsNumber() const {
    if (type_ == Type::NUMBER) return number_value_;
    if (type_ == Type::BOOLEAN) return bool_value_ ? 1.0 : 0.0;
    if (type_ == Type::STRING) {
        try {
            return std::stod(string_value_);
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string ScriptValue::AsString() const {
    if (type_ == Type::STRING) return string_value_;
    if (type_ == Type::NUMBER) return std::to_string(number_value_);
    if (type_ == Type::BOOLEAN) return bool_value_ ? "true" : "false";
    return "nil";
}

std::string ScriptValue::ToString() const {
    return AsString();
}

// ScriptContext implementation
ScriptContext::ScriptContext() {
}

void ScriptContext::SetVariable(const std::string& name, const ScriptValue& value) {
    variables_[name] = value;
}

ScriptValue ScriptContext::GetVariable(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    return ScriptValue(); // nil
}

bool ScriptContext::HasVariable(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

void ScriptContext::ClearVariables() {
    variables_.clear();
}

void ScriptContext::RegisterFunction(const std::string& name, NativeFunction func) {
    functions_[name] = func;
}

bool ScriptContext::HasFunction(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

ScriptValue ScriptContext::CallFunction(const std::string& name, const std::vector<ScriptValue>& args) {
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        return it->second(args);
    }
    return ScriptValue(); // nil
}

void ScriptContext::Print(const std::string& message) {
    output_ += message + "\n";
}

// ScriptEngine implementation
ScriptEngine::ScriptEngine() {
}

bool ScriptEngine::Initialize() {
    RegisterStandardLibrary();
    RegisterDeviceFunctions();
    return true;
}

void ScriptEngine::Shutdown() {
    context_.ClearVariables();
}

bool ScriptEngine::Execute(const std::string& script) {
    ClearError();
    
    try {
        // Tokenize
        auto tokens = Tokenize(script);
        
        // Parse
        auto statements = Parse(tokens);
        
        // Execute
        ExecuteStatements(statements);
        
        return !HasError();
    } catch (const std::exception& e) {
        SetError(std::string("Exception: ") + e.what());
        return false;
    }
}

bool ScriptEngine::ExecuteFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SetError("Cannot open file: " + filename);
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return Execute(content);
}

void ScriptEngine::RegisterStandardLibrary() {
    // print function
    context_.RegisterFunction("print", [this](const std::vector<ScriptValue>& args) {
        for (const auto& arg : args) {
            context_.Print(arg.ToString());
        }
        return ScriptValue();
    });
    
    // Math functions
    context_.RegisterFunction("abs", [](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::abs(args[0].AsNumber()));
    });
    
    context_.RegisterFunction("sqrt", [](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::sqrt(args[0].AsNumber()));
    });
    
    context_.RegisterFunction("pow", [](const std::vector<ScriptValue>& args) {
        if (args.size() < 2) return ScriptValue(0.0);
        return ScriptValue(std::pow(args[0].AsNumber(), args[1].AsNumber()));
    });
    
    // String functions
    context_.RegisterFunction("len", [](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(static_cast<double>(args[0].AsString().length()));
    });
}

void ScriptEngine::RegisterDeviceFunctions() {
    // GPIO functions
    context_.RegisterFunction("gpio.set", [this](const std::vector<ScriptValue>& args) {
        if (args.size() < 2) return ScriptValue();
        int pin = static_cast<int>(args[0].AsNumber());
        int value = static_cast<int>(args[1].AsNumber());
        context_.Print("GPIO.set(" + std::to_string(pin) + ", " + std::to_string(value) + ")");
        return ScriptValue();
    });
    
    context_.RegisterFunction("gpio.get", [this](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue(0.0);
        int pin = static_cast<int>(args[0].AsNumber());
        context_.Print("GPIO.get(" + std::to_string(pin) + ")");
        return ScriptValue(0.0); // Simulated value
    });
    
    // LED functions
    context_.RegisterFunction("led.on", [this](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue();
        int pin = static_cast<int>(args[0].AsNumber());
        context_.Print("LED.on(" + std::to_string(pin) + ")");
        return ScriptValue();
    });
    
    context_.RegisterFunction("led.off", [this](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue();
        int pin = static_cast<int>(args[0].AsNumber());
        context_.Print("LED.off(" + std::to_string(pin) + ")");
        return ScriptValue();
    });
    
    // Sensor functions
    context_.RegisterFunction("sensor.read", [this](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue(0.0);
        std::string sensor = args[0].AsString();
        context_.Print("Sensor.read(" + sensor + ")");
        return ScriptValue(25.5); // Simulated sensor value
    });
    
    // Delay function
    context_.RegisterFunction("delay", [this](const std::vector<ScriptValue>& args) {
        if (args.empty()) return ScriptValue();
        int ms = static_cast<int>(args[0].AsNumber());
        context_.Print("delay(" + std::to_string(ms) + " ms)");
        return ScriptValue();
    });
}

std::vector<ScriptEngine::Token> ScriptEngine::Tokenize(const std::string& source) {
    std::vector<Token> tokens;
    // Simplified tokenizer - just return empty for now
    // TODO: Implement full tokenizer
    return tokens;
}

std::vector<std::unique_ptr<ScriptEngine::Statement>> ScriptEngine::Parse(const std::vector<Token>& tokens) {
    std::vector<std::unique_ptr<Statement>> statements;
    // Simplified parser
    // TODO: Implement full parser
    return statements;
}

void ScriptEngine::ExecuteStatements(const std::vector<std::unique_ptr<Statement>>& statements) {
    // Simplified executor
    // TODO: Implement full executor
}

ScriptValue ScriptEngine::EvaluateExpression(const Expression& expr) {
    // Simplified evaluator
    // TODO: Implement full evaluator
    return ScriptValue();
}

void ScriptEngine::SetError(const std::string& message) {
    error_message_ = message;
}

// ScriptLibrary implementation
std::vector<ScriptLibrary::ScriptInfo> ScriptLibrary::GetExampleScripts() {
    return {
        {
            "blink_led",
            "Blink an LED connected to GPIO pin",
            "Basic",
            "// Blink LED on GPIO 13\n"
            "pin = 13\n"
            "gpio.set(pin, 1)\n"
            "delay(1000)\n"
            "gpio.set(pin, 0)\n"
            "delay(1000)\n"
        },
        {
            "read_temperature",
            "Read temperature from sensor",
            "Sensors",
            "// Read temperature sensor\n"
            "temp = sensor.read(\"temperature\")\n"
            "print(\"Temperature: \")\n"
            "print(temp)\n"
            "print(\" C\")\n"
        },
        {
            "temperature_control",
            "Control fan based on temperature",
            "Control",
            "// Temperature-based fan control\n"
            "temp = sensor.read(\"temperature\")\n"
            "if temp > 30:\n"
            "  led.on(13)  // Turn on fan\n"
            "  print(\"Fan ON\")\n"
            "else:\n"
            "  led.off(13)  // Turn off fan\n"
            "  print(\"Fan OFF\")\n"
            "end\n"
        },
        {
            "pwm_control",
            "Control LED brightness with PWM",
            "Advanced",
            "// PWM LED brightness control\n"
            "pin = 13\n"
            "brightness = 128  // 0-255\n"
            "gpio.set(pin, brightness)\n"
            "print(\"Brightness set to \")\n"
            "print(brightness)\n"
        }
    };
}

std::string ScriptLibrary::GetScript(const std::string& name) {
    auto scripts = GetExampleScripts();
    for (const auto& script : scripts) {
        if (script.name == name) {
            return script.code;
        }
    }
    return "";
}

std::vector<std::string> ScriptLibrary::GetCategories() {
    return {"Basic", "Sensors", "Control", "Advanced", "Custom"};
}

std::vector<ScriptLibrary::ScriptInfo> ScriptLibrary::GetScriptsInCategory(const std::string& category) {
    std::vector<ScriptInfo> result;
    auto scripts = GetExampleScripts();
    for (const auto& script : scripts) {
        if (script.category == category) {
            result.push_back(script);
        }
    }
    return result;
}

} // namespace scripting
} // namespace esp32_ide
