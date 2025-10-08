cmake_minimum_required(VERSION 3.10)

project("alt-packages")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
    COMMAND
    bash -c "grep 'Version:' alt-packages.spec\
            | awk -F ':' '{ print $2 }'\
            | tr -d [:space:]"
    OUTPUT_VARIABLE GEAR_VERSION
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

string(REPLACE "." ";" VERSION_LIST ${GEAR_VERSION})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(TranslationUtils)

if(UNIX)
  include(GNUInstallDirs)
else()
  message(FATAL_ERROR "System not UNIX - not implemented yet")
endif()

add_subdirectory(src)
add_subdirectory(desktop)
add_subdirectory(alterator)
