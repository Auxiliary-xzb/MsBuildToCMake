#ifndef PARSE_PROJECT_H
#define PARSE_PROJECT_H

#include <memory>
#include <string_view>
#include <tinyxml2.h>

#include "project.h"

using std::string_view;
using std::shared_ptr;
using std::make_shared;
using tinyxml2::XMLElement;

shared_ptr<Project> parse_project(const XMLElement* e);

void parse_propertygroup(const shared_ptr<Project>& p, const XMLElement* e);
void parse_itemgroup(const shared_ptr<Project>& p, const XMLElement* e);
void parse_clcompile(const shared_ptr<Project>& p, const XMLElement* e);
void parse_additionalincludes(const shared_ptr<Project>& p, const string_view& incs);
#endif