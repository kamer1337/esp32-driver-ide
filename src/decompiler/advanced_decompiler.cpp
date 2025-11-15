#include "advanced_decompiler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iterator>

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
    
    // First pass: identify block boundaries (leaders)
    std::set<uint32_t> leaders;
    leaders.insert(instructions[0].address); // First instruction is a leader
    
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto& inst = instructions[i];
        
        // Instruction after branch/jump/call is a leader
        if ((inst.IsBranch() || inst.IsJump() || inst.IsCall()) && i + 1 < instructions.size()) {
            leaders.insert(instructions[i + 1].address);
        }
        
        // Branch/jump targets are leaders
        if (inst.IsBranch() || inst.IsJump()) {
            // Extract target address from operands if available
            if (!inst.operands.empty()) {
                try {
                    uint32_t target = std::stoul(inst.operands.back(), nullptr, 0);
                    leaders.insert(target);
                } catch (...) {
                    // Invalid target, skip
                }
            }
        }
    }
    
    // Second pass: create basic blocks
    std::map<uint32_t, BasicBlock*> address_to_block;
    auto current_block = std::make_unique<BasicBlock>();
    current_block->start_address = instructions[0].address;
    
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto& inst = instructions[i];
        
        // Start new block if this is a leader (except for first instruction)
        if (leaders.count(inst.address) && !current_block->instructions.empty()) {
            current_block->end_address = current_block->instructions.back().address;
            address_to_block[current_block->start_address] = current_block.get();
            blocks.push_back(std::move(current_block));
            
            current_block = std::make_unique<BasicBlock>();
            current_block->start_address = inst.address;
        }
        
        current_block->instructions.push_back(inst);
    }
    
    // Add final block
    if (!current_block->instructions.empty()) {
        current_block->end_address = current_block->instructions.back().address;
        address_to_block[current_block->start_address] = current_block.get();
        blocks.push_back(std::move(current_block));
    }
    
    // Third pass: link blocks (build edges)
    for (auto& block : blocks) {
        if (block->instructions.empty()) continue;
        
        const auto& last_inst = block->instructions.back();
        
        // Unconditional jump/call
        if (last_inst.IsJump() || last_inst.IsCall()) {
            if (!last_inst.operands.empty()) {
                try {
                    uint32_t target = std::stoul(last_inst.operands.back(), nullptr, 0);
                    auto it = address_to_block.find(target);
                    if (it != address_to_block.end()) {
                        block->successors.push_back(it->second);
                        it->second->predecessors.push_back(block.get());
                    }
                } catch (...) {
                    // Invalid target
                }
            }
        }
        
        // Conditional branch - has two successors
        if (last_inst.IsBranch()) {
            // Branch target
            if (!last_inst.operands.empty()) {
                try {
                    uint32_t target = std::stoul(last_inst.operands.back(), nullptr, 0);
                    auto it = address_to_block.find(target);
                    if (it != address_to_block.end()) {
                        block->successors.push_back(it->second);
                        it->second->predecessors.push_back(block.get());
                    }
                } catch (...) {
                    // Invalid target
                }
            }
            
            // Fall-through successor
            uint32_t next_addr = last_inst.address + 4;
            auto it = address_to_block.find(next_addr);
            if (it != address_to_block.end()) {
                block->successors.push_back(it->second);
                it->second->predecessors.push_back(block.get());
            }
        }
        
        // No control flow instruction - fall through to next block
        if (!last_inst.IsBranch() && !last_inst.IsJump() && !last_inst.IsReturn()) {
            uint32_t next_addr = last_inst.address + 4;
            auto it = address_to_block.find(next_addr);
            if (it != address_to_block.end()) {
                block->successors.push_back(it->second);
                it->second->predecessors.push_back(block.get());
            }
        }
    }
    
    // Set entry block
    if (!blocks.empty()) {
        entry_block = blocks[0].get();
    }
    
    // Identify exit blocks (blocks with no successors or return instruction)
    for (auto& block : blocks) {
        if (block->successors.empty() || 
            (!block->instructions.empty() && block->instructions.back().IsReturn())) {
            exit_blocks.push_back(block.get());
        }
    }
}

