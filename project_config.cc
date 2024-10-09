#include "project_config.h"

using namespace axsp;

void ProjectConfiguration::set_configuration_type(const std::string& type_str) {
  if (type_str == "Application") {
    configuration_type_ = ConfigurationType::kApplication;
  }

  if (type_str == "DynamicLibrary") {
    configuration_type_ = ConfigurationType::kDynamicLibrary;
  }
}

std::string axsp::format_as(const ProjectConfiguration& project_configuration) {
  return project_configuration.ToString();
}

std::string axsp::format_as(
    const ProjectConfiguration::ConfigurationType configuration_type) {
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
