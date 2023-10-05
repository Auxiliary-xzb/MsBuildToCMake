#ifndef PARSE_PROJECT_H
#define PARSE_PROJECT_H

#include <memory>
#include <tinyxml2.h>

#include "project.h"

using std::shared_ptr;
using std::make_shared;
using tinyxml2::XMLElement;

shared_ptr<Project> parse_project(const XMLElement* e);

void parse_propertygroup(shared_ptr<Project> p, const XMLElement* e);
void parse_itemgroup(shared_ptr<Project> p, const XMLElement* e);
void parse_clcompile(shared_ptr<Project> p, const XMLElement* e);

#endif