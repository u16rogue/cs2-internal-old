cmake_minimum_required(VERSION 3.18)

project(client)

add_library(
  ${PROJECT_NAME}
  SHARED
  "global.hpp"
  "main.cpp"
  "hooks.hpp"
  "hooks.cpp"
  "game.hpp"
  "game.cpp"
  "utils.hpp"
  "utils.cpp"

  "menu/menu.hpp"
  "menu/menu.cpp"
  "menu/tabs.hpp"
  "menu/tabs/debug.hpp"
  "menu/tabs/debug.cpp"

  "cs2/structs.hpp"
  "cs2/convar.hpp"
  "cs2/convar.cpp"
  "cs2/interface.hpp"
  "cs2/interface.cpp"
  "cs2/cutl.hpp"
  "cs2/cutl.cpp"
  "cs2/inputservice.hpp"
  "cs2/inputservice.cpp"
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
  #dxsdk
  imgui
  # kahel_winpe
)

# Makes <client/...> available. Prefer this instead of nesting [..]
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  "${CMAKE_SOURCE_DIR}"
)
