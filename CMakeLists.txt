cmake_minimum_required(VERSION 3.13)
project(playgroundpp)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.12.1
  VERSION 1.12.1
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

include_directories(include)

add_subdirectory(metaprogramming_15puzzle)
add_subdirectory(15puzzle)
add_subdirectory(group_comparator)
add_subdirectory(callbacks)
add_subdirectory(go_chanel_clone)
add_subdirectory(coroutines)