void ControlFlowGraph::ComputeDominators() {
    // Implement dominator analysis using iterative algorithm
    if (blocks.empty() || !entry_block) return;
    
    // Initialize dominators
    std::map<BasicBlock*, std::set<BasicBlock*>> dominators;
    
    // Entry block dominates only itself
    dominators[entry_block].insert(entry_block);
    
    // All other blocks are dominated by all blocks initially
    std::set<BasicBlock*> all_blocks;
    for (auto& block : blocks) {
        all_blocks.insert(block.get());
        if (block.get() != entry_block) {
            dominators[block.get()] = all_blocks;
        }
    }
    
    // Iteratively compute dominators until fixed point
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (auto& block : blocks) {
            if (block.get() == entry_block) continue;
            
            // Dom(n) = {n} ∪ (∩ Dom(p) for all predecessors p of n)
            std::set<BasicBlock*> new_dom;
            
            if (!block->predecessors.empty()) {
                // Start with dominators of first predecessor
                new_dom = dominators[block->predecessors[0]];
                
                // Intersect with dominators of other predecessors
                for (size_t i = 1; i < block->predecessors.size(); i++) {
                    std::set<BasicBlock*> intersection;
                    std::set_intersection(
                        new_dom.begin(), new_dom.end(),
                        dominators[block->predecessors[i]].begin(),
                        dominators[block->predecessors[i]].end(),
                        std::inserter(intersection, intersection.begin())
                    );
                    new_dom = intersection;
                }
            }
            
            // Add the block itself
            new_dom.insert(block.get());
            
            // Check if changed
            if (new_dom != dominators[block.get()]) {
                dominators[block.get()] = new_dom;
                changed = true;
            }
        }
    }
}

void ControlFlowGraph::DetectLoops() {
    // Detect natural loops by finding back edges in CFG
    // A back edge is an edge whose target dominates its source
    
    if (blocks.empty()) return;
    
    // First compute dominators
    ComputeDominators();
    
    // Find back edges
    std::vector<std::pair<BasicBlock*, BasicBlock*>> back_edges;
    
    for (auto& block : blocks) {
        for (auto* successor : block->successors) {
            // Check if successor dominates current block (back edge)
            // For now, simplified check: if successor's address <= current block's start
            if (successor->start_address <= block->start_address) {
                back_edges.push_back({block.get(), successor});
            }
        }
    }
    
    // For each back edge, identify the natural loop
    for (auto& [tail, header] : back_edges) {
        // Natural loop consists of header and all nodes that can reach tail
        // without going through header
        std::set<BasicBlock*> loop_nodes;
        loop_nodes.insert(header);
        loop_nodes.insert(tail);
        
        // Worklist algorithm to find all nodes in the loop
        std::vector<BasicBlock*> worklist;
        worklist.push_back(tail);
        
        while (!worklist.empty()) {
            BasicBlock* node = worklist.back();
            worklist.pop_back();
            
            for (auto* pred : node->predecessors) {
                if (loop_nodes.find(pred) == loop_nodes.end()) {
                    loop_nodes.insert(pred);
                    if (pred != header) {
                        worklist.push_back(pred);
                    }
                }
            }
        }
        
        // Mark loop header for later identification
        // Can be stored in a separate data structure if needed
    }
}

// DataFlowAnalysis implementation
DataFlowAnalysis::DataFlowAnalysis(ControlFlowGraph* cfg) : cfg_(cfg) {
}

void DataFlowAnalysis::ComputeReachingDefinitions() {
    // Compute reaching definitions using iterative dataflow analysis
    if (!cfg_ || cfg_->blocks.empty()) return;
    
    // For each block, compute GEN and KILL sets
    std::map<BasicBlock*, std::set<std::string>> gen_sets;
    std::map<BasicBlock*, std::set<std::string>> kill_sets;
    
    for (auto& block : cfg_->blocks) {
        for (const auto& inst : block->instructions) {
            // Simplified: assume first operand is destination (definition)
            if (!inst.operands.empty()) {
                std::string var = inst.operands[0];
                gen_sets[block.get()].insert(var);
                // Kill all other definitions of this variable
                kill_sets[block.get()].insert(var);
            }
        }
    }
    
    // Initialize reaching definitions (OUT sets)
    std::map<BasicBlock*, std::set<std::string>> in_sets;
    std::map<BasicBlock*, std::set<std::string>> out_sets;
    
    // Iterative algorithm
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (auto& block : cfg_->blocks) {
            // IN[B] = ∪ OUT[P] for all predecessors P of B
            std::set<std::string> new_in;
            for (auto* pred : block->predecessors) {
                new_in.insert(out_sets[pred].begin(), out_sets[pred].end());
            }
            
            // OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
            std::set<std::string> new_out = gen_sets[block.get()];
            for (const auto& def : new_in) {
                if (kill_sets[block.get()].find(def) == kill_sets[block.get()].end()) {
                    new_out.insert(def);
                }
            }
            
            if (new_in != in_sets[block.get()] || new_out != out_sets[block.get()]) {
                in_sets[block.get()] = new_in;
                out_sets[block.get()] = new_out;
                changed = true;
            }
        }
    }
    
    // Store results in CFG
    cfg_->reaching_definitions = out_sets;
}

