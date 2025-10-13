cmake_minimum_required(VERSION 3.5)

project(alteratorctl LANGUAGES C)

if(UNIX)
    include(GNUInstallDirs)
else()
    message(FATAL_ERROR "System not UNIX - not implemented yet")
endif()

execute_process(
  COMMAND bash -c "grep 'Version:' ./alteratorctl.spec \
                    | awk -F: '{ print $2 }' \
                    | tr -d [:space:]"
  OUTPUT_VARIABLE ALTERATORCTL_GEAR_VERSION
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

string(REPLACE "." ";" ALTERATORCTL_VERSION_LIST ${ALTERATORCTL_GEAR_VERSION})
list(GET ALTERATORCTL_VERSION_LIST 0 ALTERATORCTL_VERSION_MAJOR)
list(GET ALTERATORCTL_VERSION_LIST 1 ALTERATORCTL_VERSION_MINOR)
list(GET ALTERATORCTL_VERSION_LIST 2 ALTERATORCTL_VERSION_PATCH)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(Glib20 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(DBUS gio-2.0 dbus-1 dbus-glib-1)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBTOMLC99 REQUIRED libtoml)
pkg_check_modules(LIBGUMBO REQUIRED gumbo)
pkg_check_modules(LIBPOLKIT REQUIRED polkit-agent-1 polkit-gobject-1)

configure_file(version.h.in version.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${LIBPOLKIT_INCLUDE_DIRS})
include_directories(${JSON_GLIB_INCLUDE_DIRS})

add_definitions(${GLIB_CFLAGS_OTHER})

link_directories(${DBUS_LIBRARY_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
link_directories(${LIBPOLKIT_LIBRARY_DIRS})

set(HEADERS
    alteratorctlapp.h
    alteratorctlmoduleinterface.h
    alteratorctlgdbussource.h
    alteratorctlcommon.h
    alteratorctlmoduleinfoparser.h

    alteratorctlmanagermodule.h
    alteratorctlcomponentsmodule.h
    alteratorctlpackagesmodule.h
    alteratorctleditionsmodule.h
    alteratorctldiagmodule.h
    alteratorctlsysteminfomodule.h
    alteratorctlremotemodule.h
)
set(SOURCES
    alteratorctlmoduleinterface.c
    alteratorctlapp.c
    alteratorctlgdbussource.c
    alteratorctlcommon.c
    alteratorctlmoduleinfoparser.c

    alteratorctlmanagermodule.c
    alteratorctlcomponentsmodule.c
    alteratorctlpackagesmodule.c
    alteratorctleditionsmodule.c
    alteratorctldiagmodule.c
    alteratorctlsysteminfomodule.c
    alteratorctlremotemodule.c

    main.c
)

add_subdirectory(po)
add_subdirectory(completions)

add_executable(alteratorctl ${HEADERS} ${SOURCES}
)
add_dependencies(alteratorctl
    generated_files
)
target_include_directories(alteratorctl PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(alteratorctl PUBLIC ${DBUS_LIBRARIES} PkgConfig::Glib20 ${GLIB_LIBRARIES}
                                          ${JSON_GLIB_LIBRARIES} ${LIBTOMLC99_LIBRARIES} ${LIBGUMBO_LIBRARIES}
                                          ${LIBPOLKIT_LIBRARIES} ${GLIB_LDFLAGS})

install(TARGETS alteratorctl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
