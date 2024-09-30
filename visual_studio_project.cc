#include "visual_studio_project.h"

#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <tixml2ex.h>

#include <filesystem>
#include <string>

using namespace axsp;

bool VisualStudioProject::ParseFromFile(
    const std::string& vcx_project_file_path) {
  if (!std::filesystem::exists(vcx_project_file_path)) {
    spdlog::error("Failed to load project: {}", vcx_project_file_path);
    return false;
  }

  if (vcx_project_xml_document_.LoadFile(vcx_project_file_path.data()) !=
      tinyxml2::XML_SUCCESS) {
    spdlog::error("Failed to load project: {}", vcx_project_file_path);
    return false;
  }

  ParseProjectConfigurations();
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
    project_configuration_vec_.emplace_back(tinyxml2::text(configuration),
                                            tinyxml2::text(platform));
  }

  spdlog::info("{}", fmt::join(project_configuration_vec_, "\n"));
}
