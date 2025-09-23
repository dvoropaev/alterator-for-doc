cmake_minimum_required(VERSION 3.20)
project(executor LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO gio-2.0 REQUIRED)
pkg_check_modules(POLKIT polkit-gobject-1 REQUIRED)

include_directories(${GIO_INCLUDE_DIRS} ${POLKIT_INCLUDE_DIRS})
link_directories(${GIO_LIBRARY_DIRS})

add_library(remote SHARED remote.c)

target_link_libraries(remote ${GIO_LIBRARIES} ${POLKIT_LIBRARIES})

install(TARGETS remote DESTINATION /usr/libexec/alterator)
