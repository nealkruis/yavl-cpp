project(yavl-cpp)
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules (YAML REQUIRED yaml-cpp)

include_directories( include/yavl-cpp /usr/include/yaml-cpp )
link_libraries( yaml-cpp )

add_library(yavl-cpp SHARED src/yavl.cpp include/yavl-cpp/yavl.h)
