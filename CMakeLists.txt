cmake_minimum_required(VERSION 3.5)

project("alterator-explorer")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
    COMMAND
    bash -c "grep 'Version:' .gear/alterator-explorer.spec | awk -F ':' '{ print $2 }' | tr -d [:space:]"
    OUTPUT_VARIABLE ALTCENTER_GEAR_VERSION
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
message (${ALTCENTER_GEAR_VERSION})

string(REPLACE "." ";" ADT_VERSION_LIST ${ALTCENTER_GEAR_VERSION})
list(GET ALTCENTER_VERSION_LIST 0 ALTCENTER_VERSION_MAJOR)
list(GET ALTCENTER_VERSION_LIST 1 ALTCENTER_VERSION_MINOR)
list(GET ALTCENTER_VERSION_LIST 2 ALTCENTER_VERSION_PATCH)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_MODULE_PATH}/TranslationUtils.cmake)

include(CheckCXXCompilerFlag)

if(UNIX)
  include(GNUInstallDirs)
else()
    message(FATAL_ERROR "System not Linux")
endif()

add_subdirectory(src)

install(FILES ${CMAKE_SOURCE_DIR}/categories/adt.category
  DESTINATION /usr/share/alterator/categories
)
install(FILES ${CMAKE_SOURCE_DIR}/categories/components.category
  DESTINATION /usr/share/alterator/categories
)
