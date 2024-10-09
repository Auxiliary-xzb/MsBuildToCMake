#include "visual_studio_project.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <tixml2ex.h>

#include <filesystem>
#include <string>

using namespace axsp;
using tinyxml2::attribute_value;
using tinyxml2::find_element;
using tinyxml2::selection;
using tinyxml2::text;

static std::vector<std::string> Split(const std::string& s,
                                      const char& delimiter);

bool VisualStudioProject::ParseFromFile(
    const std::string& vcx_project_file_path) {
  if (!std::filesystem::exists(vcx_project_file_path)) {
    spdlog::error("Failed to load project: {}", vcx_project_file_path);
    return false;
  }

  project_root_path_ =
      std::filesystem::path(vcx_project_file_path).parent_path().string();
  spdlog::info("Project root path: {}", project_root_path_);

  if (vcx_project_xml_document_.LoadFile(vcx_project_file_path.data()) !=
      tinyxml2::XML_SUCCESS) {
    spdlog::error("Failed to load project: {}", vcx_project_file_path);
    return false;
  }

  ParseConfigurationAndPlatform();
  ParseHeaderFiles();
  ParseSourceFiles();
  ParseOutputDirectories();
  ParseIntermediateDirectories();
  ParseAdditionalIncludeDirectories();
  ParsePreprocessorDefinitions();
  ParseAdditionalOptions();
  ParseAdditionalLibraryDirectories();
  ParsePostBuildEvent();
  ParsePreBuildEvent();
  return true;
}

void VisualStudioProject::ParseConfigurationAndPlatform() {
  /// 解析编译类型，目标平台。且根据其组合创建项目配置对象：
  /// 1. Project/ItemGroup/ProjectConfiguration/Configuration，编译类型
  /// 2. Project/ItemGroup/ProjectConfiguration/Platform，目标平台
  for (const auto* project_configuration :
       selection(vcx_project_xml_document_,
                 "Project/ItemGroup/ProjectConfiguration")) {
    const auto* configuration =
        find_element(project_configuration, "Configuration");

    const auto* platform = find_element(project_configuration, "Platform");

    //
    if (configuration != nullptr && platform != nullptr) {
      project_configuration_vec_.emplace_back(text(configuration),
                                              text(platform));
    }
  }

  spdlog::info("{}", fmt::join(project_configuration_vec_, "\n"));
}

void VisualStudioProject::ParseHeaderFiles() {
  /// 解析如下元素，解析成功后将头文件添加到头文件列表中：
  /// 1. Project/ItemGroup/ClInclude/Include，头文件
  for (const auto* header_file_element :
       selection(vcx_project_xml_document_, "Project/ItemGroup/ClInclude")) {
    auto header_file = attribute_value(header_file_element, "Include");
    if (!header_file.empty()) {
      header_file_vec_.emplace_back(header_file);
    }
  }

  // 对头文件列表以字母序进行排序
  std::sort(header_file_vec_.begin(), header_file_vec_.end());

  spdlog::info("{}", fmt::join(header_file_vec_, "\n"));
}

void VisualStudioProject::ParseSourceFiles() {
  /// 解析如下元素，解析成功后将源文件添加到源文件列表中：
  /// 1. Project/ItemGroup/ClCompile/Include，源文件
  for (const auto* source_file_element :
       selection(vcx_project_xml_document_, "Project/ItemGroup/ClCompile")) {
    auto source_file = attribute_value(source_file_element, "Include");
    if (!source_file.empty()) {
      source_file_vec_.emplace_back(source_file);
    }
  }

  // 对源文件列表字母序进行排序
  std::sort(source_file_vec_.begin(), source_file_vec_.end());

  spdlog::info("{}", fmt::join(source_file_vec_, "\n"));
}

