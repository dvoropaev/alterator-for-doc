macro(add_alterator_explorer_executable name)
    # add executable
    add_executable(${name} ${ARGN})

    # add install target
    set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name})
    install(TARGETS ${name} DESTINATION bin
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # copy binary
    add_custom_command(TARGET ${name} POST_BUILD COMMAND
      ${CMAKE_COMMAND} ARGS -E copy
        $<TARGET_FILE:${name}>
        ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${name}>
      COMMENT "Copy ${name} to ${CMAKE_BINARY_DIR}/bin")
endmacro()

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets Core Gui DBus Xml LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui DBus Xml LinguistTools REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMGenerateDBusServiceFile)
find_package(KF6 REQUIRED COMPONENTS DBusAddons WindowSystem WidgetsAddons REQUIRED QUIET)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    mainwindow.h
    categorywidget.h
    flowlayout.h
    controller.h
    pushbutton.h
    mainwindowsettings.h

    model/model.h


)

set(SOURCES
    main.cpp
    mainwindow.cpp
    categorywidget.cpp
    flowlayout.cpp
    controller.cpp
    pushbutton.cpp
    mainwindowsettings.cpp

    model/model.cpp

)

set(UI_FORMS
    mainwindow.ui
    categorywidget.ui
)
set(TS_FILES
${CMAKE_CURRENT_SOURCE_DIR}/ru.ts
${CMAKE_CURRENT_SOURCE_DIR}/en.ts
)

add_custom_target(app_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} * -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -no-obsolete
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_wrap_ui(UI_SOURCES ${UI_FORMS})
    qt_add_translation(QM_FILES ${TS_FILES})
    add_translation_resource(RESOURCES_SRC "alterator-explorer" ${QM_FILES})
    qt_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})
else()
    qt5_wrap_ui(UI_SOURCES ${UI_FORMS})
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_translation_resource(RESOURCES_SRC "alterator-explorer" ${QM_FILES})
    qt5_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})
endif()



add_alterator_explorer_executable(
    alterator-explorer ${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES}
)

target_link_libraries(alterator-explorer alterator-explorer-core aobuilder
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Gui
    KF6::WindowSystem
    KF6::DBusAddons
    KF6::WidgetsAddons
)

ecm_generate_dbus_service_file(
    NAME org.altlinux.alterartor-explorer
    EXECUTABLE ${CMAKE_INSTALL_FULL_BINDIR}/alterator-explorer
    DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/dbus-1/services/
)
