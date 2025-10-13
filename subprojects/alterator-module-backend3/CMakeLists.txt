cmake_minimum_required(VERSION 3.20)
project(executor LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO gio-2.0 REQUIRED)
pkg_check_modules(POLKIT polkit-gobject-1 REQUIRED)
pkg_check_modules(LIBTOML libtoml REQUIRED)

include_directories(${GIO_INCLUDE_DIRS}
                    ${POLKIT_INCLUDE_DIRS}
                    ${LIBTOML_INCLUDE_DIRS})

link_directories(${GIO_LIBRARY_DIRS})

add_library(backend3 SHARED backend3.c)

target_link_libraries(backend3 ${GIO_LIBRARIES}
                               ${POLKIT_LIBRARIES}
                               ${LIBTOML_LIBRARIES})

install(TARGETS backend3 DESTINATION /usr/libexec/alterator)
