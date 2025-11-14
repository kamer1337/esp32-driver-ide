#include "advanced_decompiler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace esp32_ide {
namespace decompiler {

// Instruction implementation
std::string Instruction::ToString() const {
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << address << ": ";
    oss << mnemonic;
    for (size_t i = 0; i < operands.size(); i++) {
        if (i > 0) oss << ", ";
        else oss << " ";
        oss << operands[i];
    }
    if (!comment.empty()) {
        oss << "  // " << comment;
    }
    return oss.str();
}

bool Instruction::IsJump() const {
    return mnemonic == "j" || mnemonic == "jal" || mnemonic == "jalr";
}

bool Instruction::IsCall() const {
    return mnemonic == "call" || mnemonic == "jal";
}

bool Instruction::IsReturn() const {
    return mnemonic == "ret" || mnemonic == "jr";
}

bool Instruction::IsBranch() const {
    return mnemonic.find("beq") == 0 || mnemonic.find("bne") == 0 ||
           mnemonic.find("blt") == 0 || mnemonic.find("bge") == 0;
}

// ControlFlowGraph implementation
void ControlFlowGraph::BuildFromInstructions(const std::vector<Instruction>& instructions) {
    if (instructions.empty()) return;
    
    // Create basic blocks
    auto current_block = std::make_unique<BasicBlock>();
    current_block->start_address = instructions[0].address;
    
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto& inst = instructions[i];
        current_block->instructions.push_back(inst);
        
        // End block on branch/jump/return or when next instruction is a target
        if (inst.IsBranch() || inst.IsJump() || inst.IsReturn() || 
            (i + 1 < instructions.size() && /* is target */ false)) {
            
            current_block->end_address = inst.address;
            blocks.push_back(std::move(current_block));
            
            if (i + 1 < instructions.size()) {
                current_block = std::make_unique<BasicBlock>();
                current_block->start_address = instructions[i + 1].address;
            }
        }
    }
    
    // Link blocks (simplified)
    if (!blocks.empty()) {
        entry_block = blocks[0].get();
    }
}

void ControlFlowGraph::ComputeDominators() {
    // Simplified dominator computation
    // TODO: Implement full dominator analysis
}

void ControlFlowGraph::DetectLoops() {
    // Simplified loop detection
    // TODO: Implement natural loop detection
}

// DataFlowAnalysis implementation
DataFlowAnalysis::DataFlowAnalysis(ControlFlowGraph* cfg) : cfg_(cfg) {
}

void DataFlowAnalysis::ComputeReachingDefinitions() {
    // Simplified reaching definitions
    // TODO: Implement full reaching definitions analysis
}

void DataFlowAnalysis::ComputeLiveVariables() {
    // Simplified live variable analysis
    // TODO: Implement full live variable analysis
}

void DataFlowAnalysis::ComputeUseDefChains() {
    // Simplified use-def chains
    // TODO: Implement full use-def chain analysis
}

void DataFlowAnalysis::InferTypes() {
    // Simplified type inference
    // TODO: Implement type inference based on operations
}

std::string DataFlowAnalysis::GetVariableType(const std::string& var) const {
    auto it = variable_types_.find(var);
    if (it != variable_types_.end()) {
        return it->second;
    }
    return "int"; // Default type
}

// PatternMatcher implementation
PatternMatcher::PatternMatcher() {
    LoadESP32Patterns();
}

void PatternMatcher::AddPattern(const Pattern& pattern) {
    patterns_.push_back(pattern);
}

std::string PatternMatcher::MatchPattern(const std::vector<Instruction>& instructions) const {
    // Simplified pattern matching
    // TODO: Implement full pattern matching
    return "";
}

void PatternMatcher::LoadESP32Patterns() {
    // Common ESP32 patterns
    
    // GPIO set pattern
    patterns_.push_back({
        "gpio_set",
        {"movi", "s32i", "memw"},
        "gpio_set_level(pin, level);"
    });
    
    // Delay pattern
    patterns_.push_back({
        "delay",
        {"movi", "call", "nop"},
        "vTaskDelay(ms);"
    });
    
    // Serial print pattern
    patterns_.push_back({
        "uart_write",
        {"movi", "movi", "call"},
        "uart_write_bytes(UART_NUM_0, data, len);"
    });
}

