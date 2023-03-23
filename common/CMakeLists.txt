cmake_minimum_required(VERSION 3.18)

project(common)

add_library(
  ${PROJECT_NAME}
  STATIC
  "include/common/logging.hpp"
  "src/logging.cpp"

  "include/common/types.hpp"
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  CXX_STANDARD 20
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  "include/"
)

if (CS2INT_LOGGING)
  target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC 
    CS2INT_COMMON_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
  )
  message("-- cs2int > Current source directory is ${CMAKE_SOURCE_DIR}")
endif()