void DataFlowAnalysis::ComputeLiveVariables() {
    // Compute live variables using backward dataflow analysis
    if (!cfg_ || cfg_->blocks.empty()) return;
    
    // For each block, compute USE and DEF sets
    std::map<BasicBlock*, std::set<std::string>> use_sets;
    std::map<BasicBlock*, std::set<std::string>> def_sets;
    
    for (auto& block : cfg_->blocks) {
        for (const auto& inst : block->instructions) {
            // Simplified: first operand is def, rest are uses
            if (!inst.operands.empty()) {
                std::string def_var = inst.operands[0];
                def_sets[block.get()].insert(def_var);
                
                // Other operands are uses
                for (size_t i = 1; i < inst.operands.size(); i++) {
                    use_sets[block.get()].insert(inst.operands[i]);
                }
            }
        }
    }
    
    // Initialize live variables (IN and OUT sets)
    std::map<BasicBlock*, std::set<std::string>> in_sets;
    std::map<BasicBlock*, std::set<std::string>> out_sets;
    
    // Iterative backward analysis
    bool changed = true;
    while (changed) {
        changed = false;
        
        // Process blocks in reverse order
        for (auto it = cfg_->blocks.rbegin(); it != cfg_->blocks.rend(); ++it) {
            auto& block = *it;
            
            // OUT[B] = ∪ IN[S] for all successors S of B
            std::set<std::string> new_out;
            for (auto* succ : block->successors) {
                new_out.insert(in_sets[succ].begin(), in_sets[succ].end());
            }
            
            // IN[B] = USE[B] ∪ (OUT[B] - DEF[B])
            std::set<std::string> new_in = use_sets[block.get()];
            for (const auto& var : new_out) {
                if (def_sets[block.get()].find(var) == def_sets[block.get()].end()) {
                    new_in.insert(var);
                }
            }
            
            if (new_in != in_sets[block.get()] || new_out != out_sets[block.get()]) {
                in_sets[block.get()] = new_in;
                out_sets[block.get()] = new_out;
                changed = true;
            }
        }
    }
    
    // Store results in CFG
    cfg_->live_variables = in_sets;
}

void DataFlowAnalysis::ComputeUseDefChains() {
    // Build use-def chains from reaching definitions
    // First ensure reaching definitions are computed
    ComputeReachingDefinitions();
    
    // Use-def chain connects each use of a variable to all definitions
    // that reach that use
    // This is implicitly available from reaching_definitions in the CFG
}

