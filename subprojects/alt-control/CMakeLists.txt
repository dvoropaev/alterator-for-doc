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

include_directories("src")

set(PROJECT_SOURCES
    src/ControlForm.cpp
    src/ControlForm.ui
    src/Controller.h
    src/DBusDataSource.h
    src/MainWindow.cpp
    src/MainWindow.ui
    src/ControlForm.h
    src/Controller.cpp
    src/DBusDataSource.cpp
    src/main.cpp
    src/MainWindow.h
)

add_custom_target(app_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} * -recursive ${CMAKE_CURRENT_BINARY_DIR} -ts ${TS_FILES} -no-obsolete
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR})


include(cmake/TranslationUtils.cmake)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${RESOURCES_SRC}
)

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
