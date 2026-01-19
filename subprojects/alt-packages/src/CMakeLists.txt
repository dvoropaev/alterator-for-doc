macro(add_aap_executable name)
	add_executable(${name} ${ARGN})

	# add install target
	set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name})
	install(TARGETS ${name} DESTINATION bin
				PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
						WORLD_READ WORLD_EXECUTE
		)

	# copy binary
	add_custom_command(TARGET ${name} POST_BUILD COMMAND
				${CMAKE_COMMAND} ARGS -E copy
						$<TARGET_FILE:${name}>
						${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:${name}>
				COMMENT "Copy ${name} to ${CMAKE_BINARY_DIR}/bin")
endmacro()

if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef -fvisibility=hidden\
		-fvisibility-inlines-hidden -pedantic -Wextra -Wno-long-long -Weffc++ -Werror\
		-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy\
		-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op\
		-Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual\
		-Wredundant-decls -Wshadow -Wstrict-null-sentinel -Wundef -Wno-unused\
		-Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -std=c++17")
endif()

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Widgets Core Gui DBus Xml LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui DBus Xml LinguistTools REQUIRED)

configure_file(app/version.h.in app/version.h)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
		constants.h

		app/application.h
		app/commandlineparser.h
		app/version.h

		entities/package.h
		entities/repo.h

		datasource/datasourceinterface.h
		datasource/dbusdatasource.h

		controllers/maincontroller.h
		controllers/basecontroller.h
		controllers/rpmcontroller.h
		controllers/aptcontroller.h
		controllers/repocontroller.h
		controllers/dynamictranslator.h

		model/packagessortfilterproxymodel.h

		ui/mainwindow/mainwindow.h
		ui/mainwindow/packageswidget.h
		ui/mainwindow/mainwindowsettings.h

		ui/managerwidgets/basemanagerwidget.h
		ui/managerwidgets/aptmanagerwidget.h
		ui/managerwidgets/repomanagerwidget.h
		ui/managerwidgets/rpmmanagerwidget.h
		ui/managerwidgets/filterwidget.h

		ui/dialogs/rpmfilesdialog.h
		ui/dialogs/rpminfodialog.h
		ui/dialogs/waitdialog.h
		ui/dialogs/errordialog.h
		ui/dialogs/aboutdialog.h
		ui/dialogs/applydialog.h
)

set(SOURCES
		main.cpp

		app/application.cpp
		app/commandlineparser.cpp

		datasource/dbusdatasource.cpp

		controllers/maincontroller.cpp
		controllers/basecontroller.cpp
		controllers/rpmcontroller.cpp
		controllers/aptcontroller.cpp
		controllers/repocontroller.cpp
		controllers/dynamictranslator.cpp

		model/packagessortfilterproxymodel.cpp

		ui/mainwindow/mainwindow.cpp
		ui/mainwindow/packageswidget.cpp
		ui/mainwindow/mainwindowsettings.cpp

		ui/managerwidgets/basemanagerwidget.cpp
		ui/managerwidgets/aptmanagerwidget.cpp
		ui/managerwidgets/repomanagerwidget.cpp
		ui/managerwidgets/rpmmanagerwidget.cpp
		ui/managerwidgets/filterwidget.cpp

		ui/dialogs/rpmfilesdialog.cpp
		ui/dialogs/rpminfodialog.cpp
		ui/dialogs/waitdialog.cpp
		ui/dialogs/errordialog.cpp
		ui/dialogs/aboutdialog.cpp
		ui/dialogs/applydialog.cpp
)

set(UI_FORMS
		ui/mainwindow/mainwindow.ui
		ui/mainwindow/packageswidget.ui

		ui/managerwidgets/basemanagerwidget.ui
		ui/managerwidgets/filterwidget.ui

		ui/dialogs/rpmfilesdialog.ui
		ui/dialogs/rpminfodialog.ui
		ui/dialogs/waitdialog.ui
		ui/dialogs/errordialog.ui
		ui/dialogs/aboutdialog.ui
		ui/dialogs/applydialog.ui
)

set(RESOURCES
	resources.qrc
)

set(TS_FILES
				${CMAKE_CURRENT_SOURCE_DIR}/ru.ts
				${CMAKE_CURRENT_SOURCE_DIR}/en.ts
)

set(APP_ICONS_DIR "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/alt-packages.svg")

add_custom_target(app_ts
				DEPENDS ${TS_FILES}
				COMMAND ${Qt${QT_VERSION_MAJOR}_LUPDATE_EXECUTABLE} * -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -no-obsolete
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

qt_wrap_ui(UI_SOURCES ${UI_FORMS})
qt_add_translation(QM_FILES ${TS_FILES})
add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})
qt_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})

add_aap_executable(${PROJECT_NAME}
		${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		Qt${QT_VERSION_MAJOR}::Widgets
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Gui
		Qt${QT_VERSION_MAJOR}::DBus
)

install(
    FILES ${APP_ICON}
    DESTINATION ${APP_ICONS_DIR}
)