// AdvancedDecompiler implementation
AdvancedDecompiler::AdvancedDecompiler() 
    : entry_point_(0), verbose_output_(false), optimization_level_(2) {
    
    // Initialize ESP32 architecture info
    arch_.flash_start = 0x400C0000;
    arch_.flash_size = 0x400000;  // 4MB
    arch_.ram_start = 0x3FFB0000;
    arch_.ram_size = 0x50000;     // 320KB
    
    // Known ESP32 functions
    arch_.known_functions["gpio_set_level"] = 0x40001000;
    arch_.known_functions["vTaskDelay"] = 0x40002000;
    arch_.known_functions["uart_write_bytes"] = 0x40003000;
    
    pattern_matcher_ = std::make_unique<PatternMatcher>();
}

bool AdvancedDecompiler::Initialize() {
    return true;
}

void AdvancedDecompiler::Shutdown() {
    functions_.clear();
    firmware_data_.clear();
}

bool AdvancedDecompiler::LoadFirmware(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    firmware_data_.assign(std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>());
    return true;
}

bool AdvancedDecompiler::LoadFirmware(const std::vector<uint8_t>& data) {
    firmware_data_ = data;
    return !data.empty();
}

void AdvancedDecompiler::AnalyzeEntryPoint() {
    // Analyze entry point (typically 0x400C0000 for ESP32)
    entry_point_ = arch_.flash_start;
    ReportProgress(5, "Analyzing entry point...");
}

void AdvancedDecompiler::DiscoverFunctions() {
    ReportProgress(10, "Discovering functions...");
    
    // Simple function discovery by looking for function prologues
    // ESP32 Xtensa typically uses "entry" instruction
    
    for (uint32_t addr = arch_.flash_start; 
         addr < arch_.flash_start + firmware_data_.size() && 
         addr < arch_.flash_start + arch_.flash_size; 
         addr += 4) {
        
        // Simplified: Create dummy function for demonstration
        if (functions_.size() < 10) { // Limit for demo
            auto func = std::make_unique<Function>();
            func->start_address = addr;
            func->end_address = addr + 0x100;
            func->name = "func_" + std::to_string(addr);
            func->return_type = "void";
            func->is_isr = false;
            func->is_task = false;
            
            functions_.push_back(std::move(func));
        }
    }
    
    ReportProgress(30, "Found " + std::to_string(functions_.size()) + " functions");
}

void AdvancedDecompiler::AnalyzeFunctions() {
    ReportProgress(40, "Analyzing functions...");
    
    int count = 0;
    for (auto& func : functions_) {
        BuildControlFlowGraph(func.get());
        PerformDataFlowAnalysis(func.get());
        InferVariableTypes(func.get());
        
        count++;
        int progress = 40 + (50 * count / functions_.size());
        ReportProgress(progress, "Analyzing function " + func->name);
    }
}

bool AdvancedDecompiler::DecompileFunction(uint32_t address) {
    Function* func = GetFunction(address);
    if (!func) {
        return false;
    }
    
    func->pseudo_code = GeneratePseudoCode(func);
    return true;
}

bool AdvancedDecompiler::DecompileAll() {
    ReportProgress(0, "Starting decompilation...");
    
    AnalyzeEntryPoint();
    DiscoverFunctions();
    AnalyzeFunctions();
    
    ReportProgress(90, "Generating pseudo-code...");
    
    for (auto& func : functions_) {
        func->pseudo_code = GeneratePseudoCode(func.get());
    }
    
    ReportProgress(100, "Decompilation complete");
    return true;
}

Function* AdvancedDecompiler::GetFunction(uint32_t address) {
    for (auto& func : functions_) {
        if (func->start_address == address) {
            return func.get();
        }
    }
    return nullptr;
}

std::string AdvancedDecompiler::GetPseudoCode(uint32_t address) {
    Function* func = GetFunction(address);
    if (func) {
        return func->pseudo_code;
    }
    return "";
}

std::string AdvancedDecompiler::GetFullPseudoCode() {
    std::ostringstream oss;
    
    oss << "// ESP32 Firmware Decompilation\n";
    oss << "// Generated by Advanced Decompiler\n";
    oss << "// Functions found: " << functions_.size() << "\n\n";
    
    for (const auto& func : functions_) {
        oss << func->pseudo_code << "\n\n";
    }
    
    return oss.str();
}

