#include <iostream>

#include "parse_project.h"

using std::cerr;
using std::cout;
using std::endl;
using tinyxml2::XMLDocument;

int main(int argc, const char* argv[]) {
  if (argc == 1) {
    cerr << "usage: msbuildtocmake project.vcxproj" << endl;
    return 1;
  }

  XMLDocument doc;
  doc.LoadFile(argv[1]);
  auto p = parse_project(doc.RootElement());

  cout << *p;

  return 0;
}
