macro(add_alterator_application_legacy_dialog_executable name)
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

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets Core Gui Xml LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui Xml LinguistTools REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
  main.cpp
  dialog.cpp
)

set(HEADERS
  dialog.h
)

set(UI_FORMS
  dialog.ui
)

set(RESOURCES
)

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ru.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/en.ts
)

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

add_alterator_application_legacy_dialog_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Xml
)
