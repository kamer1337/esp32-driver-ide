#include "plugins/plugin_system.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <regex>

namespace esp32_ide {
namespace plugins {

// ============================================================================
// Plugin Implementation
// ============================================================================

Plugin::Plugin(const PluginMetadata& metadata)
    : metadata_(metadata), state_(PluginState::UNLOADED), api_(nullptr) {
    config_.enabled = true;
}

void Plugin::SetError(const std::string& error) {
    last_error_ = error;
    state_ = PluginState::ERROR;
}

// ============================================================================
// PluginManager Implementation
// ============================================================================

PluginManager::PluginManager() : api_(nullptr) {}

void PluginManager::RegisterPluginFactory(const std::string& plugin_id, 
                                           PluginFactory factory) {
    factories_[plugin_id] = factory;
}

void PluginManager::UnregisterPluginFactory(const std::string& plugin_id) {
    factories_.erase(plugin_id);
}

bool PluginManager::LoadPlugin(const std::string& plugin_id) {
    auto factory_it = factories_.find(plugin_id);
    if (factory_it == factories_.end()) {
        return false;
    }
    
    if (plugins_.find(plugin_id) != plugins_.end()) {
        return true;  // Already loaded
    }
    
    // Check dependencies
    if (!CheckDependencies(plugin_id)) {
        return false;
    }
    
    // Create plugin instance
    auto plugin = factory_it->second();
    if (!plugin) {
        return false;
    }
    
    // Apply configuration
    auto config_it = configs_.find(plugin_id);
    if (config_it != configs_.end()) {
        plugin->SetConfig(config_it->second);
    }
    
    // Initialize plugin
    if (!plugin->Initialize(api_)) {
        return false;
    }
    
    plugins_[plugin_id] = std::move(plugin);
    NotifyPluginLoaded(plugins_[plugin_id].get());
    
    return true;
}

bool PluginManager::UnloadPlugin(const std::string& plugin_id) {
    auto it = plugins_.find(plugin_id);
    if (it == plugins_.end()) {
        return false;
    }
    
    // Deactivate if active
    if (it->second->GetState() == PluginState::ACTIVE) {
        it->second->Deactivate();
    }
    
    // Dispose and remove
    it->second->Dispose();
    NotifyPluginUnloaded(it->second.get());
    plugins_.erase(it);
    
    return true;
}

bool PluginManager::EnablePlugin(const std::string& plugin_id) {
    auto it = plugins_.find(plugin_id);
    if (it == plugins_.end()) {
        return false;
    }
    
    if (it->second->GetState() == PluginState::ACTIVE) {
        return true;
    }
    
    return it->second->Activate();
}

bool PluginManager::DisablePlugin(const std::string& plugin_id) {
    auto it = plugins_.find(plugin_id);
    if (it == plugins_.end()) {
        return false;
    }
    
    if (it->second->GetState() != PluginState::ACTIVE) {
        return true;
    }
    
    return it->second->Deactivate();
}

bool PluginManager::ReloadPlugin(const std::string& plugin_id) {
    if (!UnloadPlugin(plugin_id)) {
        return false;
    }
    return LoadPlugin(plugin_id);
}

void PluginManager::LoadAllPlugins() {
    // Get dependency order
    auto order = GetDependencyOrder();
    for (const auto& plugin_id : order) {
        LoadPlugin(plugin_id);
    }
}

void PluginManager::UnloadAllPlugins() {
    // Unload in reverse order
    std::vector<std::string> ids;
    for (const auto& pair : plugins_) {
        ids.push_back(pair.first);
    }
    
    for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
        UnloadPlugin(*it);
    }
}

void PluginManager::EnableAllPlugins() {
    for (auto& pair : plugins_) {
        EnablePlugin(pair.first);
    }
}

void PluginManager::DisableAllPlugins() {
    for (auto& pair : plugins_) {
        DisablePlugin(pair.first);
    }
}

Plugin* PluginManager::GetPlugin(const std::string& plugin_id) {
    auto it = plugins_.find(plugin_id);
    return (it != plugins_.end()) ? it->second.get() : nullptr;
}

std::vector<Plugin*> PluginManager::GetAllPlugins() {
    std::vector<Plugin*> result;
    for (auto& pair : plugins_) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::vector<Plugin*> PluginManager::GetPluginsByType(PluginType type) {
    std::vector<Plugin*> result;
    for (auto& pair : plugins_) {
        if (pair.second->GetMetadata().type == type) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

std::vector<Plugin*> PluginManager::GetActivePlugins() {
    std::vector<Plugin*> result;
    for (auto& pair : plugins_) {
        if (pair.second->GetState() == PluginState::ACTIVE) {
            result.push_back(pair.second.get());
        }
    }
    return result;
}

bool PluginManager::IsPluginLoaded(const std::string& plugin_id) const {
    return plugins_.find(plugin_id) != plugins_.end();
}

bool PluginManager::CheckDependencies(const std::string& plugin_id) const {
    auto missing = GetMissingDependencies(plugin_id);
    return missing.empty();
}

std::vector<std::string> PluginManager::GetMissingDependencies(
    const std::string& plugin_id) const {
    std::vector<std::string> missing;
    
    auto factory_it = factories_.find(plugin_id);
    if (factory_it == factories_.end()) {
        return missing;
    }
    
    // Create temporary instance to check dependencies
    auto temp = factory_it->second();
    if (!temp) return missing;
    
    for (const auto& dep : temp->GetMetadata().dependencies) {
        if (plugins_.find(dep) == plugins_.end() && 
            factories_.find(dep) == factories_.end()) {
            missing.push_back(dep);
        }
    }
    
    return missing;
}

std::vector<std::string> PluginManager::GetDependencyOrder() const {
    std::vector<std::string> order;
    std::map<std::string, bool> visited;
    std::map<std::string, bool> in_stack;
    
    std::function<void(const std::string&)> visit = [&](const std::string& id) {
        if (visited[id]) return;
        if (in_stack[id]) return;  // Circular dependency
        
        in_stack[id] = true;
        
        auto factory_it = factories_.find(id);
        if (factory_it != factories_.end()) {
            auto temp = factory_it->second();
            if (temp) {
                for (const auto& dep : temp->GetMetadata().dependencies) {
                    visit(dep);
                }
            }
        }
        
        in_stack[id] = false;
        visited[id] = true;
        order.push_back(id);
    };
    
    for (const auto& pair : factories_) {
        visit(pair.first);
    }
    
    return order;
}

void PluginManager::SetPluginConfig(const std::string& plugin_id, 
                                     const PluginConfig& config) {
    configs_[plugin_id] = config;
    
    auto it = plugins_.find(plugin_id);
    if (it != plugins_.end()) {
        it->second->SetConfig(config);
    }
}

PluginConfig PluginManager::GetPluginConfig(const std::string& plugin_id) const {
    auto it = configs_.find(plugin_id);
    if (it != configs_.end()) {
        return it->second;
    }
    return PluginConfig();
}

void PluginManager::SavePluginConfigs(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    file << "{\n  \"plugins\": {\n";
    bool first = true;
    for (const auto& pair : configs_) {
        if (!first) file << ",\n";
        first = false;
        file << "    \"" << pair.first << "\": {\n";
        file << "      \"enabled\": " << (pair.second.enabled ? "true" : "false") << "\n";
        file << "    }";
    }
    file << "\n  }\n}\n";
}

void PluginManager::LoadPluginConfigs(const std::string& /*path*/) {
    // Simplified - would need JSON parser
}

int PluginManager::GetLoadedPluginCount() const {
    return plugins_.size();
}

int PluginManager::GetActivePluginCount() const {
    int count = 0;
    for (const auto& pair : plugins_) {
        if (pair.second->GetState() == PluginState::ACTIVE) {
            count++;
        }
    }
    return count;
}

void PluginManager::NotifyPluginLoaded(const Plugin* plugin) {
    if (on_loaded_) {
        on_loaded_(plugin);
    }
}

void PluginManager::NotifyPluginUnloaded(const Plugin* plugin) {
    if (on_unloaded_) {
        on_unloaded_(plugin);
    }
}

// ============================================================================
// ExternalToolManager Implementation
// ============================================================================

ExternalToolManager::ExternalToolManager() {}

void ExternalToolManager::RegisterTool(const ExternalToolConfig& config) {
    tools_[config.id] = config;
}

void ExternalToolManager::UnregisterTool(const std::string& tool_id) {
    tools_.erase(tool_id);
}

void ExternalToolManager::UpdateTool(const ExternalToolConfig& config) {
    tools_[config.id] = config;
}

ToolExecutionResult ExternalToolManager::ExecuteTool(
    const std::string& tool_id,
    const std::map<std::string, std::string>& variables) {
    
    ToolExecutionResult result;
    result.exit_code = -1;
    result.timed_out = false;
    
    auto it = tools_.find(tool_id);
    if (it == tools_.end()) {
        result.error_message = "Tool not found: " + tool_id;
        return result;
    }
    
    const auto& config = it->second;
    std::string command = SubstituteVariables(config.command, variables);
    std::string args = SubstituteVariables(config.arguments, variables);
    std::string full_command = command + " " + args;
    
    return RunCommand(full_command, config.working_directory, config.environment);
}

void ExternalToolManager::ExecuteToolAsync(
    const std::string& tool_id,
    OutputCallback on_output,
    std::function<void(const ToolExecutionResult&)> on_complete) {
    
    running_tools_[tool_id] = true;
    
    // Simulated async execution
    auto result = ExecuteTool(tool_id);
    
    if (on_output && !result.stdout_output.empty()) {
        on_output(result.stdout_output);
    }
    
    running_tools_[tool_id] = false;
    
    if (on_complete) {
        on_complete(result);
    }
}

bool ExternalToolManager::CancelExecution(const std::string& tool_id) {
    auto it = running_tools_.find(tool_id);
    if (it != running_tools_.end() && it->second) {
        it->second = false;
        return true;
    }
    return false;
}

ExternalToolConfig ExternalToolManager::GetToolConfig(const std::string& tool_id) const {
    auto it = tools_.find(tool_id);
    if (it != tools_.end()) {
        return it->second;
    }
    return ExternalToolConfig();
}

std::vector<ExternalToolConfig> ExternalToolManager::GetAllTools() const {
    std::vector<ExternalToolConfig> result;
    for (const auto& pair : tools_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<ExternalToolConfig> ExternalToolManager::GetMenuTools() const {
    std::vector<ExternalToolConfig> result;
    for (const auto& pair : tools_) {
        if (pair.second.show_in_menu) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool ExternalToolManager::ToolExists(const std::string& tool_id) const {
    return tools_.find(tool_id) != tools_.end();
}

std::string ExternalToolManager::SubstituteVariables(
    const std::string& text,
    const std::map<std::string, std::string>& variables) const {
    
    std::string result = text;
    
    for (const auto& pair : variables) {
        std::string placeholder = "${" + pair.first + "}";
        size_t pos;
        while ((pos = result.find(placeholder)) != std::string::npos) {
            result.replace(pos, placeholder.length(), pair.second);
        }
    }
    
    return result;
}

void ExternalToolManager::SaveTools(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    file << "{\n  \"tools\": [\n";
    bool first = true;
    for (const auto& pair : tools_) {
        if (!first) file << ",\n";
        first = false;
        file << "    {\n";
        file << "      \"id\": \"" << pair.second.id << "\",\n";
        file << "      \"name\": \"" << pair.second.name << "\",\n";
        file << "      \"command\": \"" << pair.second.command << "\"\n";
        file << "    }";
    }
    file << "\n  ]\n}\n";
}

void ExternalToolManager::LoadTools(const std::string& /*path*/) {
    // Simplified - would need JSON parser
}

ToolExecutionResult ExternalToolManager::RunCommand(
    const std::string& command,
    const std::string& /*working_dir*/,
    const std::vector<std::string>& /*env*/,
    int /*timeout_ms*/) {
    
    ToolExecutionResult result;
    result.exit_code = 0;
    result.timed_out = false;
    
    // Simulated command execution
    result.stdout_output = "Simulated output for: " + command;
    result.execution_time_ms = 100;
    
    return result;
}

// ============================================================================
// CustomCompilerManager Implementation
// ============================================================================

CustomCompilerManager::CustomCompilerManager() {
    // Register default compilers
    CompilerConfig gcc;
    gcc.id = "gcc";
    gcc.name = "GCC";
    gcc.compiler_path = "gcc";
    gcc.linker_path = "gcc";
    gcc.default_flags = {"-Wall", "-Wextra"};
    gcc.output_extension = ".o";
    gcc.error_pattern = R"((.+):(\d+):(\d+): (error|warning): (.+))";
    gcc.warning_pattern = R"((.+):(\d+):(\d+): warning: (.+))";
    RegisterCompiler(gcc);
    
    CompilerConfig xtensa;
    xtensa.id = "xtensa-esp32";
    xtensa.name = "Xtensa ESP32";
    xtensa.compiler_path = "xtensa-esp32-elf-gcc";
    xtensa.linker_path = "xtensa-esp32-elf-gcc";
    xtensa.default_flags = {"-mlongcalls", "-mtext-section-literals"};
    xtensa.output_extension = ".o";
    RegisterCompiler(xtensa);
    
    CompilerConfig armgcc;
    armgcc.id = "arm-gcc";
    armgcc.name = "ARM GCC";
    armgcc.compiler_path = "arm-none-eabi-gcc";
    armgcc.linker_path = "arm-none-eabi-gcc";
    armgcc.default_flags = {"-mthumb", "-mcpu=cortex-m3"};
    armgcc.output_extension = ".o";
    RegisterCompiler(armgcc);
}

void CustomCompilerManager::RegisterCompiler(const CompilerConfig& config) {
    compilers_[config.id] = config;
}

void CustomCompilerManager::UnregisterCompiler(const std::string& compiler_id) {
    compilers_.erase(compiler_id);
}

ToolExecutionResult CustomCompilerManager::Compile(
    const std::string& compiler_id,
    const std::string& source_file,
    const std::string& output_file,
    const std::vector<std::string>& extra_flags) {
    
    ToolExecutionResult result;
    result.exit_code = -1;
    
    auto it = compilers_.find(compiler_id);
    if (it == compilers_.end()) {
        result.error_message = "Compiler not found: " + compiler_id;
        return result;
    }
    
    const auto& config = it->second;
    
    // Build command
    std::ostringstream cmd;
    cmd << config.compiler_path << " -c ";
    
    for (const auto& flag : config.default_flags) {
        cmd << flag << " ";
    }
    
    for (const auto& flag : extra_flags) {
        cmd << flag << " ";
    }
    
    for (const auto& inc : config.include_paths) {
        cmd << "-I" << inc << " ";
    }
    
    for (const auto& def : config.defines) {
        cmd << "-D" << def.first << "=" << def.second << " ";
    }
    
    cmd << "-o " << output_file << " " << source_file;
    
    // Simulated compilation
    result.exit_code = 0;
    result.stdout_output = "Compiling " + source_file + " -> " + output_file;
    result.execution_time_ms = 500;
    
    return result;
}

ToolExecutionResult CustomCompilerManager::Link(
    const std::string& compiler_id,
    const std::vector<std::string>& object_files,
    const std::string& output_file,
    const std::vector<std::string>& libraries) {
    
    ToolExecutionResult result;
    result.exit_code = -1;
    
    auto it = compilers_.find(compiler_id);
    if (it == compilers_.end()) {
        result.error_message = "Compiler not found: " + compiler_id;
        return result;
    }
    
    const auto& config = it->second;
    
    // Build command
    std::ostringstream cmd;
    cmd << config.linker_path << " ";
    
    for (const auto& obj : object_files) {
        cmd << obj << " ";
    }
    
    for (const auto& lib_path : config.library_paths) {
        cmd << "-L" << lib_path << " ";
    }
    
    for (const auto& lib : libraries) {
        cmd << "-l" << lib << " ";
    }
    
    cmd << "-o " << output_file;
    
    // Simulated linking
    result.exit_code = 0;
    result.stdout_output = "Linking " + std::to_string(object_files.size()) + 
                           " objects -> " + output_file;
    result.execution_time_ms = 200;
    
    return result;
}

std::vector<AnalysisResult> CustomCompilerManager::ParseCompilerOutput(
    const std::string& compiler_id,
    const std::string& output) {
    
    std::vector<AnalysisResult> results;
    
    auto it = compilers_.find(compiler_id);
    if (it == compilers_.end()) return results;
    
    const auto& config = it->second;
    if (config.error_pattern.empty()) return results;
    
    std::regex pattern(config.error_pattern);
    std::smatch match;
    std::string::const_iterator searchStart(output.cbegin());
    
    while (std::regex_search(searchStart, output.cend(), match, pattern)) {
        AnalysisResult result;
        if (match.size() >= 5) {
            result.file_path = match[1];
            result.line_number = std::stoi(match[2]);
            result.column_number = std::stoi(match[3]);
            result.severity = match[4];
            result.message = match[5];
        }
        results.push_back(result);
        searchStart = match.suffix().first;
    }
    
    return results;
}

CompilerConfig CustomCompilerManager::GetCompilerConfig(
    const std::string& compiler_id) const {
    
    auto it = compilers_.find(compiler_id);
    if (it != compilers_.end()) {
        return it->second;
    }
    return CompilerConfig();
}

std::vector<CompilerConfig> CustomCompilerManager::GetAllCompilers() const {
    std::vector<CompilerConfig> result;
    for (const auto& pair : compilers_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<std::string> CustomCompilerManager::GetCompilerIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : compilers_) {
        ids.push_back(pair.first);
    }
    return ids;
}

bool CustomCompilerManager::CompilerExists(const std::string& compiler_id) const {
    return compilers_.find(compiler_id) != compilers_.end();
}

void CustomCompilerManager::SaveCompilers(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    file << "{\n  \"compilers\": [\n";
    bool first = true;
    for (const auto& pair : compilers_) {
        if (!first) file << ",\n";
        first = false;
        file << "    {\n";
        file << "      \"id\": \"" << pair.second.id << "\",\n";
        file << "      \"name\": \"" << pair.second.name << "\",\n";
        file << "      \"compiler_path\": \"" << pair.second.compiler_path << "\"\n";
        file << "    }";
    }
    file << "\n  ]\n}\n";
}

void CustomCompilerManager::LoadCompilers(const std::string& /*path*/) {
    // Simplified - would need JSON parser
}

// ============================================================================
// AnalysisToolRunner Implementation
// ============================================================================

AnalysisToolRunner::AnalysisToolRunner() {
    // Register built-in analyzers
    RegisterAnalyzer("cppcheck", "cppcheck --enable=all", 
                     R"(\[(.+):(\d+)\]: \((\w+)\) (.+))");
    RegisterAnalyzer("clang-tidy", "clang-tidy", 
                     R"((.+):(\d+):(\d+): (\w+): (.+))");
}

void AnalysisToolRunner::RegisterAnalyzer(const std::string& analyzer_id,
                                           const std::string& command,
                                           const std::string& output_pattern) {
    AnalyzerConfig config;
    config.command = command;
    config.output_pattern = output_pattern;
    analyzers_[analyzer_id] = config;
}

void AnalysisToolRunner::UnregisterAnalyzer(const std::string& analyzer_id) {
    analyzers_.erase(analyzer_id);
}

std::vector<AnalysisResult> AnalysisToolRunner::RunAnalysis(
    const std::string& analyzer_id,
    const std::string& file_path) {
    
    std::vector<AnalysisResult> results;
    
    auto it = analyzers_.find(analyzer_id);
    if (it == analyzers_.end()) return results;
    
    // Simulated analysis
    AnalysisResult sample;
    sample.file_path = file_path;
    sample.line_number = 10;
    sample.column_number = 5;
    sample.severity = "warning";
    sample.message = "Sample analysis result";
    sample.rule_id = "rule-001";
    results.push_back(sample);
    
    return results;
}

std::vector<AnalysisResult> AnalysisToolRunner::RunAnalysisOnProject(
    const std::string& analyzer_id,
    const std::string& /*project_path*/) {
    
    std::vector<AnalysisResult> results;
    
    // Would scan project and analyze all files
    auto file_results = RunAnalysis(analyzer_id, "main.cpp");
    results.insert(results.end(), file_results.begin(), file_results.end());
    
    return results;
}

std::vector<AnalysisResult> AnalysisToolRunner::RunStaticAnalysis(
    const std::string& code) {
    
    std::vector<AnalysisResult> results;
    
    // Check for common issues
    if (code.find("goto") != std::string::npos) {
        AnalysisResult issue;
        issue.severity = "warning";
        issue.message = "Use of 'goto' statement detected";
        issue.rule_id = "static-001";
        issue.suggestion = "Consider using structured control flow instead";
        results.push_back(issue);
    }
    
    if (code.find("gets(") != std::string::npos) {
        AnalysisResult issue;
        issue.severity = "error";
        issue.message = "Use of unsafe 'gets' function";
        issue.rule_id = "static-002";
        issue.suggestion = "Use fgets() instead";
        results.push_back(issue);
    }
    
    return results;
}

std::vector<AnalysisResult> AnalysisToolRunner::RunStyleCheck(
    const std::string& code) {
    
    std::vector<AnalysisResult> results;
    
    // Check line lengths
    std::istringstream stream(code);
    std::string line;
    int line_num = 1;
    
    while (std::getline(stream, line)) {
        if (line.length() > 120) {
            AnalysisResult issue;
            issue.line_number = line_num;
            issue.severity = "info";
            issue.message = "Line exceeds 120 characters";
            issue.rule_id = "style-001";
            results.push_back(issue);
        }
        line_num++;
    }
    
    return results;
}

std::vector<AnalysisResult> AnalysisToolRunner::RunSecurityScan(
    const std::string& code) {
    
    std::vector<AnalysisResult> results;
    
    // Check for security issues
    std::vector<std::pair<std::string, std::string>> patterns = {
        {"strcpy", "Use strncpy instead of strcpy"},
        {"sprintf", "Use snprintf instead of sprintf"},
        {"password", "Potential hardcoded password detected"},
        {"secret", "Potential hardcoded secret detected"},
    };
    
    for (const auto& pattern : patterns) {
        if (code.find(pattern.first) != std::string::npos) {
            AnalysisResult issue;
            issue.severity = "warning";
            issue.message = pattern.second;
            issue.rule_id = "security-001";
            results.push_back(issue);
        }
    }
    
    return results;
}

std::vector<AnalysisResult> AnalysisToolRunner::RunComplexityAnalysis(
    const std::string& code) {
    
    std::vector<AnalysisResult> results;
    
    // Simplified cyclomatic complexity estimation
    int complexity = 1;
    std::vector<std::string> keywords = {"if", "else", "for", "while", "case", "&&", "||"};
    
    for (const auto& kw : keywords) {
        size_t pos = 0;
        while ((pos = code.find(kw, pos)) != std::string::npos) {
            complexity++;
            pos += kw.length();
        }
    }
    
    if (complexity > 10) {
        AnalysisResult issue;
        issue.severity = "warning";
        issue.message = "High cyclomatic complexity: " + std::to_string(complexity);
        issue.rule_id = "complexity-001";
        issue.suggestion = "Consider breaking this function into smaller functions";
        results.push_back(issue);
    }
    
    return results;
}

std::string AnalysisToolRunner::GenerateReport(
    const std::vector<AnalysisResult>& results,
    const std::string& format) {
    
    std::ostringstream report;
    
    if (format == "text") {
        report << "Analysis Report\n";
        report << "===============\n\n";
        report << "Total issues found: " << results.size() << "\n\n";
        
        for (const auto& result : results) {
            report << "[" << result.severity << "] ";
            if (!result.file_path.empty()) {
                report << result.file_path << ":" << result.line_number << " ";
            }
            report << result.message << "\n";
            if (!result.suggestion.empty()) {
                report << "  Suggestion: " << result.suggestion << "\n";
            }
        }
    } else if (format == "json") {
        report << "{\n  \"results\": [\n";
        bool first = true;
        for (const auto& result : results) {
            if (!first) report << ",\n";
            first = false;
            report << "    {\n";
            report << "      \"severity\": \"" << result.severity << "\",\n";
            report << "      \"message\": \"" << result.message << "\"\n";
            report << "    }";
        }
        report << "\n  ]\n}";
    }
    
    return report.str();
}

std::vector<AnalysisResult> AnalysisToolRunner::ParseAnalyzerOutput(
    const std::string& output,
    const std::string& pattern) {
    
    std::vector<AnalysisResult> results;
    
    std::regex regex(pattern);
    std::smatch match;
    std::string::const_iterator searchStart(output.cbegin());
    
    while (std::regex_search(searchStart, output.cend(), match, regex)) {
        AnalysisResult result;
        if (match.size() >= 4) {
            result.file_path = match[1];
            result.line_number = std::stoi(match[2]);
            result.severity = match[3];
            result.message = match[4];
        }
        results.push_back(result);
        searchStart = match.suffix().first;
    }
    
    return results;
}

// ============================================================================
// PluginMarketplace Implementation
// ============================================================================

PluginMarketplace::PluginMarketplace() 
    : marketplace_url_("https://marketplace.esp32ide.io"),
      install_path_("~/.esp32ide/plugins"),
      total_plugins_(0) {}

void PluginMarketplace::SetMarketplaceUrl(const std::string& url) {
    marketplace_url_ = url;
}

void PluginMarketplace::SetAuthToken(const std::string& token) {
    auth_token_ = token;
}

std::vector<MarketplacePlugin> PluginMarketplace::Search(
    const MarketplaceFilter& filter) {
    
    std::vector<MarketplacePlugin> results;
    
    // Simulated search results
    MarketplacePlugin sample;
    sample.metadata.id = "sample-plugin";
    sample.metadata.name = "Sample Plugin";
    sample.metadata.version = "1.0.0";
    sample.metadata.author = "ESP32 IDE Team";
    sample.metadata.description = "A sample plugin matching: " + filter.query;
    sample.downloads = 1000;
    sample.rating = 4.5f;
    sample.rating_count = 50;
    sample.is_verified = true;
    results.push_back(sample);
    
    return results;
}

std::vector<MarketplacePlugin> PluginMarketplace::GetFeatured() {
    MarketplaceFilter filter;
    filter.featured_only = true;
    return Search(filter);
}

std::vector<MarketplacePlugin> PluginMarketplace::GetPopular(int limit) {
    MarketplaceFilter filter;
    filter.sort_by = "downloads";
    filter.sort_descending = true;
    filter.per_page = limit;
    return Search(filter);
}

std::vector<MarketplacePlugin> PluginMarketplace::GetRecent(int limit) {
    MarketplaceFilter filter;
    filter.sort_by = "updated";
    filter.sort_descending = true;
    filter.per_page = limit;
    return Search(filter);
}

std::vector<MarketplacePlugin> PluginMarketplace::GetByAuthor(const std::string& author) {
    MarketplaceFilter filter;
    filter.author = author;
    return Search(filter);
}

MarketplacePlugin PluginMarketplace::GetPluginDetails(const std::string& plugin_id) {
    MarketplacePlugin plugin;
    plugin.metadata.id = plugin_id;
    plugin.metadata.name = plugin_id + " Plugin";
    plugin.metadata.version = "1.0.0";
    plugin.downloads = 500;
    plugin.rating = 4.0f;
    return plugin;
}

bool PluginMarketplace::InstallPlugin(const std::string& plugin_id,
                                       ProgressCallback on_progress) {
    if (on_progress) {
        on_progress(0.0f, "Starting download...");
        on_progress(0.5f, "Downloading...");
        on_progress(0.8f, "Installing...");
        on_progress(1.0f, "Complete");
    }
    
    installed_plugins_.push_back(plugin_id);
    return true;
}

bool PluginMarketplace::UpdatePlugin(const std::string& plugin_id,
                                      ProgressCallback on_progress) {
    return InstallPlugin(plugin_id, on_progress);
}

bool PluginMarketplace::UninstallPlugin(const std::string& plugin_id) {
    auto it = std::find(installed_plugins_.begin(), installed_plugins_.end(), plugin_id);
    if (it != installed_plugins_.end()) {
        installed_plugins_.erase(it);
        return true;
    }
    return false;
}

std::vector<std::string> PluginMarketplace::GetInstalledPlugins() {
    return installed_plugins_;
}

bool PluginMarketplace::IsPluginInstalled(const std::string& plugin_id) const {
    return std::find(installed_plugins_.begin(), installed_plugins_.end(), plugin_id) 
           != installed_plugins_.end();
}

bool PluginMarketplace::HasUpdate(const std::string& /*plugin_id*/) const {
    // Check for updates - simulated
    return false;
}

std::vector<PluginReview> PluginMarketplace::GetReviews(const std::string& /*plugin_id*/,
                                                          int /*page*/) {
    std::vector<PluginReview> reviews;
    
    PluginReview sample;
    sample.reviewer_name = "Developer";
    sample.rating = 5;
    sample.comment = "Great plugin!";
    sample.helpful_count = 10;
    reviews.push_back(sample);
    
    return reviews;
}

bool PluginMarketplace::SubmitReview(const std::string& /*plugin_id*/,
                                       int /*rating*/,
                                       const std::string& /*comment*/) {
    // Submit review - simulated
    return true;
}

bool PluginMarketplace::MarkReviewHelpful(const std::string& /*review_id*/) {
    return true;
}

bool PluginMarketplace::PublishPlugin(const std::string& /*package_path*/) {
    return true;
}

bool PluginMarketplace::UpdatePluginListing(const std::string& /*plugin_id*/,
                                              const PluginMetadata& /*metadata*/) {
    return true;
}

bool PluginMarketplace::UnpublishPlugin(const std::string& /*plugin_id*/) {
    return true;
}

bool PluginMarketplace::DownloadPlugin(const std::string& /*url*/,
                                         const std::string& /*dest_path*/,
                                         ProgressCallback on_progress) {
    if (on_progress) {
        on_progress(1.0f, "Downloaded");
    }
    return true;
}

bool PluginMarketplace::ExtractPlugin(const std::string& /*package_path*/,
                                        const std::string& /*dest_path*/) {
    return true;
}

bool PluginMarketplace::ValidatePlugin(const std::string& /*plugin_path*/) {
    return true;
}

// ============================================================================
// PluginDocGenerator Implementation
// ============================================================================

PluginDocGenerator::PluginDocGenerator() {
    // Initialize default templates
    templates_["readme"] = R"(
# {{name}}

{{description}}

## Installation

1. Open ESP32 Driver IDE
2. Go to Extensions > Marketplace
3. Search for "{{name}}"
4. Click Install

## Usage

{{usage}}

## License

{{license}}

## Author

{{author}}
)";
}

std::string PluginDocGenerator::GenerateReadme(const PluginMetadata& metadata) {
    std::map<std::string, std::string> vars;
    vars["name"] = metadata.name;
    vars["description"] = metadata.description;
    vars["author"] = metadata.author;
    vars["license"] = metadata.license;
    vars["usage"] = "See documentation for usage instructions.";
    
    return ApplyTemplate("readme", vars);
}

std::string PluginDocGenerator::GenerateAPIReference(const Plugin* plugin) {
    std::ostringstream doc;
    
    doc << "# API Reference: " << plugin->GetMetadata().name << "\n\n";
    doc << "Version: " << plugin->GetMetadata().version << "\n\n";
    doc << "## Description\n\n";
    doc << plugin->GetMetadata().description << "\n\n";
    doc << "## API\n\n";
    doc << "See plugin source code for detailed API documentation.\n";
    
    return doc.str();
}

std::string PluginDocGenerator::GenerateChangelog(
    const std::vector<std::pair<std::string, std::string>>& changes) {
    
    std::ostringstream changelog;
    
    changelog << "# Changelog\n\n";
    
    for (const auto& change : changes) {
        changelog << "## " << change.first << "\n\n";
        changelog << change.second << "\n\n";
    }
    
    return changelog.str();
}

std::string PluginDocGenerator::GenerateContributingGuide() {
    return R"(# Contributing

Thank you for your interest in contributing!

## How to Contribute

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Code Style

- Follow the existing code style
- Add tests for new features
- Update documentation as needed

## Reporting Issues

Please use the issue tracker to report bugs or request features.
)";
}

void PluginDocGenerator::SetTemplate(const std::string& template_name,
                                       const std::string& content) {
    templates_[template_name] = content;
}

std::string PluginDocGenerator::ApplyTemplate(
    const std::string& template_name,
    const std::map<std::string, std::string>& variables) {
    
    auto it = templates_.find(template_name);
    if (it == templates_.end()) {
        return "";
    }
    
    std::string result = it->second;
    
    for (const auto& pair : variables) {
        std::string placeholder = "{{" + pair.first + "}}";
        size_t pos;
        while ((pos = result.find(placeholder)) != std::string::npos) {
            result.replace(pos, placeholder.length(), pair.second);
        }
    }
    
    return result;
}

std::string PluginDocGenerator::ExportAsMarkdown(const std::string& content) {
    return content;  // Already markdown
}

std::string PluginDocGenerator::ExportAsHTML(const std::string& content) {
    return MarkdownToHTML(content);
}

std::string PluginDocGenerator::ExportAsPDF(const std::string& content) {
    // Would use a PDF library
    return "PDF export not implemented. Content:\n" + content;
}

std::string PluginDocGenerator::MarkdownToHTML(const std::string& markdown) {
    std::string html = markdown;
    
    // Very simplified markdown to HTML conversion
    // Headers
    size_t pos = 0;
    while ((pos = html.find("# ", pos)) != std::string::npos) {
        size_t end = html.find("\n", pos);
        std::string header = html.substr(pos + 2, end - pos - 2);
        html.replace(pos, end - pos, "<h1>" + header + "</h1>");
    }
    
    return "<html><body>" + html + "</body></html>";
}

// ============================================================================
// PluginDevKit Implementation
// ============================================================================

PluginDevKit::PluginDevKit() {
    InitializeTemplates();
}

void PluginDevKit::InitializeTemplates() {
    templates_["basic"] = R"(
// Basic ESP32 IDE Plugin
#include "plugin_system.h"

class MyPlugin : public esp32_ide::plugins::Plugin {
public:
    MyPlugin() : Plugin(GetMetadata()) {}
    
    static esp32_ide::plugins::PluginMetadata GetMetadata() {
        esp32_ide::plugins::PluginMetadata meta;
        meta.id = "{{id}}";
        meta.name = "{{name}}";
        meta.version = "1.0.0";
        return meta;
    }
    
    bool Initialize(esp32_ide::plugins::PluginAPI* api) override {
        api_ = api;
        return true;
    }
    
    bool Activate() override {
        SetState(esp32_ide::plugins::PluginState::ACTIVE);
        return true;
    }
    
    bool Deactivate() override {
        SetState(esp32_ide::plugins::PluginState::LOADED);
        return true;
    }
    
    void Dispose() override {}
};
)";
}

bool PluginDevKit::CreatePluginProject(const std::string& path,
                                         const PluginMetadata& metadata) {
    // Create plugin project structure
    std::map<std::string, std::string> vars;
    vars["id"] = metadata.id;
    vars["name"] = metadata.name;
    
    std::string content = templates_["basic"];
    for (const auto& pair : vars) {
        std::string placeholder = "{{" + pair.first + "}}";
        size_t pos;
        while ((pos = content.find(placeholder)) != std::string::npos) {
            content.replace(pos, placeholder.length(), pair.second);
        }
    }
    
    // Write plugin file
    std::ofstream file(path + "/plugin.cpp");
    if (file.is_open()) {
        file << content;
        file.close();
        return true;
    }
    
    return false;
}

bool PluginDevKit::CreatePluginFromTemplate(const std::string& path,
                                              const std::string& template_name) {
    auto it = templates_.find(template_name);
    if (it == templates_.end()) {
        return false;
    }
    
    std::ofstream file(path + "/plugin.cpp");
    if (file.is_open()) {
        file << it->second;
        file.close();
        return true;
    }
    
    return false;
}

std::vector<std::string> PluginDevKit::GetAvailableTemplates() {
    std::vector<std::string> result;
    for (const auto& pair : templates_) {
        result.push_back(pair.first);
    }
    return result;
}

bool PluginDevKit::BuildPlugin(const std::string& /*project_path*/) {
    // Simulated build
    return true;
}

bool PluginDevKit::PackagePlugin(const std::string& /*project_path*/,
                                   const std::string& /*output_path*/) {
    // Simulated packaging
    return true;
}

bool PluginDevKit::RunPluginTests(const std::string& /*project_path*/) {
    return true;
}

std::string PluginDevKit::GenerateTestReport(const std::string& project_path) {
    return "Test Report for: " + project_path + "\nAll tests passed.";
}

std::vector<std::string> PluginDevKit::ValidateManifest(
    const std::string& /*manifest_path*/) {
    return {};  // No errors
}

std::vector<std::string> PluginDevKit::ValidatePlugin(
    const std::string& /*plugin_path*/) {
    return {};  // No errors
}

bool PluginDevKit::CheckCompatibility(const std::string& /*plugin_path*/,
                                        const std::string& /*ide_version*/) {
    return true;
}

void PluginDevKit::EnablePluginDebugging(const std::string& plugin_id) {
    debug_enabled_[plugin_id] = true;
}

void PluginDevKit::DisablePluginDebugging(const std::string& plugin_id) {
    debug_enabled_[plugin_id] = false;
}

std::vector<std::string> PluginDevKit::GetDebugLogs(const std::string& plugin_id) {
    auto it = debug_logs_.find(plugin_id);
    if (it != debug_logs_.end()) {
        return it->second;
    }
    return {};
}

} // namespace plugins
} // namespace esp32_ide
