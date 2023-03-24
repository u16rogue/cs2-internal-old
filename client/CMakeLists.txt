cmake_minimum_required(VERSION 3.18)

project(client)

add_library(
  ${PROJECT_NAME}
  SHARED
  "main.cpp"

  "hooks.hpp"
  "hooks.cpp"
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  CXX_STANDARD 20
  OUTPUT_NAME "cs2client"
  SUFFIX ".dll"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  metapp
  common
  minhook
  # kahel_winpe
)
