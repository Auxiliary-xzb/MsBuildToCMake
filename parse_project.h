#ifndef MS_BUILD_TO_CMAKE_PARSE_PROJECT_H_
#define MS_BUILD_TO_CMAKE_PARSE_PROJECT_H_

#include <tinyxml2.h>

#include <memory>
#include <string_view>

#include "project.h"

std::shared_ptr<Project> parse_project(
    const tinyxml2::XMLElement* project_root_element);

void parse_property_group(const std::shared_ptr<Project>& p,
                          const tinyxml2::XMLElement* element);

void parse_item_group(const std::shared_ptr<Project>& p,
                      const tinyxml2::XMLElement* e);

void parse_cl_compile(const std::shared_ptr<Project>& p,
                      const tinyxml2::XMLElement* e);

void parse_additional_includes(const std::shared_ptr<Project>& p,
                               const std::string_view& incs);

#endif  // MS_BUILD_TO_CMAKE_PARSE_PROJECT_H_
