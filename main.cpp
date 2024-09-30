#include <iostream>

#include "parse_project.h"

int main(int argc, const char* argv[]) {
  if (argc == 1) {
    std::cerr << "usage: " << argv[0] << "<project.vcxproj>" << std::endl;
    return 1;
  }

  tinyxml2::XMLDocument document;
  document.LoadFile(argv[1]);
  auto p = parse_project(document.RootElement());

  std::cout << *p;

  return 0;
}
