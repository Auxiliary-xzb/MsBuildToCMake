cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project(msbuildtocmake LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
include(CTest)

FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_Declare(
    tinyxml
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG 9.0.0
)

FetchContent_Declare(
    tinyxmlex
    GIT_REPOSITORY https://github.com/stanthomas/tinyxml2-ex.git
)

FetchContent_MakeAvailable(gtest tinyxml tinyxmlex)

include_directories(${tinyxml_SOURCE_DIR} ${tinyxmlex_SOURCE_DIR})

add_executable(${PROJECT_NAME}
    project.cpp
    parse_project.cpp
    main.cpp
    ${tinyxml2_SOURCE_DIR}/tinyxml2.cpp
)