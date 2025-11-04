#ifndef ESP32_COMPILER_H
#define ESP32_COMPILER_H

#include <string>
#include <vector>
#include <functional>

namespace esp32_ide {

/**
 * @brief ESP32 compiler and build system
 * 
 * Handles compilation and uploading of ESP32 code
 */
class ESP32Compiler {
public:
    enum class BoardType {
        ESP32,
        ESP32_S2,
        ESP32_S3,
        ESP32_C3
    };
    
    enum class CompileStatus {
        SUCCESS,
        ERROR,
        WARNING,
        IN_PROGRESS
    };
    
    struct CompileResult {
        CompileStatus status;
        std::string message;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        size_t program_size;
        size_t data_size;
    };
    
    using OutputCallback = std::function<void(const std::string&, CompileStatus)>;
    
    ESP32Compiler();
    ~ESP32Compiler();
    
    // Compilation
    CompileResult Compile(const std::string& code, BoardType board);
    bool Upload(BoardType board);
    
    // Board selection
    void SetBoard(BoardType board);
    BoardType GetBoard() const;
    std::string GetBoardName(BoardType board) const;
    
    // Output callback
    void SetOutputCallback(OutputCallback callback);
    
    // Syntax checking
    bool CheckSyntax(const std::string& code);
    std::vector<std::string> GetSyntaxErrors(const std::string& code);
    
    // Performance analysis
    struct PerformanceMetrics {
        size_t code_lines;
        size_t estimated_ram_usage;
        size_t estimated_flash_usage;
        std::vector<std::string> warnings;
        std::vector<std::string> optimization_suggestions;
        int performance_score; // 0-100
    };
    
    PerformanceMetrics AnalyzePerformance(const std::string& code);
    
private:
    BoardType current_board_;
    OutputCallback output_callback_;
    
    void OutputMessage(const std::string& message, CompileStatus status);
    bool CheckBracketBalance(const std::string& code);
    bool CheckRequiredFunctions(const std::string& code);
};

} // namespace esp32_ide

#endif // ESP32_COMPILER_H
