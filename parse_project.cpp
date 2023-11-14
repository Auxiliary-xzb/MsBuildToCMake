#include "parse_project.h"
#include <tixml2ex.h>
#include <ranges>
#include <string_view>

using std::string_view;
using std::ranges::views::lazy_split;
using namespace std::ranges;
using namespace std::views;

shared_ptr<Project> parse_project(const XMLElement* e) {
    auto p = make_shared<Project>();

    for(auto x: e) {
        string name = x->Name();
        if (name == "PropertyGroup") parse_propertygroup(p, x);
        if (name == "ItemGroup") parse_itemgroup(p, x);
        if (name == "ItemDefinitionGroup") parse_itemgroup(p, x);
    }

    return p;
}

void parse_propertygroup(const shared_ptr<Project>& p, const XMLElement* e) {
    auto av = e->Attribute("Condition");
    if (av) {
        string_view condition(av);
        if (condition.find_first_of("Release") != string_view::npos) {
            // We care about this one

            for(auto i: e) {
                string name = i->Name();
                string text = i->GetText();

                if (name == "TargetName") p->name = text;
                if (name == "ConfigurationType") {
                    if (text == "Application")
                        p->isApplication = true;
                }
            }

        }
    }
}

void parse_itemgroup(const shared_ptr<Project>& p, const XMLElement* e) {
    auto av = e->Attribute("Condition");
    if (av) {
        string_view condition(av);
        if (condition.find_first_of("Release") == string_view::npos) return;
    }

    for(auto x: e) {
        string name = x->Name();
        if (name == "ClCompile") {
            auto inc = x->Attribute("Include");
            if (inc) {
                p->source.emplace_back(inc);
            }
            else {
                parse_clcompile(p, x);
            }
        }
    }
}

void parse_clcompile(const shared_ptr<Project>& p, const XMLElement* e) {
    for (auto x: e) {
        string name = x->Name();
        string text = x->GetText();
        if (name == "AdditionalIncludeDirectories") {
            parse_additionalincludes(p, text);
        }
    }


}

void parse_additionalincludes(const shared_ptr<Project>& p, const string_view& incs) {
    string_view delim { ";" };
    const string ProjectDir = "$(ProjectDir)";

    for(const auto i : split(incs, delim)) {
        auto c = string_view(i.begin(), i.end());
        if (c.starts_with(ProjectDir)) {
            c.remove_prefix(ProjectDir.size());
            string mydir = string("${CMAKE_PROJECT_DIR}") + string(c);
            p->includes.emplace_back(mydir);
        } else {
            p->includes.emplace_back(c);
        }
    }
}


