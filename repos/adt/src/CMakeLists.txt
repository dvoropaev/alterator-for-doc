if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "e2k")
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-undef -fvisibility=default -pedantic -Wall"
    )
  else()
      message("Is GNU Compiler.")
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-undef -fvisibility=hidden \
      -fvisibility-inlines-hidden -pedantic -Werror=sign-compare \
      -Werror=old-style-cast -Wall -Wextra -Wno-long-long \
      -Weffc++ -Werror -Wall -Wextra -pedantic -Wcast-align -Wcast-qual \
      -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self \
      -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast \
      -Woverloaded-virtual -Wredundant-decls -Wshadow -Wstrict-null-sentinel \
      -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses \
      -fdiagnostics-show-option"
    )
  endif()
endif()

set(QT_VERSION_MAJOR 6)

macro(add_adt_executable name)
  add_executable(${name} ${ARGN})

  # add install target
  set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name})
  install(TARGETS ${name} DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
    GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

  # copy binary
  add_custom_command(TARGET ${name} POST_BUILD COMMAND
    ${CMAKE_COMMAND} ARGS -E copy
      $<TARGET_FILE:${name}>
      ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${name}>
    COMMENT "Copy ${name} to ${CMAKE_BINARY_DIR}/bin")
endmacro()


set(QT_VERSION_MAJOR 6)
find_package(tomlplusplus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui DBus Xml LinguistTools REQUIRED)

configure_file(app/version.h.in version.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
  app/adtapp.h
  app/adtcommandlineparser.h
  app/adtcommandlineoptions.h
  app/adtsettings.h
  app/adtexecutor.h

  controllers/basecontroller.h
  controllers/guicontroller.h
  controllers/clicontroller.h

  gui/mainwindow.h
  gui/maintoolswidget.h
  gui/maintestswidget.h
  gui/statuscommonwidget.h
  gui/detailsdialog.h
  gui/stopdialog.h
  gui/serviceunregisteredwidget.h
  gui/toolswidgetinterface.h
  gui/testswidgetinterface.h
  gui/mainwindowinterface.h
  gui/loadingwidget.h
  gui/widgetstatus.h
  gui/toolwidget.h
  gui/varstableview.h
  gui/AboutDialog.h

  model/builder/adtmodelbuilderinterface.h
  model/builder/adtmodelbuilderdbus.h
  model/builder/dbusservicewatcher.h
  model/builder/dbusconfig.h
  model/treemodel.h
  model/treeitem.h

  model/adttool.h
  model/adttest.h
  model/builder/adttoolbuilder.h
  model/builder/adtbuilderconstants.h
  model/builder/adtbuilderhelper.h
  model/builder/adttoolvarsmodelbuilder.h
  model/builder/adttoolvarsmodelrowbuilders.h
  model/builder/adtvarmodelbuilderinterface.h
  model/builder/adtvarmodelbuildfactory.h
  model/tooltabviewdelegate.h

  vars/adtvarsinterface.h
  vars/adtvar.h
  vars/adtvarbuilderinterface.h
  vars/adtvarbuilderfactory.h
  vars/adtvarbuilderint.h
  vars/adtvarbuilderstring.h
  vars/adtvarbuilderenum.h
)

set(SOURCES
  app/main.cpp
  app/adtapp.cpp
  app/adtcommandlineparser.cpp
  app/adtsettings.cpp
  app/adtexecutor.cpp

  controllers/basecontroller.cpp
  controllers/guicontroller.cpp
  controllers/clicontroller.cpp

  gui/mainwindow.cpp
  gui/maintoolswidget.cpp
  gui/maintestswidget.cpp
  gui/statuscommonwidget.cpp
  gui/detailsdialog.cpp
  gui/stopdialog.cpp
  gui/serviceunregisteredwidget.cpp
  gui/loadingwidget.cpp
  gui/toolwidget.cpp
  gui/varstableview.cpp
  gui/AboutDialog.cpp

  model/builder/adtmodelbuilderdbus.cpp
  model/builder/dbusservicewatcher.cpp
  model/builder/adtbuilderhelper.cpp
  model/builder/adttoolvarsmodelbuilder.cpp
  model/builder/adttoolvarsmodelrowbuilders.cpp
  model/builder/adtvarmodelbuildfactory.cpp
  model/tooltabviewdelegate.cpp

  model/treemodel.cpp
  model/treeitem.cpp

  model/adttool.cpp
  model/adttest.cpp
  model/builder/adttoolbuilder.cpp

  vars/adtvarsinterface.cpp
  vars/adtvar.cpp

  vars/adtvarbuilderfactory.cpp
  vars/adtvarbuilderint.cpp
  vars/adtvarbuilderstring.cpp
  vars/adtvarbuilderenum.cpp
)

set(UI_FORMS
  gui/mainwindow.ui
  gui/maintoolswidget.ui
  gui/maintestswidget.ui
  gui/statuscommonwidget.ui
  gui/detailsdialog.ui
  gui/serviceunregistered.ui
  gui/stopdialog.ui
  gui/toolwidget.ui
  gui/AboutDialog.ui
)

set(RESOURCES
  gui/icon.qrc
)

set(TS_FILES
  ${CMAKE_SOURCE_DIR}/ts/adt_en.ts
  ${CMAKE_SOURCE_DIR}/ts/adt_ru.ts
)

add_translation(QM_FILES ${TS_FILES})
add_translation_resource(RESOURCES_SRC "adt" ${QM_FILES})
qt_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})

qt_wrap_ui(UI_SOURCES ${UI_FORMS})
add_adt_executable(adt ${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/setup/ru.basealt.adt.desktop
  DESTINATION share/applications
)
install(FILES ${CMAKE_SOURCE_DIR}/setup/adt.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES ${CMAKE_SOURCE_DIR}/setup/adt.backend
  DESTINATION /usr/share/alterator/backends
)

target_link_libraries(adt
  Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Gui tomlplusplus
)
