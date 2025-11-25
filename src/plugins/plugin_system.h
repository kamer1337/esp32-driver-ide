#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace esp32_ide {
namespace plugins {

// ============================================================================
// Plugin Architecture (Plugin API, Plugin manager, Marketplace integration)
// ============================================================================

/**
 * @brief Plugin type enumeration
 */
enum class PluginType {
    LANGUAGE_SUPPORT,
    THEME,
    TOOL,
    DEBUGGER,
    COMPILER,
    ANALYZER,
    UI_EXTENSION,
    DEVICE_SUPPORT,
    PROTOCOL,
    CUSTOM
};

/**
 * @brief Plugin state
 */
enum class PluginState {
    UNLOADED,
    LOADED,
    ACTIVE,
    DISABLED,
    ERROR
};

/**
 * @brief Plugin metadata
 */
struct PluginMetadata {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string license;
    std::string homepage;
    std::string repository;
    PluginType type;
    std::vector<std::string> keywords;
    std::vector<std::string> dependencies;
    std::string min_ide_version;
    std::string icon_path;
};

/**
 * @brief Plugin configuration
 */
struct PluginConfig {
    bool enabled;
    std::map<std::string, std::string> settings;
    std::vector<std::string> keybindings;
};

/**
 * @brief Plugin API interface for extensions
 */
class PluginAPI {
public:
    virtual ~PluginAPI() = default;
    
    // Editor API
    virtual std::string GetCurrentFileContent() const = 0;
    virtual void SetCurrentFileContent(const std::string& content) = 0;
    virtual std::string GetSelectedText() const = 0;
    virtual void InsertText(const std::string& text) = 0;
    virtual int GetCursorLine() const = 0;
    virtual int GetCursorColumn() const = 0;
    virtual void SetCursor(int line, int column) = 0;
    
    // File API
    virtual std::vector<std::string> GetOpenFiles() const = 0;
    virtual std::string GetCurrentFilePath() const = 0;
    virtual bool OpenFile(const std::string& path) = 0;
    virtual bool SaveFile(const std::string& path) = 0;
    
    // UI API
    virtual void ShowMessage(const std::string& message, const std::string& type = "info") = 0;
    virtual std::string ShowInputDialog(const std::string& prompt, const std::string& default_value = "") = 0;
    virtual bool ShowConfirmDialog(const std::string& message) = 0;
    virtual void ShowProgress(const std::string& title, float progress) = 0;
    virtual void HideProgress() = 0;
    
    // Output API
    virtual void WriteToConsole(const std::string& message) = 0;
    virtual void ClearConsole() = 0;
    
    // Commands API
    virtual void RegisterCommand(const std::string& command_id, 
                                 std::function<void()> handler) = 0;
    virtual void ExecuteCommand(const std::string& command_id) = 0;
    
    // Settings API
    virtual std::string GetSetting(const std::string& key) const = 0;
    virtual void SetSetting(const std::string& key, const std::string& value) = 0;
    
    // Event API
    virtual void OnEvent(const std::string& event_name, 
                        std::function<void(const std::map<std::string, std::string>&)> handler) = 0;
    virtual void EmitEvent(const std::string& event_name, 
                          const std::map<std::string, std::string>& data) = 0;
};

/**
 * @brief Base class for all plugins
 */
class Plugin {
public:
    Plugin(const PluginMetadata& metadata);
    virtual ~Plugin() = default;
    
    // Lifecycle
    virtual bool Initialize(PluginAPI* api) = 0;
    virtual bool Activate() = 0;
    virtual bool Deactivate() = 0;
    virtual void Dispose() = 0;
    
    // Metadata
    const PluginMetadata& GetMetadata() const { return metadata_; }
    PluginState GetState() const { return state_; }
    
    // Configuration
    void SetConfig(const PluginConfig& config) { config_ = config; }
    const PluginConfig& GetConfig() const { return config_; }
    
    // Error handling
    std::string GetLastError() const { return last_error_; }
    
protected:
    PluginMetadata metadata_;
    PluginState state_;
    PluginConfig config_;
    PluginAPI* api_;
    std::string last_error_;
    
    void SetState(PluginState state) { state_ = state; }
    void SetError(const std::string& error);
};

/**
 * @brief Plugin manager for loading, managing, and unloading plugins
 */
class PluginManager {
public:
    using PluginFactory = std::function<std::unique_ptr<Plugin>()>;
    using PluginCallback = std::function<void(const Plugin*)>;
    
    PluginManager();
    ~PluginManager() = default;
    
    // Plugin registration
    void RegisterPluginFactory(const std::string& plugin_id, PluginFactory factory);
    void UnregisterPluginFactory(const std::string& plugin_id);
    
