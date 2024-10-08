#include "visual_studio_project.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <tixml2ex.h>

#include <filesystem>
#include <string>

using namespace axsp;

static std::vector<std::string> Split(const std::string& s,
                                      const char& delimiter);

void ProjectConfiguration::set_configuration_type(const std::string& type_str) {
  if (type_str == "Application") {
    configuration_type_ = ConfigurationType::kApplication;
  }

  if (type_str == "DynamicLibrary") {
    configuration_type_ = ConfigurationType::kDynamicLibrary;
  }
}

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
  return true;
}

void VisualStudioProject::ParseConfigurationAndPlatform() {
  // <Project ...>
  //   <ItemGroup Label="ProjectConfigurations">
  //     <ProjectConfiguration Include="Debug|Win32">
  //       <Configuration>Debug</Configuration>
  //       <Platform>Win32</Platform>
  //     </ProjectConfiguration>
  //   </ItemGroup>
  // </Project>
  // 解析出Debug和Win32，并使用他们创建一个项目配置
  for (const auto* project_configuration :
       tinyxml2::selection(vcx_project_xml_document_,
                           "Project/ItemGroup/ProjectConfiguration")) {
    const auto* configuration =
        tinyxml2::find_element(project_configuration, "Configuration");

    const auto* platform =
        tinyxml2::find_element(project_configuration, "Platform");
    if (configuration != nullptr && platform != nullptr) {
      project_configuration_vec_.emplace_back(tinyxml2::text(configuration),
                                              tinyxml2::text(platform));
    }
  }

  spdlog::info("{}", fmt::join(project_configuration_vec_, "\n"));
}

void VisualStudioProject::ParseHeaderFiles() {
  // <Project ...>
  //   <ItemGroup>
  //     <ClInclude Include="xx.h" />
  //     <ClInclude Include="yyy.h" />
  //    </ItemGroup>
  // </Project>
  // 收集所有的源文件

  for (const auto* header_file_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemGroup/ClInclude")) {
    auto header_file =
        tinyxml2::attribute_value(header_file_element, "Include");
    if (!header_file.empty()) {
      header_file_vec_.emplace_back(header_file);
    }
  }

  std::sort(header_file_vec_.begin(), header_file_vec_.end());

  spdlog::info("{}", fmt::join(header_file_vec_, "\n"));
}

void VisualStudioProject::ParseSourceFiles() {
  // <Project ...>
  //   <ItemGroup>
  //     <ClCompile Include="xx.cc" />
  //     <ClCompile Include="yyy.cc" />
  //    </ItemGroup>
  // </Project>
  // 收集所有的源文件

  for (const auto* source_file_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemGroup/ClCompile")) {
    auto source_file =
        tinyxml2::attribute_value(source_file_element, "Include");
    if (!source_file.empty()) {
      source_file_vec_.emplace_back(source_file);
    }
  }

  std::sort(source_file_vec_.begin(), source_file_vec_.end());

  spdlog::info("{}", fmt::join(source_file_vec_, "\n"));
}

void VisualStudioProject::ParseOutputDirectories() {
  for (const auto* property_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/PropertyGroup")) {
    const auto* out_dir_element =
        tinyxml2::find_element(property_group_element, "OutDir");
    if (out_dir_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(property_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->output_directory_ = tinyxml2::text(out_dir_element);
    spdlog::info("configuration : {}, OutDir is {} ", it->ToString(),
                 tinyxml2::text(out_dir_element));
  }
}

void VisualStudioProject::ParseIntermediateDirectories() {
  for (const auto* property_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/PropertyGroup")) {
    const auto* int_dir_element =
        tinyxml2::find_element(property_group_element, "IntDir");
    if (int_dir_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(property_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->intermediate_directory_ = tinyxml2::text(int_dir_element);
    spdlog::info("configuration : {}, IntDir is {} ", it->ToString(),
                 tinyxml2::text(int_dir_element));
  }
}

void VisualStudioProject::ParseAdditionalIncludeDirectories() {
  for (const auto* item_definition_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_include_directories_element =
        tinyxml2::find_element(item_definition_group_element,
                               "ClCompile/AdditionalIncludeDirectories");
    if (additional_include_directories_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(item_definition_group_element, "Condition");

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
        Split(tinyxml2::text(additional_include_directories_element), ';');
    spdlog::info("configuration : {},  additional include directories is {} ",
                 it->ToString(),
                 fmt::join(it->additional_include_directory_vec_, " "));
  }
}

void VisualStudioProject::ParsePreprocessorDefinitions() {
  for (const auto* item_definition_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* preprocessor_definitions_element = tinyxml2::find_element(
        item_definition_group_element, "ClCompile/PreprocessorDefinitions");
    if (preprocessor_definitions_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(item_definition_group_element, "Condition");

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
        Split(tinyxml2::text(preprocessor_definitions_element), ';');
    spdlog::info("configuration : {},  preprocessor definitions is {} ",
                 it->ToString(),
                 fmt::join(it->preprocessor_definition_vec_, " "));
  }
}

void VisualStudioProject::ParseAdditionalOptions() {
  for (const auto* item_definition_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_options_element = tinyxml2::find_element(
        item_definition_group_element, "ClCompile/AdditionalOptions");
    if (additional_options_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(item_definition_group_element, "Condition");

    auto it = std::find_if(
        project_configuration_vec_.begin(), project_configuration_vec_.end(),
        [&](const ProjectConfiguration& project_configuration) {
          return condition.find(project_configuration.ToString()) != -1;
        });

    if (it == project_configuration_vec_.end()) {
      spdlog::info("No Such ProjectConfiguration : {}", condition);
      continue;
    }

    it->additional_options_ = tinyxml2::text(additional_options_element);
    spdlog::info("configuration : {}, additional options is {} ",
                 it->ToString(), it->additional_options_);
  }
}

void VisualStudioProject::ParseAdditionalLibraryDirectories() {
  for (const auto* item_definition_group_element : tinyxml2::selection(
           vcx_project_xml_document_, "Project/ItemDefinitionGroup")) {
    const auto* additional_library_directories_element = tinyxml2::find_element(
        item_definition_group_element, "Link/AdditionalLibraryDirectories");
    if (additional_library_directories_element == nullptr) {
      continue;
    }

    auto condition =
        tinyxml2::attribute_value(item_definition_group_element, "Condition");

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
        Split(tinyxml2::text(additional_library_directories_element), ';');
    spdlog::info("configuration : {},  additional library directories is {} ",
                 it->ToString(),
                 fmt::join(it->additional_library_directory_vec_, " "));
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
