#ifndef VISUAL_STUDIO_PROJECT_H
#define VISUAL_STUDIO_PROJECT_H

#include <tinyxml2.h>

#include <string>
#include <vector>

#include "project_config.h"

namespace axsp {

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

#endif  // VISUAL_STUDIO_PROJECT_H
