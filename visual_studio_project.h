#ifndef VISUAL_STUDIO_PROJECT_H
#define VISUAL_STUDIO_PROJECT_H

#include <spdlog/fmt/bundled/core.h>
#include <tinyxml2.h>

#include <string>
#include <vector>

namespace axsp {

class ProjectConfiguration {
  friend class VisualStudioProject;

 public:
  enum class ConfigurationType {
    kUndefined,
    kApplication,     ///< 应用程序
    kDynamicLibrary,  ///< 动态库
  };

 public:
  ProjectConfiguration(std::string configuration, std::string platform)
      : configuration_(std::move(configuration)),
        platform_(std::move(platform)),
        configuration_type_(ConfigurationType::kUndefined) {}

  [[nodiscard]] std::string ToString() const {
    return fmt::format("{}|{}", configuration_, platform_);
  }

  bool operator==(const ProjectConfiguration &other) const {
    return configuration_ == other.configuration_ &&
           platform_ == other.platform_;
  }

  [[nodiscard]] auto configuration() const -> std::string {
    return configuration_;
  }

  [[nodiscard]] auto platform() const -> std::string { return platform_; }

  [[nodiscard]] auto output_directory() const -> std::string {
    return output_directory_;
  }

  [[nodiscard]] auto intermediate_directory() const -> std::string {
    return intermediate_directory_;
  }

  [[nodiscard]] auto additional_options() const -> std::string {
    return additional_options_;
  }

  void set_configuration_type(const std::string &type_str);

 private:
  std::string configuration_;             ///< 编译类型，Debug，Release等
  std::string platform_;                  ///< 编译目标平台
  ConfigurationType configuration_type_;  ///< 编译结果类型
  std::string output_directory_;          ///< 输出文件目录
  std::string intermediate_directory_;    ///< 中间生成文件目录
  std::vector<std::string>
      additional_include_directory_vec_;                  ///< 额外包含的目录
  std::vector<std::string> preprocessor_definition_vec_;  ///< 预处理定义宏
  std::string additional_options_;                        ///< 额外的编译器选项
  std::vector<std::string>
      additional_library_directory_vec_;                ///< 额外的库查找目录
  std::vector<std::string> additional_dependency_vec_;  ///< 额外的依赖，DLL/lib
  std::string post_build_event_;                        ///< 生成前事件命令
  std::string pre_build_event_;                         ///< 生成后事件命令
};

inline auto format_as(const ProjectConfiguration &project_configuration)
    -> std::string {
  return project_configuration.ToString();
}

inline auto format_as(
    const ProjectConfiguration::ConfigurationType configuration_type)
    -> std::string {
  std::string configuration_type_str;
  switch (configuration_type) {
    case ProjectConfiguration::ConfigurationType::kApplication: {
      configuration_type_str = "Application";
      break;
    }
    case ProjectConfiguration::ConfigurationType::kDynamicLibrary: {
      configuration_type_str = "DynamicLibrary";
      break;
    }
    default:
      configuration_type_str = "Unknown";
      break;
  }

  return configuration_type_str;
}

class VisualStudioProject {
 public:
  VisualStudioProject() = default;

  bool ParseFromFile(const std::string &vcx_project_file_path);

  auto project_name() const -> std::string { return project_name_; }

  auto source_file_list() const -> std::vector<std::string> {
    return source_file_vec_;
  }

  auto header_file_list() const -> std::vector<std::string> {
    return header_file_vec_;
  }

 private:
  void ParseConfigurationAndPlatform();
  void ParseHeaderFiles();
  void ParseSourceFiles();
  void ParseOutputDirectories();
  void ParseIntermediateDirectories();
  void ParseAdditionalIncludeDirectories();
  void ParsePreprocessorDefinitions();
  void ParseAdditionalOptions();
  void ParseAdditionalLibraryDirectories();
  void ParsePostBuildEvent();
  void ParsePreBuildEvent();

 private:
  std::string project_name_;                  ///< 项目名称
  std::string project_root_path_;             ///< 项目根目录
  std::vector<std::string> source_file_vec_;  ///< 源文件列表
  std::vector<std::string> header_file_vec_;  ///< 头文件列表
  std::vector<ProjectConfiguration> project_configuration_vec_;
  tinyxml2::XMLDocument vcx_project_xml_document_;  ///< XML对象
};

}  // namespace axsp

#endif //VISUAL_STUDIO_PROJECT_H