void VisualStudioProject::ParseOutputDirectories() {
  /// 解析输出目录，并更新对应的项目配置对象
  /// 1. Project/PropertyGroup/OutDir，输出目录
  /// 2. Project/PropertyGroup(Condition)/OutDir，Condition属性用于
  ///    确定项目配置对象
  for (const auto* property_group_element :
       selection(vcx_project_xml_document_, "Project/PropertyGroup")) {
    const auto* out_dir_element =
        find_element(property_group_element, "OutDir");
    if (out_dir_element == nullptr) {
      continue;
    }

    auto condition = attribute_value(property_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->output_directory_ = text(out_dir_element);
    spdlog::info("configuration : {}, OutDir is {} ", it->ToString(),
                 text(out_dir_element));
  }
}

void VisualStudioProject::ParseIntermediateDirectories() {
  /// 解析中间目录，并更新对应的项目配置对象
  /// 1. Project/PropertyGroup/IntDir，中间目录
  /// 2. Project/PropertyGroup(Condition)/IntDir，Condition属性用于
  ///    确定项目配置对象
  for (const auto* property_group_element :
       selection(vcx_project_xml_document_, "Project/PropertyGroup")) {
    const auto* int_dir_element =
        find_element(property_group_element, "IntDir");
    if (int_dir_element == nullptr) {
      continue;
    }

    auto condition = attribute_value(property_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->intermediate_directory_ = text(int_dir_element);
    spdlog::info("configuration : {}, IntDir is {} ", it->ToString(),
                 text(int_dir_element));
  }
}

void VisualStudioProject::ParseAdditionalIncludeDirectories() {
  /// 解析额外的include目录，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/ClCompile/AdditionalIncludeDirectories，
  ///    额外的include目录
  /// 2. Project/ItemDefinitionGroup(Condition)/ClCompile/
  ///    AdditionalIncludeDirectories，Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_include_directories_element =
        find_element(item_definition_group_element,
                     "ClCompile/AdditionalIncludeDirectories");
    if (additional_include_directories_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->additional_include_directory_vec_ =
        Split(text(additional_include_directories_element), ';');
    spdlog::info("configuration : {},  additional include directories is {} ",
                 it->ToString(),
                 fmt::join(it->additional_include_directory_vec_, " "));
  }
}

void VisualStudioProject::ParsePreprocessorDefinitions() {
  /// 解析预处理器宏定义，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/ClCompile/PreprocessorDefinitions，
  ///    预处理器宏定义；
  /// 2. Project/ItemDefinitionGroup(Condition)/ClCompile/
  ///    PreprocessorDefinitions，Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* preprocessor_definitions_element = find_element(
        item_definition_group_element, "ClCompile/PreprocessorDefinitions");
    if (preprocessor_definitions_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->preprocessor_definition_vec_ =
        Split(text(preprocessor_definitions_element), ';');
    spdlog::info("configuration : {},  preprocessor definitions is {} ",
                 it->ToString(),
                 fmt::join(it->preprocessor_definition_vec_, " "));
  }
}

void VisualStudioProject::ParseAdditionalOptions() {
  /// 解析编译器选项，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/ClCompile/AdditionalOptions，编译器选项；
  /// 2. Project/ItemDefinitionGroup(Condition)/ClCompile/AdditionalOptions，
  ///    Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_options_element = find_element(
        item_definition_group_element, "ClCompile/AdditionalOptions");
    if (additional_options_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->additional_options_ = text(additional_options_element);
    spdlog::info("configuration : {}, additional options is {} ",
                 it->ToString(), it->additional_options_);
  }
}

void VisualStudioProject::ParseAdditionalLibraryDirectories() {
  /// 解析额外的动态库路径，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/Link/AdditionalLibraryDirectories，
  ///    额外的动态库路径；
  /// 2. Project/ItemDefinitionGroup(Condition)/Link/
  ///    AdditionalLibraryDirectories，Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_library_directories_element = find_element(
        item_definition_group_element, "Link/AdditionalLibraryDirectories");
    if (additional_library_directories_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->additional_library_directory_vec_ =
        Split(text(additional_library_directories_element), ';');
    spdlog::info("configuration : {},  additional library directories is {} ",
                 it->ToString(),
                 fmt::join(it->additional_library_directory_vec_, " "));
  }
}

void VisualStudioProject::ParsePostBuildEvent() {
  /// 解析编译后事件，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/PostBuildEvent/Command，编译后事件；
  /// 2. Project/ItemDefinitionGroup(Condition)/PostBuildEvent/Command，
  ///    Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* post_build_event_command_element =
        find_element(item_definition_group_element, "PostBuildEvent/Command");
    if (post_build_event_command_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->post_build_event_ = text(post_build_event_command_element);
    spdlog::info("configuration : {},  post build event command is {} ",
                 it->ToString(), it->post_build_event_);
  }
}

void VisualStudioProject::ParsePreBuildEvent() {
  /// 解析编译前事件，并更新对应的项目配置对象
  /// 1. Project/ItemDefinitionGroup/PreBuildEvent/Command，编译前事件；
  /// 2. Project/ItemDefinitionGroup(Condition)/PreBuildEvent/Command，
  ///    Condition属性用于确定项目配置对象
  for (const auto* item_definition_group_element :
       selection(vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* pre_build_event_command_element =
        find_element(item_definition_group_element, "PreBuildEvent/Command");
    if (pre_build_event_command_element == nullptr) {
      continue;
    }

    auto condition =
        attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->pre_build_event_ = text(pre_build_event_command_element);
    spdlog::info("configuration : {},  pre build event command is {} ",
                 it->ToString(), it->pre_build_event_);
  }
}

std::vector<std::string> Split(const std::string& s, const char& delimiter) {
  std::vector<std::string> tokens;
  std::istringstream ss(s);
  std::string token;

  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }

  return tokens;
}
