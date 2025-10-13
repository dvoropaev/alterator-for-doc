set(DESKTOPS_DATADIR "share/applications")

file(GLOB DESKTOPS "${PROJECT_SOURCE_DIR}/desktop/*.desktop")

install(FILES ${DESKTOPS} DESTINATION ${DESKTOPS_DATADIR})
