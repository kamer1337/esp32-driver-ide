#ifndef ESP32_IDE_ADVANCED_DECOMPILER_H
#define ESP32_IDE_ADVANCED_DECOMPILER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <functional>

namespace esp32_ide {
namespace decompiler {

/**
 * Advanced ESP32 Firmware Decompiler
 * 
 * High-quality decompilation using advanced methods:
 * - Control Flow Graph (CFG) analysis
 * - Data Flow Analysis (DFA)
 * - Type inference
 * - Function signature detection
 * - String and constant extraction
 * - Pattern-based recognition
 * - ESP32-specific API detection
 */

// Forward declarations
struct Instruction;
struct BasicBlock;
struct Function;
struct ControlFlowGraph;

/**
 * Instruction - Represents a single assembly instruction
 */
struct Instruction {
    uint32_t address;
    uint32_t opcode;
    std::string mnemonic;
    std::vector<std::string> operands;
    std::string comment;
    
    std::string ToString() const;
    bool IsJump() const;
    bool IsCall() const;
    bool IsReturn() const;
    bool IsBranch() const;
};

/**
 * BasicBlock - A sequence of instructions with single entry/exit
 */
struct BasicBlock {
    uint32_t start_address;
    uint32_t end_address;
    std::vector<Instruction> instructions;
    std::vector<BasicBlock*> successors;
    std::vector<BasicBlock*> predecessors;
    
    bool is_visited;
    int depth_first_number;
    
    BasicBlock() : start_address(0), end_address(0), is_visited(false), depth_first_number(-1) {}
};

/**
 * Function - Represents a decompiled function
 */
struct Function {
    uint32_t start_address;
    uint32_t end_address;
    std::string name;
    std::string return_type;
    std::vector<std::string> parameters;
    std::vector<std::string> local_variables;
    std::unique_ptr<ControlFlowGraph> cfg;
    std::string pseudo_code;
    
    // ESP32-specific attributes
    bool is_isr;  // Interrupt Service Routine
    bool is_task; // FreeRTOS task
    std::string task_priority;
    std::string stack_size;
};

/**
 * ControlFlowGraph - CFG for a function
 */
struct ControlFlowGraph {
    Function* function;
    std::vector<std::unique_ptr<BasicBlock>> blocks;
    BasicBlock* entry_block;
    std::vector<BasicBlock*> exit_blocks;
    
    // Analysis results
    std::map<BasicBlock*, std::set<std::string>> live_variables;
    std::map<BasicBlock*, std::set<std::string>> reaching_definitions;
    
    void BuildFromInstructions(const std::vector<Instruction>& instructions);
    void ComputeDominators();
    void DetectLoops();
};

/**
 * DataFlowAnalysis - Data flow analysis for decompilation
 */
class DataFlowAnalysis {
public:
    DataFlowAnalysis(ControlFlowGraph* cfg);
    ~DataFlowAnalysis() = default;
    
    void ComputeReachingDefinitions();
    void ComputeLiveVariables();
    void ComputeUseDefChains();
    void InferTypes();
    
    std::string GetVariableType(const std::string& var) const;

private:
    ControlFlowGraph* cfg_;
    std::map<std::string, std::string> variable_types_;
};

/**
 * PatternMatcher - Recognizes common code patterns
 */
class PatternMatcher {
public:
    struct Pattern {
        std::string name;
        std::vector<std::string> instruction_pattern;
        std::string high_level_code;
    };
    
    PatternMatcher();
    ~PatternMatcher() = default;
    
    void AddPattern(const Pattern& pattern);
    std::string MatchPattern(const std::vector<Instruction>& instructions) const;
    
    // ESP32-specific patterns
    void LoadESP32Patterns();

private:
    std::vector<Pattern> patterns_;
};

/**
 * AdvancedDecompiler - Main decompiler class
 */
class AdvancedDecompiler {
public:
    AdvancedDecompiler();
    ~AdvancedDecompiler() = default;

    // Initialization
    bool Initialize();
    void Shutdown();
    