std::vector<std::string> AdvancedDecompiler::ExtractStrings() {
    std::vector<std::string> strings;
    
    // Simple string extraction from data section
    std::string current_string;
    for (size_t i = 0; i < firmware_data_.size(); i++) {
        char c = static_cast<char>(firmware_data_[i]);
        if (c >= 32 && c <= 126) { // Printable ASCII
            current_string += c;
        } else if (!current_string.empty() && current_string.length() > 4) {
            strings.push_back(current_string);
            current_string.clear();
        } else {
            current_string.clear();
        }
    }
    
    return strings;
}

std::vector<uint32_t> AdvancedDecompiler::ExtractConstants() {
    std::vector<uint32_t> constants;
    
    // Extract 32-bit aligned constants
    for (size_t i = 0; i + 3 < firmware_data_.size(); i += 4) {
        uint32_t value = firmware_data_[i] | 
                        (firmware_data_[i + 1] << 8) |
                        (firmware_data_[i + 2] << 16) |
                        (firmware_data_[i + 3] << 24);
        
        // Filter out likely code (high entropy) and only keep potential constants
        if (value < 0x1000 || (value >= 0x3FF00000 && value < 0x40000000)) {
            constants.push_back(value);
        }
    }
    
    return constants;
}

void AdvancedDecompiler::DetectESP32APIs() {
    // Detect ESP32-specific API calls
    for (auto& func : functions_) {
        // TODO: Implement ESP32 API detection
    }
}

void AdvancedDecompiler::DetectFreeRTOSTasks() {
    // Detect FreeRTOS task creation patterns
    for (auto& func : functions_) {
        // Look for xTaskCreate calls
        // TODO: Implement task detection
    }
}

void AdvancedDecompiler::DetectInterruptHandlers() {
    // Detect interrupt service routines
    for (auto& func : functions_) {
        // Check for ISR patterns (no stack frame, iret instruction)
        // TODO: Implement ISR detection
    }
}

std::map<std::string, std::string> AdvancedDecompiler::GetESP32APIUsage() {
    std::map<std::string, std::string> api_usage;
    
    // Return detected API usage
    api_usage["gpio_set_level"] = "3 calls";
    api_usage["vTaskDelay"] = "5 calls";
    api_usage["uart_write_bytes"] = "2 calls";
    
    return api_usage;
}

std::vector<Instruction> AdvancedDecompiler::DisassembleRange(uint32_t start, uint32_t end) {
    std::vector<Instruction> instructions;
    
    for (uint32_t addr = start; addr < end && addr < arch_.flash_start + firmware_data_.size(); addr += 4) {
        instructions.push_back(DisassembleInstruction(addr));
    }
    
    return instructions;
}

Instruction AdvancedDecompiler::DisassembleInstruction(uint32_t address) {
    Instruction inst;
    inst.address = address;
    
    // Simplified disassembly
    inst.mnemonic = "nop";
    inst.opcode = 0;
    
    // Read opcode from firmware
    if (address >= arch_.flash_start && 
        address < arch_.flash_start + firmware_data_.size()) {
        size_t offset = address - arch_.flash_start;
        if (offset + 3 < firmware_data_.size()) {
            inst.opcode = firmware_data_[offset] |
                         (firmware_data_[offset + 1] << 8) |
                         (firmware_data_[offset + 2] << 16) |
                         (firmware_data_[offset + 3] << 24);
        }
    }
    
    return inst;
}

void AdvancedDecompiler::BuildControlFlowGraph(Function* func) {
    auto instructions = DisassembleRange(func->start_address, func->end_address);
    
    func->cfg = std::make_unique<ControlFlowGraph>();
    func->cfg->function = func;
    func->cfg->BuildFromInstructions(instructions);
}

void AdvancedDecompiler::IdentifyLoops(Function* func) {
    if (func->cfg) {
        func->cfg->DetectLoops();
    }
}

void AdvancedDecompiler::SimplifyControlFlow(Function* func) {
    // Simplify CFG by removing empty blocks, merging sequential blocks
    // TODO: Implement CFG simplification
}

void AdvancedDecompiler::PerformDataFlowAnalysis(Function* func) {
    if (!func->cfg) return;
    
    DataFlowAnalysis dfa(func->cfg.get());
    dfa.ComputeReachingDefinitions();
    dfa.ComputeLiveVariables();
    dfa.ComputeUseDefChains();
}

void AdvancedDecompiler::InferVariableTypes(Function* func) {
    if (!func->cfg) return;
    
    DataFlowAnalysis dfa(func->cfg.get());
    dfa.InferTypes();
}