    // Plugin lifecycle
    bool LoadPlugin(const std::string& plugin_id);
    bool UnloadPlugin(const std::string& plugin_id);
    bool EnablePlugin(const std::string& plugin_id);
    bool DisablePlugin(const std::string& plugin_id);
    bool ReloadPlugin(const std::string& plugin_id);
    
    // Batch operations
    void LoadAllPlugins();
    void UnloadAllPlugins();
    void EnableAllPlugins();
    void DisableAllPlugins();
    
    // Plugin queries
    Plugin* GetPlugin(const std::string& plugin_id);
    std::vector<Plugin*> GetAllPlugins();
    std::vector<Plugin*> GetPluginsByType(PluginType type);
    std::vector<Plugin*> GetActivePlugins();
    bool IsPluginLoaded(const std::string& plugin_id) const;
    
    // Dependency management
    bool CheckDependencies(const std::string& plugin_id) const;
    std::vector<std::string> GetMissingDependencies(const std::string& plugin_id) const;
    std::vector<std::string> GetDependencyOrder() const;
    
    // Configuration
    void SetPluginConfig(const std::string& plugin_id, const PluginConfig& config);
    PluginConfig GetPluginConfig(const std::string& plugin_id) const;
    void SavePluginConfigs(const std::string& path);
    void LoadPluginConfigs(const std::string& path);
    
    // API access
    void SetAPI(PluginAPI* api) { api_ = api; }
    
    // Events
    void SetPluginLoadedCallback(PluginCallback callback) { on_loaded_ = callback; }
    void SetPluginUnloadedCallback(PluginCallback callback) { on_unloaded_ = callback; }
    
    // Statistics
    int GetLoadedPluginCount() const;
    int GetActivePluginCount() const;
    
private:
    std::map<std::string, PluginFactory> factories_;
    std::map<std::string, std::unique_ptr<Plugin>> plugins_;
    std::map<std::string, PluginConfig> configs_;
    PluginAPI* api_;
    PluginCallback on_loaded_;
    PluginCallback on_unloaded_;
    
    void NotifyPluginLoaded(const Plugin* plugin);
    void NotifyPluginUnloaded(const Plugin* plugin);
};

// ============================================================================
// Custom Tools (External tool integration, Custom compilers, Analysis tools)
// ============================================================================

/**
 * @brief External tool configuration
 */
struct ExternalToolConfig {
    std::string id;
    std::string name;
    std::string command;
    std::string arguments;
    std::string working_directory;
    std::vector<std::string> environment;
    bool capture_output;
    bool show_in_menu;
    std::string keybinding;
    std::string icon;
};

/**
 * @brief Tool execution result
 */
struct ToolExecutionResult {
    int exit_code;
    std::string stdout_output;
    std::string stderr_output;
    long long execution_time_ms;
    bool timed_out;
    std::string error_message;
};

/**
 * @brief Custom compiler configuration
 */
struct CompilerConfig {
    std::string id;
    std::string name;
    std::string compiler_path;
    std::string linker_path;
    std::vector<std::string> default_flags;
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
    std::map<std::string, std::string> defines;
    std::string output_extension;
    std::string error_pattern;  // Regex for parsing errors
    std::string warning_pattern;  // Regex for parsing warnings
};

/**
 * @brief Analysis tool result
 */
struct AnalysisResult {
    std::string file_path;
    int line_number;
    int column_number;
    std::string severity;  // "error", "warning", "info", "hint"
    std::string message;
    std::string rule_id;
    std::string suggestion;
};

/**
 * @brief External tool manager
 */
class ExternalToolManager {
public:
    using OutputCallback = std::function<void(const std::string&)>;
    
    ExternalToolManager();
    ~ExternalToolManager() = default;
    
    // Tool registration
    void RegisterTool(const ExternalToolConfig& config);
    void UnregisterTool(const std::string& tool_id);
    void UpdateTool(const ExternalToolConfig& config);
    
    // Tool execution
    ToolExecutionResult ExecuteTool(const std::string& tool_id,
                                     const std::map<std::string, std::string>& variables = {});
    void ExecuteToolAsync(const std::string& tool_id,
                          OutputCallback on_output,
                          std::function<void(const ToolExecutionResult&)> on_complete);
    bool CancelExecution(const std::string& tool_id);
    
    // Tool queries
    ExternalToolConfig GetToolConfig(const std::string& tool_id) const;
    std::vector<ExternalToolConfig> GetAllTools() const;
    std::vector<ExternalToolConfig> GetMenuTools() const;
    bool ToolExists(const std::string& tool_id) const;
    
