cmake_minimum_required(VERSION 3.18)

message(FATAL_ERROR "-- cs2int > Dont use.")

set(DXSDK_PATH "" CACHE PATH "Path to your DirectX SDK Installation.")
if (DXSDK_PATH STREQUAL "")
  if (NOT DEFINED ENV{DXSDK_DIR})
    message(FATAL_ERROR "-- cs2int > DXSDK_DIR env was not found. Please manually set DXSDK_PATH in your CMakeCache.txt or through -DDXSDK_PATH=path")
  endif()
  set(DXSDK_PATH "$ENV{DXSDK_DIR}")
  string(REPLACE "\\" "/" DXSDK_PATH "${DXSDK_PATH}")
endif()

if (NOT EXISTS "${DXSDK_PATH}Include/D3D11.h")
  message(FATAL_ERROR "-- cs2int > DirectX SDK path was not found. (${DXSDK_PATH})")
endif()

project(dxsdk)

add_library(
  ${PROJECT_NAME}
  STATIC
  "${DXSDK_PATH}Include/D3D11.h"
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  LINKER_LANGUAGE CXX
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  "${DXSDK_PATH}Lib/x64/d3d11.lib"
  "${DXSDK_PATH}Lib/x64/dxgi.lib"
  "${DXSDK_PATH}Lib/x64/dxguid.lib"
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  "${DXSDK_PATH}Include"
)
