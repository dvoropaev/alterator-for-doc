find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
    loggermanager.h
    loggermessage.h
    logger.h
    consolelogger.h
    filelogger.h
    sysloglogger.h
)

set(SOURCES
    loggermanager.cpp
    logger.cpp
    consolelogger.cpp
    filelogger.cpp
    sysloglogger.cpp
)

add_definitions(
    -DAB_CORE_LIBRARY
)

add_library(logger STATIC ${SOURCES} ${HEADERS})
target_include_directories(logger PUBLIC ${Qt${QT_VERSION_MAJOR}Gui_INCLUDE_DIRS})
target_link_libraries(logger Qt${QT_VERSION_MAJOR}::Core)