    // Variable substitution
    std::string SubstituteVariables(const std::string& text,
                                     const std::map<std::string, std::string>& variables) const;
    
    // Configuration
    void SaveTools(const std::string& path);
    void LoadTools(const std::string& path);
    
private:
    std::map<std::string, ExternalToolConfig> tools_;
    std::map<std::string, bool> running_tools_;
    
    ToolExecutionResult RunCommand(const std::string& command,
                                    const std::string& working_dir,
                                    const std::vector<std::string>& env,
                                    int timeout_ms = 30000);
};

/**
 * @brief Custom compiler manager
 */
class CustomCompilerManager {
public:
    CustomCompilerManager();
    ~CustomCompilerManager() = default;
    
    // Compiler registration
    void RegisterCompiler(const CompilerConfig& config);
    void UnregisterCompiler(const std::string& compiler_id);
    
    // Compilation
    ToolExecutionResult Compile(const std::string& compiler_id,
                                 const std::string& source_file,
                                 const std::string& output_file,
                                 const std::vector<std::string>& extra_flags = {});
    ToolExecutionResult Link(const std::string& compiler_id,
                              const std::vector<std::string>& object_files,
                              const std::string& output_file,
                              const std::vector<std::string>& libraries = {});
    
    // Error parsing
    std::vector<AnalysisResult> ParseCompilerOutput(const std::string& compiler_id,
                                                      const std::string& output);
    
    // Compiler queries
    CompilerConfig GetCompilerConfig(const std::string& compiler_id) const;
    std::vector<CompilerConfig> GetAllCompilers() const;
    std::vector<std::string> GetCompilerIds() const;
    bool CompilerExists(const std::string& compiler_id) const;
    
    // Configuration
    void SaveCompilers(const std::string& path);
    void LoadCompilers(const std::string& path);
    
private:
    std::map<std::string, CompilerConfig> compilers_;
};

/**
 * @brief Analysis tool runner
 */
class AnalysisToolRunner {
public:
    AnalysisToolRunner();
    ~AnalysisToolRunner() = default;
    
    // Tool registration
    void RegisterAnalyzer(const std::string& analyzer_id,
                           const std::string& command,
                           const std::string& output_pattern);
    void UnregisterAnalyzer(const std::string& analyzer_id);
    
    // Analysis
    std::vector<AnalysisResult> RunAnalysis(const std::string& analyzer_id,
                                              const std::string& file_path);
    std::vector<AnalysisResult> RunAnalysisOnProject(const std::string& analyzer_id,
                                                       const std::string& project_path);
    
    // Built-in analyzers
    std::vector<AnalysisResult> RunStaticAnalysis(const std::string& code);
    std::vector<AnalysisResult> RunStyleCheck(const std::string& code);
    std::vector<AnalysisResult> RunSecurityScan(const std::string& code);
    std::vector<AnalysisResult> RunComplexityAnalysis(const std::string& code);
    
    // Report generation
    std::string GenerateReport(const std::vector<AnalysisResult>& results,
                                const std::string& format = "text");
    
private:
    struct AnalyzerConfig {
        std::string command;
        std::string output_pattern;
    };
    
    std::map<std::string, AnalyzerConfig> analyzers_;
    
    std::vector<AnalysisResult> ParseAnalyzerOutput(const std::string& output,
                                                      const std::string& pattern);
};

// ============================================================================
// Extension Ecosystem (Community contributions, Plugin repository, Documentation)
// ============================================================================

/**
 * @brief Marketplace plugin listing
 */
struct MarketplacePlugin {
    PluginMetadata metadata;
    std::string download_url;
    int downloads;
    float rating;
    int rating_count;
    std::chrono::system_clock::time_point published_at;
    std::chrono::system_clock::time_point updated_at;
    std::string changelog;
    std::vector<std::string> screenshots;
    bool is_verified;
    bool is_featured;
};

/**
 * @brief Plugin review
 */
struct PluginReview {
    std::string reviewer_id;
    std::string reviewer_name;
    int rating;  // 1-5
    std::string comment;
    std::chrono::system_clock::time_point created_at;
    int helpful_count;
};

/**
 * @brief Marketplace search filter
 */
struct MarketplaceFilter {
    std::string query;
    PluginType type;
    std::string author;
    float min_rating;
    bool verified_only;
    bool featured_only;
    std::string sort_by;  // "downloads", "rating", "updated", "name"
    bool sort_descending;
    int page;
    int per_page;
};

/**
 * @brief Plugin marketplace client
 */
class PluginMarketplace {
public:
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    
    PluginMarketplace();
    ~PluginMarketplace() = default;
    
