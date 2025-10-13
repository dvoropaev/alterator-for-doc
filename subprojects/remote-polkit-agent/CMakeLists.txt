cmake_minimum_required(VERSION 3.20)
project(remote-polkit-agent LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO gio-2.0 REQUIRED)
pkg_check_modules(POLKIT polkit-gobject-1 REQUIRED)
pkg_check_modules(POLKIT_AGENT polkit-agent-1 REQUIRED)

include_directories(${GIO_INCLUDE_DIRS} ${POLKIT_INCLUDE_DIRS})
link_directories(${GIO_LIBRARY_DIRS})

set(SOURCES
    main.c
    polkitagentremotelistener.c)

add_executable(remote-polkit-agent ${SOURCES})

target_link_libraries(remote-polkit-agent ${GIO_LIBRARIES}
                                          ${POLKIT_LIBRARIES}
                                          ${POLKIT_AGENT_LIBRARIES})

target_compile_definitions(remote-polkit-agent PRIVATE -D_POLKIT_AGENT_COMPILATION)

install(TARGETS remote-polkit-agent DESTINATION /usr/bin)
