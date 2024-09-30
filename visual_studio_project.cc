#include "visual_studio_project.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <tixml2ex.h>

#include <filesystem>
#include <string>

using namespace axsp;

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

  ParseProjectConfigurations();
  ParseHeaderFiles();
  ParseSourceFiles();
  return true;
}

void VisualStudioProject::ParseProjectConfigurations() {
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

void VisualStudioProject::ParseProjectConfiguration() {
  for (auto& project_configuration : project_configuration_vec_) {
    auto configuration = project_configuration.ToString();

    // 查找生成的目标类型
    for (const auto* property_group_element : tinyxml2::selection(
             vcx_project_xml_document_, "Project/PropertyGroup")) {
      auto condition = property_group_element->Attribute("Condition");
      if (condition != nullptr &&
          std::string(condition).find_first_of(configuration)) {
        for (const auto* configuration_type_element :
             tinyxml2::selection(property_group_element, "ConfigurationType")) {
          project_configuration.set_configuration_type(
              tinyxml2::text(configuration_type_element));
        }
      }
    }
  }

  for (auto& project_configuration : project_configuration_vec_) {
    //   spdlog::info("{}",
    //                static_cast<int>(project_configuration.configuration_type_));
   }
}
