cmake_minimum_required(VERSION 3.10)

project("alt-components")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
		COMMAND /bin/sh -c "grep 'Version:' alt-components.spec | awk -F ':' '{ print $2 }' | tr -d [:space:]"
		OUTPUT_VARIABLE GEAR_VERSION
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
message(${GEAR_VERSION})

string(REPLACE "." ";" VERSION_LIST ${GEAR_VERSION})
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
add_subdirectory(alterator)
add_subdirectory(setup)
