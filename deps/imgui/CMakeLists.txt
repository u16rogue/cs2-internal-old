cmake_minimum_required(VERSION 3.18)
# why doesnt imgui just provide a fucking target????????? hello????

set(IMGUI_PATH "" CACHE PATH "Path to your Local ImGui repository.")
if (NOT EXISTS "${IMGUI_PATH}/imgui.h" OR NOT EXISTS "${IMGUI_PATH}/imgui.cpp")
  message(FATAL_ERROR "-- cs2int > Invalid IMGUI_PATH! Please set it to a valid ImGui repository directory. (Files imgui.h and imgui.cpp not found)")
endif()

project(imgui)

add_library(
  ${PROJECT_NAME}
  STATIC
  "${IMGUI_PATH}/imconfig.h"
  "${IMGUI_PATH}/imgui.cpp"
  "${IMGUI_PATH}/imgui.h"
  "${IMGUI_PATH}/imgui_draw.cpp"
  "${IMGUI_PATH}/imgui_internal.h"
  "${IMGUI_PATH}/imgui_tables.cpp"
  "${IMGUI_PATH}/imgui_widgets.cpp"
  "${IMGUI_PATH}/backends/imgui_impl_win32.h"
  "${IMGUI_PATH}/backends/imgui_impl_win32.cpp"
  "${IMGUI_PATH}/backends/imgui_impl_dx11.h"
  "${IMGUI_PATH}/backends/imgui_impl_dx11.cpp"
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  "${IMGUI_PATH}"
  "${IMGUI_PATH}/backends"
)

# target_link_libraries(
#   ${PROJECT_NAME}
#   PRIVATE
#   dxsdk
# )