    // Configuration
    void SetMarketplaceUrl(const std::string& url);
    void SetAuthToken(const std::string& token);
    
    // Search and browse
    std::vector<MarketplacePlugin> Search(const MarketplaceFilter& filter);
    std::vector<MarketplacePlugin> GetFeatured();
    std::vector<MarketplacePlugin> GetPopular(int limit = 10);
    std::vector<MarketplacePlugin> GetRecent(int limit = 10);
    std::vector<MarketplacePlugin> GetByAuthor(const std::string& author);
    MarketplacePlugin GetPluginDetails(const std::string& plugin_id);
    
    // Installation
    bool InstallPlugin(const std::string& plugin_id, ProgressCallback on_progress = nullptr);
    bool UpdatePlugin(const std::string& plugin_id, ProgressCallback on_progress = nullptr);
    bool UninstallPlugin(const std::string& plugin_id);
    std::vector<std::string> GetInstalledPlugins();
    bool IsPluginInstalled(const std::string& plugin_id) const;
    bool HasUpdate(const std::string& plugin_id) const;
    
    // Reviews
    std::vector<PluginReview> GetReviews(const std::string& plugin_id, int page = 1);
    bool SubmitReview(const std::string& plugin_id, int rating, const std::string& comment);
    bool MarkReviewHelpful(const std::string& review_id);
    
    // Publishing (for developers)
    bool PublishPlugin(const std::string& package_path);
    bool UpdatePluginListing(const std::string& plugin_id, const PluginMetadata& metadata);
    bool UnpublishPlugin(const std::string& plugin_id);
    
    // Statistics
    int GetTotalPluginCount() const { return total_plugins_; }
    int GetInstalledCount() const { return installed_plugins_.size(); }
    
private:
    std::string marketplace_url_;
    std::string auth_token_;
    std::string install_path_;
    std::vector<std::string> installed_plugins_;
    int total_plugins_;
    
    bool DownloadPlugin(const std::string& url, const std::string& dest_path,
                        ProgressCallback on_progress);
    bool ExtractPlugin(const std::string& package_path, const std::string& dest_path);
    bool ValidatePlugin(const std::string& plugin_path);
};

/**
 * @brief Plugin documentation generator
 */
class PluginDocGenerator {
public:
    PluginDocGenerator();
    ~PluginDocGenerator() = default;
    
    // Documentation generation
    std::string GenerateReadme(const PluginMetadata& metadata);
    std::string GenerateAPIReference(const Plugin* plugin);
    std::string GenerateChangelog(const std::vector<std::pair<std::string, std::string>>& changes);
    std::string GenerateContributingGuide();
    
    // Template support
    void SetTemplate(const std::string& template_name, const std::string& content);
    std::string ApplyTemplate(const std::string& template_name,
                               const std::map<std::string, std::string>& variables);
    
    // Export formats
    std::string ExportAsMarkdown(const std::string& content);
    std::string ExportAsHTML(const std::string& content);
    std::string ExportAsPDF(const std::string& content);
    
private:
    std::map<std::string, std::string> templates_;
    
    std::string MarkdownToHTML(const std::string& markdown);
};

/**
 * @brief Plugin development toolkit
 */
class PluginDevKit {
public:
    PluginDevKit();
    ~PluginDevKit() = default;
    
    // Project scaffolding
    bool CreatePluginProject(const std::string& path, const PluginMetadata& metadata);
    bool CreatePluginFromTemplate(const std::string& path, const std::string& template_name);
    std::vector<std::string> GetAvailableTemplates();
    
    // Building
    bool BuildPlugin(const std::string& project_path);
    bool PackagePlugin(const std::string& project_path, const std::string& output_path);
    
    // Testing
    bool RunPluginTests(const std::string& project_path);
    std::string GenerateTestReport(const std::string& project_path);
    
    // Validation
    std::vector<std::string> ValidateManifest(const std::string& manifest_path);
    std::vector<std::string> ValidatePlugin(const std::string& plugin_path);
    bool CheckCompatibility(const std::string& plugin_path, const std::string& ide_version);
    
    // Debugging
    void EnablePluginDebugging(const std::string& plugin_id);
    void DisablePluginDebugging(const std::string& plugin_id);
    std::vector<std::string> GetDebugLogs(const std::string& plugin_id);
    
private:
    std::map<std::string, std::string> templates_;
    std::map<std::string, bool> debug_enabled_;
    std::map<std::string, std::vector<std::string>> debug_logs_;
    
    void InitializeTemplates();
};

} // namespace plugins
} // namespace esp32_ide

#endif // PLUGIN_SYSTEM_H
