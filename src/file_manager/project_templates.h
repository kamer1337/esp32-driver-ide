#ifndef PROJECT_TEMPLATES_H
#define PROJECT_TEMPLATES_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace esp32_ide {

/**
 * @brief Template file structure
 */
struct TemplateFile {
    std::string path;           // Relative path within project
    std::string content;        // File content (may contain variables like ${PROJECT_NAME})
    bool is_directory;          // True if this is a directory
};

/**
 * @brief Project template definition
 */
class ProjectTemplate {
public:
    ProjectTemplate(const std::string& id, const std::string& name);
    ~ProjectTemplate() = default;

    // Getters
    const std::string& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const std::string& GetDescription() const { return description_; }
    const std::string& GetCategory() const { return category_; }
    const std::vector<std::string>& GetTags() const { return tags_; }
    const std::vector<TemplateFile>& GetFiles() const { return files_; }

    // Setters
    void SetDescription(const std::string& desc) { description_ = desc; }
    void SetCategory(const std::string& category) { category_ = category; }
    void AddTag(const std::string& tag) { tags_.push_back(tag); }
    void SetIcon(const std::string& icon) { icon_ = icon; }

    // Files
    void AddFile(const std::string& path, const std::string& content);
    void AddDirectory(const std::string& path);

    // Variables
    void SetVariable(const std::string& name, const std::string& value);
    const std::map<std::string, std::string>& GetVariables() const { return variables_; }

    // Template processing
    std::string ProcessContent(const std::string& content) const;
    void ApplyVariables(const std::map<std::string, std::string>& user_vars);

private:
    std::string id_;
    std::string name_;
    std::string description_;
    std::string category_;
    std::vector<std::string> tags_;
    std::string icon_;
    std::vector<TemplateFile> files_;
    std::map<std::string, std::string> variables_;
};

/**
 * @brief Project template manager
 */
class ProjectTemplateManager {
public:
    ProjectTemplateManager();
    ~ProjectTemplateManager() = default;

    // Initialization
    bool Initialize();
    void Shutdown();

    // Template management
    void RegisterTemplate(std::unique_ptr<ProjectTemplate> template_obj);
    bool UnregisterTemplate(const std::string& template_id);
    ProjectTemplate* GetTemplate(const std::string& template_id) const;
    std::vector<std::string> GetTemplateIds() const;
    std::vector<ProjectTemplate*> GetAllTemplates() const;

    // Filtering
    std::vector<ProjectTemplate*> GetTemplatesByCategory(const std::string& category) const;
    std::vector<ProjectTemplate*> GetTemplatesByTag(const std::string& tag) const;
    std::vector<std::string> GetCategories() const;
    std::vector<std::string> GetTags() const;

    // Project creation
    struct CreateProjectOptions {
        std::string project_name;
        std::string project_path;
        std::string template_id;
        std::map<std::string, std::string> variables;
        bool create_git_repo;
        bool open_after_create;
    };

    bool CreateProject(const CreateProjectOptions& options);
    bool CreateProjectFromTemplate(const std::string& template_id, const std::string& project_path,
                                   const std::map<std::string, std::string>& variables);

    // Callbacks
    using ProjectCreatedCallback = std::function<void(const std::string& path)>;
    void SetProjectCreatedCallback(ProjectCreatedCallback callback) { project_created_callback_ = callback; }

    // Built-in templates
    void RegisterBuiltInTemplates();

private:
    std::map<std::string, std::unique_ptr<ProjectTemplate>> templates_;
    ProjectCreatedCallback project_created_callback_;

    // Built-in template creators
    std::unique_ptr<ProjectTemplate> CreateBasicSketchTemplate();
    std::unique_ptr<ProjectTemplate> CreateWiFiProjectTemplate();
    std::unique_ptr<ProjectTemplate> CreateBluetoothProjectTemplate();
    std::unique_ptr<ProjectTemplate> CreateWebServerTemplate();
    std::unique_ptr<ProjectTemplate> CreateIoTSensorTemplate();
    std::unique_ptr<ProjectTemplate> CreateLEDControlTemplate();
    std::unique_ptr<ProjectTemplate> CreateMotorControlTemplate();
    std::unique_ptr<ProjectTemplate> CreateDisplayProjectTemplate();

    // Helper methods
    bool CreateDirectoryStructure(const std::string& base_path, const std::vector<TemplateFile>& files);
    bool WriteTemplateFiles(const std::string& base_path, const std::vector<TemplateFile>& files);
    void NotifyProjectCreated(const std::string& path);
};

} // namespace esp32_ide

#endif // PROJECT_TEMPLATES_H
