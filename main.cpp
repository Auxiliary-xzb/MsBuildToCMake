#include <iostream>

#include "visual_studio_project.h"

int main(int argc, const char* argv[]) {
  if (argc == 1) {
    std::cerr << "usage: " << argv[0] << " <project.vcxproj>" << std::endl;
    return 1;
  }

  axsp::VisualStudioProject visual_studio_project;
  visual_studio_project.ParseFromFile(argv[1]);

  return 0;
}
