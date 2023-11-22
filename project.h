#ifndef PROJECT_H
#define PROJECT_H

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

enum class LanguageStandard {
    unspecified,
    cpp11,
    cpp14,
    cpp17,
    cpp20,
    cpp23,
    cpp2x
};

struct Project {
    string name;
    bool isApplication;
    LanguageStandard standard = LanguageStandard::unspecified;

    vector<string> source;
    vector<string> includes;
};

ostream& operator<<(ostream&, const Project&);

#endif