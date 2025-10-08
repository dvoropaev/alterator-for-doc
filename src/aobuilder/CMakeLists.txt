cmake_minimum_required(VERSION 3.5)

project(aobuilder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Core DBus Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Widgets REQUIRED)

set(HEADERS
    constants.h

    builders/keyprovider.h

    builders/objectbulderinterface.h
    builders/objectbuilderfactory.h
    builders/baseobjectbuilder.h
    builders/localapplicationobjectbuilder.h
    builders/legacyobjectbuilder.h
    builders/categoryobjectbuilder.h
    builders/objectbuilder.h
    builders/aobuilderinterface.h
    builders/aobuilderimpl.h
    builders/diag1objectbuilder.h

    datasource/datasourceinterface.h
    datasource/datasourcedbusimpl.h

    objects/object.h
    objects/category.h
    objects/localappobject.h
    objects/legacyobject.h
    objects/diag1object.h

    parsers/objectparserinterface.h
    parsers/baseobjectparser.h
)

set(SOURCES
    builders/keyprovider.cpp

    builders/objectbuilderfactory.cpp
    builders/baseobjectbuilder.cpp
    builders/localapplicationobjectbuilder.cpp
    builders/legacyobjectbuilder.cpp
    builders/categoryobjectbuilder.cpp
    builders/aobuilderimpl.cpp
    builders/objectbuilder.cpp
    builders/diag1objectbuilder.cpp

    datasource/datasourcedbusimpl.cpp

    objects/object.cpp
    objects/category.cpp
    objects/localappobject.cpp
    objects/legacyobject.cpp
    objects/diag1object.cpp

    parsers/baseobjectparser.cpp
)

add_library(aobuilder ${HEADERS}  ${SOURCES})
target_link_libraries(aobuilder
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
)
