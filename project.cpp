#include "project.h"
#include <ranges>
#include <algorithm>
#include <string_view>

using std::endl;
using namespace std;
using namespace std::views;

ostream& operator<<(ostream& out, const vector<string>& lst) {
    if (out) {
        auto work(lst);
        ranges::sort(work);
        const auto [first, last] = ranges::unique(work);
        work.erase(first, last);
        for(auto s: work) {
            out << "    " << s << endl;
        }
    }
    return out;
}

ostream& operator<<(ostream& out, const Project& p) {
    if (out) {

        if (p.name != "") {
            out << "project(" << p.name << " LANGUAGES CXX)" << endl;
        }

        if (p.isApplication) out << "add_application";
        else out << "add_library";
        out << "(${PROJECT_NAME}" <<  endl;
        out << p.source;
        out << ")" << endl;

        if (!p.includes.empty()) {
            out << "target_include_directories(${PROJECT_NAME} PUBLIC" << endl;
            out << p.includes;
            out << ")" << endl;
        }

    }
    return out;
}