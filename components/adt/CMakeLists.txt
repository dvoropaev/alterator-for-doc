cmake_minimum_required(VERSION 3.5)

project("ALT Diagnostic tool")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

execute_process(
  COMMAND bash -c "grep 'Version:' .gear/adt.spec \
                    | awk -F: '{ print $2 }' \
                    | tr -d [:space:]"
  OUTPUT_VARIABLE ADT_GEAR_VERSION
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

string(REPLACE "." ";" ADT_VERSION_LIST ${ADT_GEAR_VERSION})
list(GET ADT_VERSION_LIST 0 ADT_VERSION_MAJOR)
list(GET ADT_VERSION_LIST 1 ADT_VERSION_MINOR)
list(GET ADT_VERSION_LIST 2 ADT_VERSION_PATCH)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(TranslationUtils)

if(UNIX)
  include(GNUInstallDirs)
else()
  if(WIN32)
    set(${CMAKE_INSTALL_LIBDIR} "lib")
    set(${CMAKE_INSTALL_DATADIR} "share")
    set(${CMAKE_INSTALL_INCLUDEDIR} "include")
    set(${CMAKE_INSTALL_BINDIR} "bin")
    message(STATUS
      "Setting installation destination on Windows to: ${CMAKE_INSTALL_PREFIX}"
    )
  else()
    message(FATAL_ERROR "System not UNIX nor WIN32 - not implemented yet")
  endif()
endif()

add_subdirectory(src)

