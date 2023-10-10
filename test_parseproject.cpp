#include <gtest/gtest.h>
#include "parse_project.h"


TEST(AdditionalIncludes, GivenListOfDirectoriesWithoutVariables_PopulatesDirectories) {
    auto p = make_shared<Project>();

    parse_additionalincludes(p, "C:\\First;C:\\Second");

    ASSERT_EQ(p->includes.size(), 2);
    ASSERT_EQ(p->includes[0], "C:\\First");
    ASSERT_EQ(p->includes[1], "C:\\Second");
}