#include "compiler/esp32_compiler.h"
#include <algorithm>
#include <sstream>

namespace esp32_ide {

ESP32Compiler::ESP32Compiler() : current_board_(BoardType::ESP32) {}

ESP32Compiler::~ESP32Compiler() = default;

ESP32Compiler::CompileResult ESP32Compiler::Compile(const std::string& code, BoardType board) {
    CompileResult result;
    result.status = CompileStatus::IN_PROGRESS;
    result.program_size = 0;
    result.data_size = 0;
    
    OutputMessage("==================================================", CompileStatus::IN_PROGRESS);
    OutputMessage("Compiling for " + GetBoardName(board) + "...", CompileStatus::WARNING);
    OutputMessage("Checking syntax...", CompileStatus::IN_PROGRESS);
    
    // Check bracket balance
    if (!CheckBracketBalance(code)) {
        result.status = CompileStatus::ERROR;
        result.errors.push_back("Mismatched braces {} or brackets []");
        result.message = "Compilation failed: Syntax errors found";
        OutputMessage(result.message, CompileStatus::ERROR);
        return result;
    }
    
    // Check required functions
    if (!CheckRequiredFunctions(code)) {
        result.status = CompileStatus::WARNING;
        result.warnings.push_back("Missing setup() or loop() function");
        OutputMessage("Warning: Missing setup() or loop() function", CompileStatus::WARNING);
    }
    
    // Simulate compilation success
    result.status = CompileStatus::SUCCESS;
    result.program_size = 234532; // Simulated values
    result.data_size = 28784;
    
    std::ostringstream oss;
    oss << "Sketch uses " << result.program_size << " bytes (17%) of program storage space.";
    OutputMessage(oss.str(), CompileStatus::SUCCESS);
    
    oss.str("");
    oss << "Global variables use " << result.data_size << " bytes (8%) of dynamic memory.";
    OutputMessage(oss.str(), CompileStatus::SUCCESS);
    
    result.message = "Compilation complete!";
    OutputMessage(result.message, CompileStatus::SUCCESS);
    OutputMessage("==================================================", CompileStatus::IN_PROGRESS);
    
    return result;
}

bool ESP32Compiler::Upload(BoardType board) {
    OutputMessage("==================================================", CompileStatus::IN_PROGRESS);
    OutputMessage("Uploading to " + GetBoardName(board) + "...", CompileStatus::WARNING);
    
    // Simulate upload process
    OutputMessage("Connecting to ESP32...", CompileStatus::IN_PROGRESS);
    OutputMessage("Writing at 0x00010000... (10%)", CompileStatus::IN_PROGRESS);
    OutputMessage("Writing at 0x00020000... (50%)", CompileStatus::IN_PROGRESS);
    OutputMessage("Writing at 0x00030000... (100%)", CompileStatus::IN_PROGRESS);
    OutputMessage("Upload successful!", CompileStatus::SUCCESS);
    OutputMessage("Hard resetting via RTS pin...", CompileStatus::SUCCESS);
    OutputMessage("==================================================", CompileStatus::IN_PROGRESS);
    
    return true;
}

void ESP32Compiler::SetBoard(BoardType board) {
    current_board_ = board;
}

ESP32Compiler::BoardType ESP32Compiler::GetBoard() const {
    return current_board_;
}

std::string ESP32Compiler::GetBoardName(BoardType board) const {
    switch (board) {
        case BoardType::ESP32:    return "ESP32 Dev Module";
        case BoardType::ESP32_S2: return "ESP32-S2";
        case BoardType::ESP32_S3: return "ESP32-S3";
        case BoardType::ESP32_C3: return "ESP32-C3";
        default:                  return "Unknown Board";
    }
}

void ESP32Compiler::SetOutputCallback(OutputCallback callback) {
    output_callback_ = callback;
}

bool ESP32Compiler::CheckSyntax(const std::string& code) {
    return CheckBracketBalance(code);
}

std::vector<std::string> ESP32Compiler::GetSyntaxErrors(const std::string& code) {
    std::vector<std::string> errors;
    
    if (!CheckBracketBalance(code)) {
        errors.push_back("Mismatched braces, brackets, or parentheses");
    }
    
    if (!CheckRequiredFunctions(code)) {
        errors.push_back("Missing required setup() or loop() function");
    }
    
    return errors;
}

void ESP32Compiler::OutputMessage(const std::string& message, CompileStatus status) {
    if (output_callback_) {
        output_callback_(message, status);
    }
}

bool ESP32Compiler::CheckBracketBalance(const std::string& code) {
    int braces = 0;
    int brackets = 0;
    int parens = 0;
    bool in_string = false;
    bool in_char = false;
    bool in_comment = false;
    bool in_line_comment = false;
    
    for (size_t i = 0; i < code.length(); i++) {
        char c = code[i];
        
        // Handle line comments
        if (in_line_comment) {
            if (c == '\n') in_line_comment = false;
            continue;
        }
        
        // Handle multi-line comments
        if (in_comment) {
            if (c == '*' && i + 1 < code.length() && code[i + 1] == '/') {
                in_comment = false;
                i++;
            }
            continue;
        }
        
        // Check for comment start
        if (c == '/' && i + 1 < code.length()) {
            if (code[i + 1] == '/') {
                in_line_comment = true;
                continue;
            } else if (code[i + 1] == '*') {
                in_comment = true;
                i++;
                continue;
            }
        }
        
        // Handle strings
        if (c == '"' && !in_char) {
            if (i == 0 || code[i - 1] != '\\') {
                in_string = !in_string;
            }
            continue;
        }
        
        // Handle character literals
        if (c == '\'' && !in_string) {
            if (i == 0 || code[i - 1] != '\\') {
                in_char = !in_char;
            }
            continue;
        }
        
        if (in_string || in_char) continue;
        
        // Count brackets
        if (c == '{') braces++;
        else if (c == '}') braces--;
        else if (c == '[') brackets++;
        else if (c == ']') brackets--;
        else if (c == '(') parens++;
        else if (c == ')') parens--;
    }
    
    return braces == 0 && brackets == 0 && parens == 0;
}

bool ESP32Compiler::CheckRequiredFunctions(const std::string& code) {
    return code.find("void setup()") != std::string::npos &&
           code.find("void loop()") != std::string::npos;
}

// Performance analysis implementation
ESP32Compiler::PerformanceMetrics ESP32Compiler::AnalyzePerformance(const std::string& code) {
    PerformanceMetrics metrics;
    metrics.performance_score = 100;
    
    // Count lines of code
    metrics.code_lines = std::count(code.begin(), code.end(), '\n') + 1;
    
    // Memory estimation constants
    constexpr size_t BASE_RAM_USAGE = 4096;      // Base RAM overhead
    constexpr size_t STRING_OBJECT_SIZE = 32;     // Approximate size per String object
    constexpr size_t COMPILED_SIZE_MULTIPLIER = 4; // Approximate compiled size factor
    
    // Estimate RAM usage
    size_t string_count = 0;
    size_t pos = 0;
    while ((pos = code.find("String ", pos)) != std::string::npos) {
        string_count++;
        pos++;
    }
    metrics.estimated_ram_usage = BASE_RAM_USAGE + (string_count * STRING_OBJECT_SIZE);
    
    // Estimate flash usage (rough approximation)
    metrics.estimated_flash_usage = code.size() * COMPILED_SIZE_MULTIPLIER;
    
    // Check for performance issues
    
    // Check for blocking delays in loop
    if (code.find("void loop()") != std::string::npos && code.find("delay(") != std::string::npos) {
        metrics.warnings.push_back("Blocking delay() calls detected in loop()");
        metrics.optimization_suggestions.push_back("Consider using millis() for non-blocking timing");
        metrics.performance_score -= 10;
    }
    
    // Check for String usage (memory intensive)
    if (string_count > 0) {
        metrics.warnings.push_back("String objects detected - may cause memory fragmentation");
        metrics.optimization_suggestions.push_back("Consider using char arrays for better memory management");
        metrics.performance_score -= 5 * string_count;
    }
    
    // Check for Serial.print in loops
    size_t loop_pos = code.find("for(");
    if (loop_pos != std::string::npos) {
        size_t loop_end = code.find("}", loop_pos);
        if (loop_end != std::string::npos) {
            std::string loop_body = code.substr(loop_pos, loop_end - loop_pos);
            if (loop_body.find("Serial.print") != std::string::npos) {
                metrics.warnings.push_back("Serial.print() calls inside loops may slow execution");
                metrics.optimization_suggestions.push_back("Minimize Serial output in tight loops");
                metrics.performance_score -= 15;
            }
        }
    }
    
    // Check for inefficient analogRead usage
    size_t analog_read_count = 0;
    pos = 0;
    while ((pos = code.find("analogRead(", pos)) != std::string::npos) {
        analog_read_count++;
        pos++;
    }
    if (analog_read_count > 5) {
        metrics.warnings.push_back("Multiple analogRead() calls detected");
        metrics.optimization_suggestions.push_back("Cache analog readings if reading same pin multiple times");
        metrics.performance_score -= 5;
    }
    
    // Check for delay in interrupts
    if (code.find("IRAM_ATTR") != std::string::npos || code.find("ISR") != std::string::npos) {
        if (code.find("delay(") != std::string::npos) {
            metrics.warnings.push_back("CRITICAL: delay() in interrupt service routine");
            metrics.optimization_suggestions.push_back("Remove all blocking calls from ISRs");
            metrics.performance_score -= 30;
        }
    }
    
    // Check WiFi without proper connection handling
    if (code.find("WiFi.") != std::string::npos) {
        if (code.find("while (WiFi.status()") == std::string::npos && 
            code.find("if (WiFi.status()") == std::string::npos) {
            metrics.warnings.push_back("WiFi operations without status checking");
            metrics.optimization_suggestions.push_back("Always check WiFi.status() before using WiFi");
            metrics.performance_score -= 10;
        }
    }
    
    // Memory warnings
    if (metrics.estimated_ram_usage > 200000) {
        metrics.warnings.push_back("High estimated RAM usage - may cause crashes");
        metrics.optimization_suggestions.push_back("Review data structures and reduce memory footprint");
        metrics.performance_score -= 20;
    }
    
    // Ensure score doesn't go below 0
    if (metrics.performance_score < 0) {
        metrics.performance_score = 0;
    }
    
    return metrics;
}

} // namespace esp32_ide
