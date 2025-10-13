set(INTERFACES_DIR "share/dbus-1/interfaces")
set(ACTIONS_DIR "share/polkit-1/actions")
set(DESKTOPS_DATADIR "share/applications")

file(GLOB INTERFACES "${PROJECT_SOURCE_DIR}/setup/*.xml")
file(GLOB ACTIONS "${PROJECT_SOURCE_DIR}/setup/*.policy")
file(GLOB DESKTOPS "${PROJECT_SOURCE_DIR}/setup/*.desktop")

install(FILES ${INTERFACES} DESTINATION ${INTERFACES_DIR})
install(FILES ${ACTIONS} DESTINATION ${ACTIONS_DIR})
install(FILES ${DESKTOPS} DESTINATION ${DESKTOPS_DATADIR})
