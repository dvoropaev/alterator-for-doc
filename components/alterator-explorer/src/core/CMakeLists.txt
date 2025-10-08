find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets Core DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core DBus)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set (HEADERS
)

set (SOURCES
)

add_library(alterator-explorer-core ${SOURCES} ${HEADERS})

add_subdirectory(logger)
target_link_libraries(alterator-explorer-core logger)

target_link_libraries(alterator-explorer-core
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
)
