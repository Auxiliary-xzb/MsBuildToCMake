#include "project.h"

using std::endl;

ostream& operator<<(ostream& out, const Project& p) {
    if (out) {
        if (p.isApplication) out << "add_application";
        else out << "add_library";
        out << "(" << p.name << endl;

        for(auto s: p.source) {
            out << "    " << s << endl;
        }
        out << ")" << endl;
    }
    return out;
}