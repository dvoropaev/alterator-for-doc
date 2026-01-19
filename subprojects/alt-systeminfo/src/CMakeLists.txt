macro(add_alterator_application_systeminfo_executable name)
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

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)

configure_file(utility/debug.h.in utility/debug.h)
configure_file(app/version.h.in app/version.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
  main.cpp
  app/application.cpp

  controller/maincontroller.cpp
  controller/editioncontroller.cpp

  databuilder/databuilder.cpp

  ui/mainwindow/mainwindow.cpp
  ui/propertytableview/propertytableview.cpp
  ui/operationsystempropertieswidget/operationsystempropertieswidget.cpp
  ui/hardwarepropertieswidget/hardwarepropertieswidget.cpp
  ui/usefulsourceswidget/usefulsourceswidget.cpp
  ui/editionswidget/editionfieldwidget.cpp
  ui/settingswidget/settingswidget.cpp

  ui/dialogs/licensedialog/licensedialog.cpp
  ui/dialogs/editionwizard/editionwizard.cpp
  ui/dialogs/editionwizard/selectionwizardpage.cpp
  ui/dialogs/editionwizard/agreementwizardpage.cpp

  interface/managerinterface.cpp
  interface/systeminfointerface.cpp
  interface/aptinterface.cpp
  interface/currenteditioninterface.cpp
  interface/editioninterface.cpp

  entity/product.cpp
)

set(HEADERS
  app/application.h

  controller/maincontroller.h
  controller/editioncontroller.h

  databuilder/databuilder.h

  ui/mainwindow/mainwindow.h
  ui/propertytableview/propertytableview.h
  ui/operationsystempropertieswidget/operationsystempropertieswidget.h
  ui/usefulsourceswidget/usefulsourceswidget.h
  ui/hardwarepropertieswidget/hardwarepropertieswidget.h
  ui/editionswidget/editionfieldwidget.h
  ui/settingswidget/settingswidget.h

  ui/dialogs/licensedialog/licensedialog.h
  ui/dialogs/editionwizard/editionwizard.h
  ui/dialogs/editionwizard/selectionwizardpage.h
  ui/dialogs/editionwizard/agreementwizardpage.h

  utility/constants.h
  utility/debug.h

  interface/managerinterface.h
  interface/systeminfointerface.h
  interface/systeminfointerface.inl
  interface/aptinterface.h
  interface/currenteditioninterface.h
  interface/editioninterface.h

  entity/product.h
  entity/edition.h
  entity/branch.h

)

set(UI_FORMS
  ui/mainwindow/mainwindow.ui
  ui/propertytableview/propertytableview.ui
  ui/operationsystempropertieswidget/operationsystempropertieswidget.ui
  ui/hardwarepropertieswidget/hardwarepropertieswidget.ui
  ui/usefulsourceswidget/usefulsourceswidget.ui
  ui/editionswidget/editionfieldwidget.ui
  ui/settingswidget/settingswidget.ui

  ui/dialogs/licensedialog/licensedialog.ui
  ui/dialogs/editionwizard/editionwizard.ui
  ui/dialogs/editionwizard/selectionwizardpage.ui
  ui/dialogs/editionwizard/agreementwizardpage.ui
)

set(RESOURCES
  logo.qrc
)

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ru.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/en.ts
)

set(APP_ICONS_DIR "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/alt-systeminfo.svg")

add_custom_target(app_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} * -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -no-obsolete
)

set_source_files_properties(${TS_FILES}
  PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}
)

qt_wrap_ui(UI_SOURCES ${UI_FORMS})
qt_add_translation(QM_FILES ${TS_FILES})
add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})
qt_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})

add_alterator_application_systeminfo_executable(${PROJECT_NAME}
  ${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::DBus
)

install(
    FILES ${APP_ICON}
    DESTINATION ${APP_ICONS_DIR}
)
