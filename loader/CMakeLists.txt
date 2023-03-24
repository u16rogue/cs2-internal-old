cmake_minimum_required(VERSION 3.18)

if (NOT TARGET client)
  message(FATAL_ERROR "-- cs2int > The target 'client' in unavailable for the loader target. Loader requires information from the client target.")
endif()

project(loader)

add_executable(
  ${PROJECT_NAME}
  "main.cpp"
)

get_target_property(CS2INT_LOADER_CLIENT_FNAME client OUTPUT_NAME)
get_target_property(CS2INT_LOADER_CLIENT_EXTEN client SUFFIX)
set(CS2INT_LOADER_TARGET_FNAME "${CS2INT_LOADER_CLIENT_FNAME}${CS2INT_LOADER_CLIENT_EXTEN}")
set(CS2INT_LOADER_TARGET_LIB "${CMAKE_BINARY_DIR}/${CS2INT_LOADER_TARGET_FNAME}")
message("-- cs2int > Loader target library @ ${CS2INT_LOADER_TARGET_LIB}")

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
  CS2INT_TARGET_FNAME="${CS2INT_LOADER_TARGET_FNAME}"
  CS2INT_TARGET_LIB="${CS2INT_LOADER_TARGET_LIB}"
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  CXX_STANDARD 20
  OUTPUT_NAME "load"
  SUFFIX ".exe"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  common
  metapp
)
