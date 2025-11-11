cmake_minimum_required(VERSION 3.5)

project(alt-control VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets DBus LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus LinguistTools)

set(TS_FILES
    ts/en_US.ts
    ts/ru_RU.ts
)

include_directories("src" ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SOURCES
    src/main.cpp

    # app
    src/app/ControlApp.h
    src/app/ControlApp.cpp

    src/app/AppSettings.h
    src/app/AppSettings.cpp

    # controller
    src/controller/Controller.h
    src/controller/Controller.cpp

    src/controller/DBusDataSource.h
    src/controller/DBusDataSource.cpp

    # data
    src/data/TranslatableObject.h
    src/data/TranslatableObject.cpp

    src/data/Facilty.h
    src/data/Facilty.cpp

    src/data/FacilityModel.h
    src/data/FacilityModel.cpp

    # ui
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.ui

    src/ui/AboutDialog.h
    src/ui/AboutDialog.cpp
    src/ui/AboutDialog.ui

    src/ui/ObjectInfoWidget.h
    src/ui/ObjectInfoWidget.cpp
    src/ui/ObjectInfoWidget.ui

    src/ui/ObjectInfoDelegate.h
    src/ui/ObjectInfoDelegate.cpp

    src/ui/StateForm.h
    src/ui/StateForm.cpp
    src/ui/StateForm.ui
)

add_custom_target(app_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} * -recursive ${CMAKE_CURRENT_BINARY_DIR} -ts ${TS_FILES} -no-obsolete
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR})


include(cmake/TranslationUtils.cmake)
include(cmake/Version.cmake)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${RESOURCES_SRC}
)

target_link_libraries(alt-control PRIVATE Qt6::Core)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
