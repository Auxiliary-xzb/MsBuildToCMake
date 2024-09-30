#include "parse_project.h"

#include <tixml2ex.h>

#include <ranges>
#include <string_view>

std::shared_ptr<Project> parse_project(const tinyxml2::XMLElement* e) {
  auto p = std::make_shared<Project>();

  for (auto x : e) {
    std::string name = x->Name();
    if (name == "PropertyGroup") parse_property_group(p, x);
    if (name == "ItemGroup") parse_item_group(p, x);
    if (name == "ItemDefinitionGroup") parse_item_group(p, x);
  }

  return p;
}

void parse_conditional_propertygroup(const std::shared_ptr<Project>&,
                                     const tinyxml2::XMLElement*);
void parse_global_propertygroup(const std::shared_ptr<Project>&,
                                const tinyxml2::XMLElement*);

void parse_property_group(const std::shared_ptr<Project>& p,
                          const tinyxml2::XMLElement* e) {
  auto av = e->Attribute("Condition");
  if (av) {
    std::string_view condition(av);
    if (condition.find_first_of("Release") != std::string_view::npos) {
      parse_conditional_propertygroup(p, e);
    }
  }

  auto gbl = e->Attribute("Label");
  if (gbl && std::string(gbl) == "Globals") parse_global_propertygroup(p, e);
}

void parse_conditional_propertygroup(const std::shared_ptr<Project>& p,
                                     const tinyxml2::XMLElement* e) {
  for (auto i : e) {
    std::string name = i->Name();
    std::string text = i->GetText();

    if (name == "TargetName") p->name = text;
    if (name == "ConfigurationType") {
      if (text == "Application") p->isApplication = true;
    }
    if (name == "ClCompile") parse_cl_compile(p, i);
  }
}

void parse_global_propertygroup(const std::shared_ptr<Project>& p,
                                const tinyxml2::XMLElement* e) {
  for (auto i : e) {
    std::string name = i->Name();
    std::string text = i->GetText();

    if (name == "RootNamespace") p->name = text;
  }
}

void parse_item_group(const std::shared_ptr<Project>& p,
                      const tinyxml2::XMLElement* e) {
  auto av = e->Attribute("Condition");
  if (av) {
    std::string_view condition(av);
    if (condition.find_first_of("Release") == std::string_view::npos) return;
  }

  for (auto x : e) {
    std::string name = x->Name();
    if (name == "ClCompile") {
      auto inc = x->Attribute("Include");
      if (inc) {
        p->source.emplace_back(inc);
      } else {
        parse_cl_compile(p, x);
      }
    }
  }
}

void parse_cl_compile(const std::shared_ptr<Project>& p,
                      const tinyxml2::XMLElement* e) {
  for (auto x : e) {
    std::string name = x->Name();
    std::string text = x->GetText();
    if (name == "AdditionalIncludeDirectories") {
      parse_additional_includes(p, text);
    }
    if (name == "LanguageStandard") {
      if (text == "stdcpp11")
        p->standard = LanguageStandard::cpp11;
      else if (text == "stdcpp14")
        p->standard = LanguageStandard::cpp14;
      else if (text == "stdcpp17")
        p->standard = LanguageStandard::cpp17;
      else if (text == "stdcpp20")
        p->standard = LanguageStandard::cpp20;
      else if (text == "stdcpp23")
        p->standard = LanguageStandard::cpp23;
      else if (text == "stdcpp2x")
        p->standard = LanguageStandard::cpp2x;
    }
  }
}

void parse_additional_includes(const std::shared_ptr<Project>& p,
                               const std::string_view& incs) {
  std::string_view delim{";"};
  const std::string ProjectDir = "$(ProjectDir)";

  for (const auto i : std::ranges::views::split(incs, delim)) {
    auto c = std::string_view(i.begin(), i.end());
    if (c.starts_with(ProjectDir)) {
      c.remove_prefix(ProjectDir.size());
      std::string mydir = std::string("${CMAKE_PROJECT_DIR}") + std::string(c);
      p->includes.emplace_back(mydir);
    } else {
      p->includes.emplace_back(c);
    }
  }
}