    // Load firmware
    bool LoadFirmware(const std::string& filename);
    bool LoadFirmware(const std::vector<uint8_t>& data);
    
    // Analysis
    void AnalyzeEntryPoint();
    void DiscoverFunctions();
    void AnalyzeFunctions();
    
    // Decompilation
    bool DecompileFunction(uint32_t address);
    bool DecompileAll();
    
    // Results
    const std::vector<std::unique_ptr<Function>>& GetFunctions() const { return functions_; }
    Function* GetFunction(uint32_t address);
    std::string GetPseudoCode(uint32_t address);
    std::string GetFullPseudoCode();
    
    // String extraction
    std::vector<std::string> ExtractStrings();
    std::vector<uint32_t> ExtractConstants();
    
    // ESP32-specific analysis
    void DetectESP32APIs();
    void DetectFreeRTOSTasks();
    void DetectInterruptHandlers();
    std::map<std::string, std::string> GetESP32APIUsage();
    
    // Settings
    void SetVerboseOutput(bool verbose) { verbose_output_ = verbose; }
    void SetOptimizationLevel(int level) { optimization_level_ = level; }
    
    // Progress callback
    using ProgressCallback = std::function<void(int percent, const std::string& status)>;
    void SetProgressCallback(ProgressCallback callback) { progress_callback_ = callback; }

private:
    std::vector<uint8_t> firmware_data_;
    uint32_t entry_point_;
    std::vector<std::unique_ptr<Function>> functions_;
    std::map<uint32_t, std::string> string_table_;
    std::map<uint32_t, uint32_t> constant_table_;
    std::unique_ptr<PatternMatcher> pattern_matcher_;
    bool verbose_output_;
    int optimization_level_;
    ProgressCallback progress_callback_;
    
    // ESP32 architecture specifics
    struct ESP32Architecture {
        uint32_t flash_start;
        uint32_t flash_size;
        uint32_t ram_start;
        uint32_t ram_size;
        std::map<std::string, uint32_t> known_functions;
    } arch_;
    
    // Disassembly
    std::vector<Instruction> DisassembleRange(uint32_t start, uint32_t end);
    Instruction DisassembleInstruction(uint32_t address);
    
    // Control flow analysis
    void BuildControlFlowGraph(Function* func);
    void IdentifyLoops(Function* func);
    void SimplifyControlFlow(Function* func);
    
    // Data flow analysis
    void PerformDataFlowAnalysis(Function* func);
    void InferVariableTypes(Function* func);
    
    // Pseudo-code generation
    std::string GeneratePseudoCode(Function* func);
    std::string GenerateBlockPseudoCode(const BasicBlock* block);
    std::string GenerateInstructionPseudoCode(const Instruction& inst);
    
    // Helper functions
    bool IsValidCodeAddress(uint32_t address) const;
    bool IsValidDataAddress(uint32_t address) const;
    std::string GetSymbolName(uint32_t address) const;
    void ReportProgress(int percent, const std::string& status);
};

/**
 * DecompilerOutput - Formatting and output generation
 */
class DecompilerOutput {
public:
    enum class Format {
        C_STYLE,
        PSEUDO_CODE,
        ASSEMBLY_ANNOTATED,
        HTML,
        MARKDOWN
    };
    
    DecompilerOutput(Format format);
    ~DecompilerOutput() = default;
    
    std::string FormatFunction(const Function* func) const;
    std::string FormatFullProgram(const std::vector<std::unique_ptr<Function>>& functions) const;
    
    void SetIndentSize(int size) { indent_size_ = size; }
    void SetShowAddresses(bool show) { show_addresses_ = show; }
    void SetShowComments(bool show) { show_comments_ = show; }

private:
    Format format_;
    int indent_size_;
    bool show_addresses_;
    bool show_comments_;
    
    std::string FormatCStyle(const Function* func) const;
    std::string FormatPseudoCode(const Function* func) const;
    std::string FormatAssembly(const Function* func) const;
    std::string Indent(int level) const;
};

} // namespace decompiler
} // namespace esp32_ide

#endif // ESP32_IDE_ADVANCED_DECOMPILER_H