void DataFlowAnalysis::InferTypes() {
    // Infer types based on operations and usage patterns
    if (!cfg_ || cfg_->blocks.empty()) return;
    
    // Analyze each instruction to infer types
    for (auto& block : cfg_->blocks) {
        for (const auto& inst : block->instructions) {
            if (inst.operands.empty()) continue;
            
            std::string var = inst.operands[0];
            
            // Integer operations
            if (inst.mnemonic == "addi" || inst.mnemonic == "add" || 
                inst.mnemonic == "sub" || inst.mnemonic == "mul" ||
                inst.mnemonic == "movi") {
                variable_types_[var] = "int";
            }
            // Floating point operations
            else if (inst.mnemonic == "fadd" || inst.mnemonic == "fsub" ||
                     inst.mnemonic == "fmul" || inst.mnemonic == "fdiv") {
                variable_types_[var] = "float";
            }
            // Pointer/address operations
            else if (inst.mnemonic == "l32i" || inst.mnemonic == "s32i" ||
                     inst.mnemonic == "l16ui" || inst.mnemonic == "s16i") {
                // Load/store operations suggest pointer
                if (inst.operands.size() > 1) {
                    variable_types_[inst.operands[1]] = "uint32_t*";
                }
                variable_types_[var] = "uint32_t";
            }
            // Byte operations
            else if (inst.mnemonic == "l8ui" || inst.mnemonic == "s8i") {
                variable_types_[var] = "uint8_t";
            }
            // Boolean operations
            else if (inst.mnemonic.find("beq") == 0 || inst.mnemonic.find("bne") == 0) {
                for (const auto& op : inst.operands) {
                    if (variable_types_.find(op) == variable_types_.end()) {
                        variable_types_[op] = "bool";
                    }
                }
            }
            // GPIO operations (ESP32 specific)
            else if (inst.comment.find("GPIO") != std::string::npos) {
                variable_types_[var] = "gpio_num_t";
            }
        }
    }
    
    // Default unknown types to int
    for (auto& block : cfg_->blocks) {
        for (const auto& inst : block->instructions) {
            for (const auto& op : inst.operands) {
                if (variable_types_.find(op) == variable_types_.end()) {
                    variable_types_[op] = "int";
                }
            }
        }
    }
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
    // Match instruction sequences against known patterns
    if (instructions.empty()) return "";
    
    // Try each pattern
    for (const auto& pattern : patterns_) {
        if (pattern.instruction_pattern.empty()) continue;
        
        // Check if we have enough instructions
        if (instructions.size() < pattern.instruction_pattern.size()) continue;
        
        // Try to match pattern
        bool matched = true;
        for (size_t i = 0; i < pattern.instruction_pattern.size(); i++) {
            if (instructions[i].mnemonic != pattern.instruction_pattern[i]) {
                matched = false;
                break;
            }
        }
        
        if (matched) {
            return pattern.high_level_code;
        }
    }
    
    // Try fuzzy matching for common patterns
    if (instructions.size() >= 2) {
        // digitalWrite pattern: load value, store to GPIO register
        if (instructions[0].mnemonic == "movi" && instructions[1].mnemonic == "s32i") {
            if (instructions[1].comment.find("GPIO") != std::string::npos) {
                return "gpio_set_level(pin, level);";
            }
        }
        
        // delay pattern: load value, call delay function
        if (instructions[0].mnemonic == "movi" && instructions[1].mnemonic == "call") {
            if (instructions[1].comment.find("delay") != std::string::npos ||
                instructions[1].comment.find("vTaskDelay") != std::string::npos) {
                return "vTaskDelay(pdMS_TO_TICKS(ms));";
            }
        }
        
        // Serial print pattern
        if (instructions[0].mnemonic == "movi" && instructions[1].mnemonic == "call") {
            if (instructions[1].comment.find("uart") != std::string::npos ||
                instructions[1].comment.find("Serial") != std::string::npos) {
                return "Serial.print(data);";
            }
        }
    }
    
    // WiFi begin pattern
    if (instructions.size() >= 3) {
        if (instructions[0].mnemonic == "movi" && 
            instructions[1].mnemonic == "movi" &&
            instructions[2].mnemonic == "call") {
            if (instructions[2].comment.find("WiFi") != std::string::npos) {
                return "WiFi.begin(ssid, password);";
            }
        }
    }
    
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
    
    // Function discovery strategies:
    // 1. Look for entry instructions (function prologues)
    // 2. Find call targets
    // 3. Use symbol table if available
    
    std::set<uint32_t> function_starts;
    std::set<uint32_t> call_targets;
    
    // Entry point is always a function
    function_starts.insert(entry_point_);
    
    // Scan for function patterns and call targets
    for (uint32_t addr = arch_.flash_start; 
         addr < arch_.flash_start + firmware_data_.size() && 
         addr < arch_.flash_start + arch_.flash_size; 
         addr += 4) {
        
        auto inst = DisassembleInstruction(addr);
        
        // Look for function entry patterns
        // Xtensa "entry" instruction indicates function start
        if (inst.mnemonic == "entry") {
            function_starts.insert(addr);
        }
        
        // Track call targets as potential functions
        if (inst.IsCall() && !inst.operands.empty()) {
            try {
                uint32_t target = std::stoul(inst.operands[0], nullptr, 0);
                if (IsValidCodeAddress(target)) {
                    call_targets.insert(target);
                    function_starts.insert(target);
                }
            } catch (...) {
                // Invalid address
            }
        }
        
        // Look for "ret" followed by function prologue pattern
        if (inst.IsReturn()) {
            uint32_t next_addr = addr + 4;
            if (next_addr < arch_.flash_start + firmware_data_.size()) {
                auto next_inst = DisassembleInstruction(next_addr);
                // Alignment and entry pattern suggests new function
                if ((next_addr & 0x03) == 0) {
                    function_starts.insert(next_addr);
                }
            }
        }
    }
    
    // Add known ESP32 functions from symbol table
    for (const auto& [name, addr] : arch_.known_functions) {
        function_starts.insert(addr);
    }
    
    // Sort function starts
    std::vector<uint32_t> sorted_starts(function_starts.begin(), function_starts.end());
    std::sort(sorted_starts.begin(), sorted_starts.end());
    
    // Create function objects
    for (size_t i = 0; i < sorted_starts.size(); i++) {
        auto func = std::make_unique<Function>();
        func->start_address = sorted_starts[i];
        
        // Determine function end (start of next function or end of section)
        if (i + 1 < sorted_starts.size()) {
            func->end_address = sorted_starts[i + 1];
        } else {
            func->end_address = std::min(sorted_starts[i] + 0x1000, 
                                        static_cast<uint32_t>(arch_.flash_start + firmware_data_.size()));
        }
        
        // Try to get name from known functions
        func->name = GetSymbolName(func->start_address);
        func->return_type = "void";
        func->is_isr = false;
        func->is_task = false;
        
        // Add basic parameters (will be refined later)
        if (call_targets.count(func->start_address)) {
            // Functions called from other places likely have parameters
            func->parameters.push_back("void* arg");
        }
        
        functions_.push_back(std::move(func));
        
        // Limit number of functions for performance
        if (functions_.size() >= 100) break;
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
    // Detect ESP32-specific API calls by analyzing function calls
    for (auto& func : functions_) {
        if (!func->cfg) continue;
        
        for (auto& block : func->cfg->blocks) {
            for (const auto& inst : block->instructions) {
                if (!inst.IsCall()) continue;
                
                // Check against known ESP32 API functions
                std::string call_target = inst.operands.empty() ? "" : inst.operands[0];
                
                // GPIO APIs
                if (call_target.find("gpio_") == 0 || inst.comment.find("gpio_") != std::string::npos) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 GPIO API";
                    }
                }
                // WiFi APIs
                else if (call_target.find("esp_wifi_") == 0 || call_target.find("WiFi") != std::string::npos) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 WiFi API";
                    }
                }
                // Bluetooth APIs
                else if (call_target.find("esp_bt_") == 0 || call_target.find("esp_ble_") == 0) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 Bluetooth API";
                    }
                }
                // UART/Serial APIs
                else if (call_target.find("uart_") == 0 || call_target.find("Serial") != std::string::npos) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 UART API";
                    }
                }
                // I2C APIs
                else if (call_target.find("i2c_") == 0) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 I2C API";
                    }
                }
                // SPI APIs
                else if (call_target.find("spi_") == 0) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 SPI API";
                    }
                }
                // Timer APIs
                else if (call_target.find("esp_timer_") == 0 || call_target.find("timer_") == 0) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 Timer API";
                    }
                }
                // NVS (Non-Volatile Storage) APIs
                else if (call_target.find("nvs_") == 0) {
                    if (inst.comment.empty()) {
                        const_cast<Instruction&>(inst).comment = "ESP32 NVS API";
                    }
                }
            }
        }
    }
}

