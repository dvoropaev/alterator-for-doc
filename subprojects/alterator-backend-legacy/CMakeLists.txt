cmake_minimum_required(VERSION 3.5)

project("alterator-application-legacy_dialog")
set(VERSION "0.1.0")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(${VERSION})

string(REPLACE "." ";" VERSION_LIST ${VERSION})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_MODULE_PATH}/TranslationUtils.cmake)

include(CheckCXXCompilerFlag)

if(UNIX)
  include(GNUInstallDirs)
else()
  message(FATAL_ERROR "System not Linux")
endif()

add_subdirectory(src)
