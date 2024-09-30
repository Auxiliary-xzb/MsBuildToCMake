#ifndef PARSE_PROJECT_H
#define PARSE_PROJECT_H

#include <tinyxml2.h>

#include <memory>
#include <string_view>

#include "project.h"

using std::make_shared;
using std::shared_ptr;
using std::string_view;
using tinyxml2::XMLElement;

shared_ptr<Project> parse_project(const XMLElement* e);

void parse_propertygroup(const shared_ptr<Project>& p, const XMLElement* e);
void parse_itemgroup(const shared_ptr<Project>& p, const XMLElement* e);
void parse_clcompile(const shared_ptr<Project>& p, const XMLElement* e);
void parse_additionalincludes(const shared_ptr<Project>& p,
                              const string_view& incs);
#endif