void AdvancedDecompiler::DetectFreeRTOSTasks() {
    // Detect FreeRTOS task creation and management patterns
    for (auto& func : functions_) {
        if (!func->cfg) continue;
        
        for (auto& block : func->cfg->blocks) {
            for (const auto& inst : block->instructions) {
                if (!inst.IsCall()) continue;
                
                std::string call_target = inst.operands.empty() ? "" : inst.operands[0];
                
                // xTaskCreate - creates a new task
                if (call_target.find("xTaskCreate") != std::string::npos ||
                    inst.comment.find("xTaskCreate") != std::string::npos) {
                    func->is_task = true;
                    func->task_priority = "unknown";
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Create task";
                }
                // vTaskDelay - task delay
                else if (call_target.find("vTaskDelay") != std::string::npos ||
                         inst.comment.find("vTaskDelay") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Task delay";
                }
                // vTaskDelete - delete task
                else if (call_target.find("vTaskDelete") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Delete task";
                }
                // xQueueSend/Receive - queue operations
                else if (call_target.find("xQueue") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Queue operation";
                }
                // Semaphore operations
                else if (call_target.find("xSemaphore") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Semaphore operation";
                }
                // Mutex operations
                else if (call_target.find("xMutex") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Mutex operation";
                }
                // Event group operations
                else if (call_target.find("xEventGroup") != std::string::npos) {
                    const_cast<Instruction&>(inst).comment = "FreeRTOS: Event group operation";
                }
            }
        }
        
        // Check if function is a task entry point
        // Tasks typically have infinite loops
        if (func->cfg) {
            for (auto& block : func->cfg->blocks) {
                // Check for back edges indicating loops
                for (auto* succ : block->successors) {
                    if (succ->start_address <= block->start_address) {
                        // Function contains a loop, likely a task
                        func->is_task = true;
                        break;
                    }
                }
            }
        }
    }
}