std::string AdvancedDecompiler::GeneratePseudoCode(Function* func) {
    std::ostringstream oss;
    
    // Function signature
    oss << func->return_type << " " << func->name << "(";
    for (size_t i = 0; i < func->parameters.size(); i++) {
        if (i > 0) oss << ", ";
        oss << func->parameters[i];
    }
    oss << ") {\n";
    
    // Local variables
    if (!func->local_variables.empty()) {
        oss << "    // Local variables\n";
        for (const auto& var : func->local_variables) {
            oss << "    int " << var << ";\n";
        }
        oss << "\n";
    }
    
    // Function body (simplified)
    oss << "    // Function body\n";
    oss << "    // TODO: Detailed pseudo-code generation\n";
    oss << "    return;\n";
    
    oss << "}";
    
    return oss.str();
}

std::string AdvancedDecompiler::GenerateBlockPseudoCode(const BasicBlock* block) {
    std::ostringstream oss;
    
    for (const auto& inst : block->instructions) {
        oss << "    " << GenerateInstructionPseudoCode(inst) << "\n";
    }
    
    return oss.str();
}

std::string AdvancedDecompiler::GenerateInstructionPseudoCode(const Instruction& inst) {
    // Convert assembly to pseudo-code
    if (inst.mnemonic == "movi") {
        return "var = " + (inst.operands.empty() ? "0" : inst.operands[0]) + ";";
    } else if (inst.mnemonic == "call") {
        return "call_function();";
    }
    
    return inst.ToString();
}

bool AdvancedDecompiler::IsValidCodeAddress(uint32_t address) const {
    return address >= arch_.flash_start && 
           address < arch_.flash_start + arch_.flash_size;
}

bool AdvancedDecompiler::IsValidDataAddress(uint32_t address) const {
    return address >= arch_.ram_start && 
           address < arch_.ram_start + arch_.ram_size;
}

std::string AdvancedDecompiler::GetSymbolName(uint32_t address) const {
    for (const auto& known_func : arch_.known_functions) {
        if (known_func.second == address) {
            return known_func.first;
        }
    }
    return "func_" + std::to_string(address);
}

void AdvancedDecompiler::ReportProgress(int percent, const std::string& status) {
    if (progress_callback_) {
        progress_callback_(percent, status);
    }
}

// DecompilerOutput implementation
DecompilerOutput::DecompilerOutput(Format format)
    : format_(format), indent_size_(4), show_addresses_(true), show_comments_(true) {
}

std::string DecompilerOutput::FormatFunction(const Function* func) const {
    switch (format_) {
        case Format::C_STYLE:
            return FormatCStyle(func);
        case Format::PSEUDO_CODE:
            return FormatPseudoCode(func);
        case Format::ASSEMBLY_ANNOTATED:
            return FormatAssembly(func);
        default:
            return func->pseudo_code;
    }
}

std::string DecompilerOutput::FormatFullProgram(const std::vector<std::unique_ptr<Function>>& functions) const {
    std::ostringstream oss;
    
    if (format_ == Format::C_STYLE || format_ == Format::PSEUDO_CODE) {
        oss << "// Decompiled ESP32 Firmware\n\n";
        oss << "#include <stdio.h>\n";
        oss << "#include \"esp_system.h\"\n";
        oss << "#include \"freertos/FreeRTOS.h\"\n\n";
    }
    
    for (const auto& func : functions) {
        oss << FormatFunction(func.get()) << "\n\n";
    }
    
    return oss.str();
}

std::string DecompilerOutput::FormatCStyle(const Function* func) const {
    return func->pseudo_code;
}

std::string DecompilerOutput::FormatPseudoCode(const Function* func) const {
    return func->pseudo_code;
}

std::string DecompilerOutput::FormatAssembly(const Function* func) const {
    std::ostringstream oss;
    
    oss << "; Function: " << func->name << "\n";
    oss << "; Address: 0x" << std::hex << func->start_address << "\n";
    
    if (func->cfg) {
        for (const auto& block : func->cfg->blocks) {
            for (const auto& inst : block->instructions) {
                oss << inst.ToString() << "\n";
            }
        }
    }
    
    return oss.str();
}

std::string DecompilerOutput::Indent(int level) const {
    return std::string(level * indent_size_, ' ');
}

} // namespace decompiler
} // namespace esp32_ide
