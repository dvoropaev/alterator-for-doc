execute_process(
  COMMAND /bin/sh -c "grep 'Version:' .gear/alt-services.spec | awk -F ':' '{ print $2 }' | tr -d [:space:]"
  OUTPUT_VARIABLE GEAR_VERSION
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
message("${PROJECT_NAME} version: ${GEAR_VERSION}")

string(REPLACE "." ";" VERSION_LIST ${GEAR_VERSION})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)

configure_file(src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
