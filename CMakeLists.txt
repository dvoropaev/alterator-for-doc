cmake_minimum_required(VERSION 3.20)
project(alterator-manager LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB glib-2.0 REQUIRED)
pkg_check_modules(GIO gio-2.0 REQUIRED)
pkg_check_modules(GMODULE gmodule-2.0 REQUIRED)
pkg_check_modules(POLKIT polkit-gobject-1 REQUIRED)
pkg_check_modules(LIBTOML libtoml REQUIRED)

pkg_get_variable(SYSTEMUNITDIR systemd systemd_system_unit_dir)
pkg_get_variable(USERUNITDIR systemd systemd_user_unit_dir)

include_directories(${GIO_INCLUDE_DIRS} ${POLKIT_INCLUDE_DIRS} ${LIBTOML_INCLUDE_DIRS})
link_directories(${GIO_LIBRARY_DIRS} ${GMODULE_LIBRARY_DIRS})

set(SOURCES
    src/main.c
    src/alterator_manager_backends.c
    src/alterator_manager_modules.c
    src/alterator_manager_dbus.c
    src/alterator_manager_interface.c
    src/alterator_manager_default_interfaces.c
    src/alterator_manager_polkit.c
    src/alterator_manager_data.c
    src/alterator_manager_sender_environment.c
    src/alterator_manager_backend_monitor.c)

add_executable(alterator-manager ${SOURCES})

target_link_libraries(alterator-manager ${GIO_LIBRARIES}
                                        ${GMODULE_LIBRARIES}
                                        ${POLKIT_LIBRARIES}
                                        ${LIBTOML_LIBRARIES})

target_compile_definitions(alterator-manager PRIVATE G_LOG_DOMAIN="alterator-manager")


add_executable(am-dev-tool src/alterator_manager_dev_tool.c)
target_link_libraries(am-dev-tool ${GIO_LIBRARIES})


set(dbus_config_dir /usr/share/dbus-1/system.d)
set(dbus_services_dir /usr/share/dbus-1/services)
set(dbus_system_services_dir /usr/share/dbus-1/system-services)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/alterator_manager_module_info.h)

install(TARGETS am-dev-tool DESTINATION bin)
install(TARGETS alterator-manager DESTINATION sbin)
install(FILES src/org.altlinux.alterator-manager.conf DESTINATION ${dbus_config_dir})
install(FILES src/org.altlinux.alterator-manager.service DESTINATION ${dbus_system_services_dir})
install(FILES src/org.altlinux.alterator-manager.service-user DESTINATION ${dbus_services_dir})
install(FILES src/alterator-manager.service DESTINATION ${SYSTEMUNITDIR})
install(FILES src/alterator-manager.service-user DESTINATION ${USERUNITDIR})
install(FILES src/org.altlinux.alterator.manager.policy DESTINATION /usr/share/polkit-1/actions)
install(FILES ${HEADERS} DESTINATION /usr/include/alterator)

