cmake_minimum_required(VERSION 3.20)
project(executor LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO gio-2.0 REQUIRED)
pkg_check_modules(POLKIT polkit-gobject-1 REQUIRED)
pkg_check_modules(LIBTOML libtoml REQUIRED)
pkg_check_modules(JSONC json-c REQUIRED)

include_directories(${GIO_INCLUDE_DIRS}
                    ${POLKIT_INCLUDE_DIRS}
                    ${LIBTOML_INCLUDE_DIRS}
                    ${JSONC_INCLUDE_DIRS})

link_directories(${GIO_LIBRARY_DIRS})

add_library(executor SHARED executor.c)

target_link_libraries(executor ${GIO_LIBRARIES}
                               ${POLKIT_LIBRARIES}
                               ${LIBTOML_LIBRARIES}
                               ${JSONC_LIBRARIES})

install(TARGETS executor DESTINATION /usr/libexec/alterator)
