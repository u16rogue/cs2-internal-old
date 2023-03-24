cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(CS2INT_LOGGING "Enables the logging macro to be enabled (cs2log)" ON)

if (CS2INT_LOGGING)
  add_compile_definitions(CS2INT_COMMON_LOGGING=1)
  message("-- cs2int > Logging is enabled! cs2log is available for use.")
else()
  add_compile_definitions(CS2INT_COMMON_LOGGING=0)
  message("-- cs2int > Logging is disabled!")
endif()

project(cs2internal)

#add_subdirectory("deps/kahel-s2kit")
#add_subdirectory("deps/kahel-winpe")
add_subdirectory("deps/metapp")
add_subdirectory("deps/minhook")

add_subdirectory("common")
add_subdirectory("client")
add_subdirectory("loader")
