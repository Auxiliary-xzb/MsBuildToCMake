#ifndef PROJECT_H
#define PROJECT_H

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

struct Project {
    string name;
    bool isApplication;
    vector<string> source;
    vector<string> includes;
};

ostream& operator<<(ostream&, const Project&);

#endif