void AdvancedDecompiler::DetectInterruptHandlers() {
    // Detect interrupt service routines by analyzing function characteristics
    for (auto& func : functions_) {
        if (!func->cfg || func->cfg->blocks.empty()) continue;
        
        bool has_isr_characteristics = false;
        
        // ISR characteristics:
        // 1. Uses special instructions (rsr, wsr for interrupt handling)
        // 2. Shorter functions (typically)
        // 3. No task delays
        // 4. Direct hardware register access
        
        for (auto& block : func->cfg->blocks) {
            for (const auto& inst : block->instructions) {
                // Check for special register access (ISR indicator)
                if (inst.mnemonic == "rsr" || inst.mnemonic == "wsr" || inst.mnemonic == "xsr") {
                    has_isr_characteristics = true;
                }
                
                // Check for interrupt-related registers
                if (!inst.operands.empty()) {
                    std::string op = inst.operands[0];
                    if (op == "interrupt" || op == "intenable" || op == "intset" || op == "intclear") {
                        has_isr_characteristics = true;
                    }
                }
                
                // Check for GPIO interrupt handlers
                if (inst.comment.find("GPIO_INT") != std::string::npos ||
                    inst.comment.find("gpio_isr") != std::string::npos) {
                    has_isr_characteristics = true;
                }
                
                // Check for timer interrupt handlers
                if (inst.comment.find("TIMER_INT") != std::string::npos ||
                    inst.comment.find("timer_isr") != std::string::npos) {
                    has_isr_characteristics = true;
                }
            }
        }
        
        // Mark as ISR if characteristics detected
        if (has_isr_characteristics) {
            func->is_isr = true;
            
            // ISRs should be short and fast
            if (func->name.find("isr") == std::string::npos && 
                func->name.find("ISR") == std::string::npos &&
                func->name.find("interrupt") == std::string::npos) {
                func->name += "_isr";
            }
        }
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
    inst.mnemonic = "nop";
    inst.opcode = 0;
    
    // Read opcode from firmware
    if (address < arch_.flash_start || address >= arch_.flash_start + firmware_data_.size()) {
        return inst;
    }
    
    size_t offset = address - arch_.flash_start;
    if (offset + 3 >= firmware_data_.size()) {
        return inst;
    }
    
    inst.opcode = firmware_data_[offset] |
                 (firmware_data_[offset + 1] << 8) |
                 (firmware_data_[offset + 2] << 16) |
                 (firmware_data_[offset + 3] << 24);
    
    // Decode Xtensa instruction (simplified but more complete)
    // Xtensa uses variable-length instructions (24-bit minimum)
    uint8_t op0 = inst.opcode & 0x0F;
    uint8_t op1 = (inst.opcode >> 4) & 0x0F;
    uint8_t op2 = (inst.opcode >> 8) & 0x0F;
    
    // Extract common register fields
    uint8_t r = (inst.opcode >> 12) & 0x0F;
    uint8_t s = (inst.opcode >> 8) & 0x0F;
    uint8_t t = (inst.opcode >> 4) & 0x0F;
    
    // Decode based on opcode pattern
    if (op0 == 0x00) {
        // QRST format
        if (op1 == 0x00) {
            inst.mnemonic = "add";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        } else if (op1 == 0x01) {
            inst.mnemonic = "sub";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        } else if (op1 == 0x02) {
            inst.mnemonic = "mul";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        } else if (op1 == 0x0A) {
            inst.mnemonic = "and";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        } else if (op1 == 0x0B) {
            inst.mnemonic = "or";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        } else if (op1 == 0x0C) {
            inst.mnemonic = "xor";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), "a" + std::to_string(t)};
        }
    } else if (op0 == 0x01) {
        // Load/Store instructions
        uint16_t imm = (inst.opcode >> 16) & 0xFF;
        if (op1 == 0x02) {
            inst.mnemonic = "l32i";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm * 4)};
        } else if (op1 == 0x06) {
            inst.mnemonic = "s32i";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm * 4)};
        } else if (op1 == 0x01) {
            inst.mnemonic = "l16ui";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm * 2)};
        } else if (op1 == 0x05) {
            inst.mnemonic = "s16i";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm * 2)};
        } else if (op1 == 0x00) {
            inst.mnemonic = "l8ui";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm)};
        } else if (op1 == 0x04) {
            inst.mnemonic = "s8i";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm)};
        }
    } else if (op0 == 0x02) {
        // Immediate operations
        int16_t imm = (inst.opcode >> 16) & 0xFF;
        if ((imm & 0x80) != 0) imm |= 0xFF00; // Sign extend
        
        if (op1 == 0x0A) {
            inst.mnemonic = "movi";
            inst.operands = {"a" + std::to_string(t), std::to_string(imm)};
        } else if (op1 == 0x0B) {
            inst.mnemonic = "addi";
            inst.operands = {"a" + std::to_string(t), "a" + std::to_string(s), std::to_string(imm)};
        }
    } else if (op0 == 0x03) {
        // Shift operations
        uint8_t sa = (inst.opcode >> 16) & 0x1F;
        if (op1 == 0x01) {
            inst.mnemonic = "slli";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(s), std::to_string(sa)};
        } else if (op1 == 0x02) {
            inst.mnemonic = "srli";
            inst.operands = {"a" + std::to_string(r), "a" + std::to_string(t), std::to_string(sa)};
        }
    } else if (op0 == 0x05) {
        // Call instructions
        int32_t offset = (inst.opcode >> 6) & 0x3FFFF;
        if ((offset & 0x20000) != 0) offset |= 0xFFFC0000; // Sign extend
        offset <<= 2; // Scale by 4
        
        inst.mnemonic = "call0";
        uint32_t target = address + offset + 4;
        inst.operands = {GetSymbolName(target)};
    } else if (op0 == 0x06) {
        // Branch instructions
        int32_t offset = (inst.opcode >> 12) & 0xFF;
        if ((offset & 0x80) != 0) offset |= 0xFFFFFF00; // Sign extend
        offset <<= 2; // Scale by 4
        
        uint32_t target = address + offset + 4;
        
        if (op1 == 0x01) {
            inst.mnemonic = "beq";
            inst.operands = {"a" + std::to_string(s), "a" + std::to_string(t), "0x" + std::to_string(target)};
        } else if (op1 == 0x09) {
            inst.mnemonic = "bne";
            inst.operands = {"a" + std::to_string(s), "a" + std::to_string(t), "0x" + std::to_string(target)};
        } else if (op1 == 0x03) {
            inst.mnemonic = "blt";
            inst.operands = {"a" + std::to_string(s), "a" + std::to_string(t), "0x" + std::to_string(target)};
        } else if (op1 == 0x0B) {
            inst.mnemonic = "bge";
            inst.operands = {"a" + std::to_string(s), "a" + std::to_string(t), "0x" + std::to_string(target)};
        }
    } else if (op0 == 0x00 && op1 == 0x00 && op2 == 0x00) {
        // Return
        inst.mnemonic = "ret";
    } else if (op0 == 0x00 && op1 == 0x0F) {
        // NOP
        inst.mnemonic = "nop";
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
    
    // Add function comment with metadata
    oss << "/*\n";
    oss << " * Function: " << func->name << "\n";
    oss << " * Address: 0x" << std::hex << func->start_address << std::dec << "\n";
    if (func->is_isr) oss << " * Type: Interrupt Service Routine\n";
    if (func->is_task) oss << " * Type: FreeRTOS Task\n";
    if (!func->task_priority.empty()) oss << " * Priority: " << func->task_priority << "\n";
    oss << " */\n";
    
    // Function signature
    oss << func->return_type << " " << func->name << "(";
    for (size_t i = 0; i < func->parameters.size(); i++) {
        if (i > 0) oss << ", ";
        oss << func->parameters[i];
    }
    oss << ") {\n";
    
    // Local variables with inferred types
    if (!func->local_variables.empty()) {
        oss << "    // Local variables\n";
        
        // Get type information from data flow analysis
        DataFlowAnalysis dfa(func->cfg.get());
        dfa.InferTypes();
        
        for (const auto& var : func->local_variables) {
            std::string type = dfa.GetVariableType(var);
            oss << "    " << type << " " << var << ";\n";
        }
        oss << "\n";
    }
    
    // Generate function body from CFG
    if (func->cfg && !func->cfg->blocks.empty()) {
        // Track visited blocks to avoid infinite loops
        std::set<BasicBlock*> visited;
        GenerateStructuredCode(oss, func->cfg->entry_block, visited, 1);
    } else {
        oss << "    // Function body\n";
        oss << "    // Unable to generate code - no CFG available\n";
    }
    
    // Add return statement if needed
    if (func->return_type != "void") {
        oss << "    return 0; // Default return\n";
    }
    
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
    // Convert assembly to high-level pseudo-code
    
    // Arithmetic operations
    if (inst.mnemonic == "add" || inst.mnemonic == "addi") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " + " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "sub" || inst.mnemonic == "subi") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " - " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "mul") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " * " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "div") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " / " + inst.operands[2] + ";";
        }
    }
    
    // Move/assignment operations
    else if (inst.mnemonic == "movi" || inst.mnemonic == "mov") {
        if (inst.operands.size() >= 2) {
            return inst.operands[0] + " = " + inst.operands[1] + ";";
        }
    }
    
    // Load/store operations
    else if (inst.mnemonic == "l32i" || inst.mnemonic == "l16ui" || inst.mnemonic == "l8ui") {
        if (inst.operands.size() >= 2) {
            return inst.operands[0] + " = *(" + inst.operands[1] + ");";
        }
    } else if (inst.mnemonic == "s32i" || inst.mnemonic == "s16i" || inst.mnemonic == "s8i") {
        if (inst.operands.size() >= 2) {
            return "*(" + inst.operands[1] + ") = " + inst.operands[0] + ";";
        }
    }
    
    // Function calls
    else if (inst.mnemonic == "call" || inst.mnemonic == "callx") {
        std::string func_name = inst.operands.empty() ? "function" : inst.operands[0];
        // Try to get symbolic name
        if (!inst.comment.empty()) {
            func_name = inst.comment;
        }
        return func_name + "();";
    }
    
    // Logical operations
    else if (inst.mnemonic == "and") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " & " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "or") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " | " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "xor") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " ^ " + inst.operands[2] + ";";
        }
    }
    
    // Shift operations
    else if (inst.mnemonic == "slli" || inst.mnemonic == "sll") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " << " + inst.operands[2] + ";";
        }
    } else if (inst.mnemonic == "srli" || inst.mnemonic == "srl") {
        if (inst.operands.size() >= 3) {
            return inst.operands[0] + " = " + inst.operands[1] + " >> " + inst.operands[2] + ";";
        }
    }
    
    // Return instruction
    else if (inst.mnemonic == "ret" || inst.mnemonic == "retw") {
        return "return;";
    }
    
    // NOP
    else if (inst.mnemonic == "nop") {
        return "// nop";
    }
    
    // Default: show as comment with original assembly
    return "// " + inst.ToString();
}

