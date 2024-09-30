#ifndef MS_BUILD_TO_CMAKE_PROJECT_H_
#define MS_BUILD_TO_CMAKE_PROJECT_H_

#include <iostream>
#include <string>
#include <vector>

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
  std::string name;
  bool isApplication;
  LanguageStandard standard = LanguageStandard::unspecified;

  std::vector<std::string> source;
  std::vector<std::string> includes;
};

std::ostream& operator<<(std::ostream&, const Project&);

#endif  // MS_BUILD_TO_CMAKE_PROJECT_H_
