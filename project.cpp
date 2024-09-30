#include "project.h"

#include <algorithm>
#include <ranges>
#include <string_view>

std::ostream& operator<<(std::ostream& out,
                         const std::vector<std::string>& lst) {
  if (out) {
    auto work(lst);
    std::ranges::sort(work);
    const auto [first, last] = std::ranges::unique(work);
    work.erase(first, last);
    for (auto s : work) {
      out << "    " << s << std::endl;
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Project& p) {
  if (out) {
    if (p.name != "") {
      out << "project(" << p.name << " LANGUAGES CXX)" << std::endl
          << std::endl;
    }

    if (p.standard != LanguageStandard::unspecified) {
      out << "set(CXX_STANDARD ";
      switch (p.standard) {
        case LanguageStandard::cpp2x:
          out << "2x";
          break;
        case LanguageStandard::cpp11:
          out << "11";
          break;
        case LanguageStandard::cpp14:
          out << "14";
          break;
        case LanguageStandard::cpp17:
          out << "17";
          break;
        case LanguageStandard::cpp20:
          out << "20";
          break;
        case LanguageStandard::cpp23:
          out << "23";
          break;
      }
      out << ")" << std::endl;
      out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)" << std::endl << std::endl;
    }

    if (p.isApplication) {
      out << "add_executable";
    } else {
      out << "add_library";
    }
    out << "(${PROJECT_NAME}" << std::endl;
    out << p.source;
    out << ")" << std::endl;

    if (!p.includes.empty()) {
      out << "target_include_directories(${PROJECT_NAME} PUBLIC" << std::endl;
      out << p.includes;
      out << ")" << std::endl;
    }
  }
  return out;
}