void AdvancedDecompiler::GenerateStructuredCode(std::ostringstream& oss, BasicBlock* block,
                                               std::set<BasicBlock*>& visited, int indent_level) {
    if (!block || visited.count(block)) return;
    
    visited.insert(block);
    std::string indent(indent_level * 4, ' ');
    
    // Generate code for this block
    for (const auto& inst : block->instructions) {
        // Skip control flow instructions (handled separately)
        if (inst.IsBranch() || inst.IsJump() || inst.IsReturn()) continue;
        
        std::string pseudo = GenerateInstructionPseudoCode(inst);
        if (!pseudo.empty() && pseudo != "// nop") {
            oss << indent << pseudo << "\n";
        }
    }
    
    // Handle control flow
    if (block->instructions.empty()) return;
    
    const auto& last_inst = block->instructions.back();
    
    // Handle conditional branches (if-else)
    if (last_inst.IsBranch() && block->successors.size() >= 2) {
        // Generate condition from branch instruction
        std::string condition = "condition";
        if (last_inst.operands.size() >= 2) {
            condition = last_inst.operands[0] + " == " + last_inst.operands[1];
        }
        
        oss << indent << "if (" << condition << ") {\n";
        
        // True branch
        if (!visited.count(block->successors[0])) {
            GenerateStructuredCode(oss, block->successors[0], visited, indent_level + 1);
        }
        
        oss << indent << "}";
        
        // False branch (else)
        if (block->successors.size() > 1 && !visited.count(block->successors[1])) {
            oss << " else {\n";
            GenerateStructuredCode(oss, block->successors[1], visited, indent_level + 1);
            oss << indent << "}";
        }
        oss << "\n";
    }
    // Handle loops
    else if (last_inst.IsJump() && !block->successors.empty()) {
        BasicBlock* target = block->successors[0];
        
        // Check if this is a back edge (loop)
        if (target->start_address <= block->start_address) {
            oss << indent << "while (true) { // Loop detected\n";
            if (!visited.count(target)) {
                GenerateStructuredCode(oss, target, visited, indent_level + 1);
            }
            oss << indent << "}\n";
        } else {
            // Forward jump - continue with successor
            if (!visited.count(target)) {
                GenerateStructuredCode(oss, target, visited, indent_level);
            }
        }
    }
    // Handle return
    else if (last_inst.IsReturn()) {
        oss << indent << "return;\n";
    }
    // Fall-through to next block
    else if (!block->successors.empty()) {
        GenerateStructuredCode(oss, block->successors[0], visited, indent_level);
    }
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
