cmake_minimum_required(VERSION 3.10)

project("alt-systeminfo"
        VERSION 0.4.6
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG true)
else()
    set(DEBUG false)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_MODULE_PATH}/TranslationUtils.cmake)

include(CheckCXXCompilerFlag)

if(UNIX)
  include(GNUInstallDirs)
else()
  message(FATAL_ERROR "System not Linux")
endif()

add_subdirectory(src)
add_subdirectory(alterator)
add_subdirectory(desktop)
