macro(add_alt_executable name)
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
		COMMENT "Copy ${name} to ${CMAKE_BINARY_DIR}/bin"
	)
endmacro()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(qbase COMPONENTS logger REQUIRED)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

configure_file(appversion.h.in appversion.h)

set(HEADERS
	constants.h
	application.h

	ui/mainwindow/mainwindow.h
	ui/mainwindow/mainwindowsettings.h
	ui/componentswidget/componentswidget.h
	ui/applydialog/transactiontabwidget.h
	ui/applydialog/wizard.h
	ui/applydialog/transactionwizard.h
	ui/applydialog/updatingsourceswizard.h
	ui/applydialog/wizardpage.h
	ui/applydialog/requestwizardpage.h
	ui/applydialog/resolvewizardpage.h
	ui/applydialog/progresswizardpage.h
	ui/aboutdialog/aboutdialog.h
	ui/statusbar/statusbar.h
	ui/statusbar/mainstatusbar.h
	ui/statusbar/applystatusbar.h
	ui/warnings/warningsdialog.h
	ui/errordialog/errordialog.h

	controller/controller.h

	model/objects/object.h
	model/objects/section.h
	model/objects/category.h
	model/objects/component.h
	model/objects/package.h
	model/objects/tag.h
	model/model.h
	model/componentfilterproxymodel.h
	model/proxymodel.h
	model/transactionfilterproxymodel.h
	model/ensembleproxymodel.h
	model/item.h
	model/modelbuilder.h

	repository/componentrepository.h
	repository/packagerepository.h

	service/transaction.h
	service/transactionservice.h

	dbus/dbusproxy.h
)

set(SOURCES
	main.cpp
	application.cpp

	ui/mainwindow/mainwindow.cpp
	ui/mainwindow/mainwindowsettings.cpp
	ui/componentswidget/componentswidget.cpp
	ui/applydialog/transactiontabwidget.cpp
	ui/applydialog/updatingsourceswizard.cpp
	ui/applydialog/wizard.cpp
	ui/applydialog/transactionwizard.cpp
	ui/applydialog/wizardpage.cpp
	ui/applydialog/requestwizardpage.cpp
	ui/applydialog/resolvewizardpage.cpp
	ui/applydialog/progresswizardpage.cpp
	ui/aboutdialog/aboutdialog.cpp
	ui/statusbar/statusbar.cpp
	ui/statusbar/mainstatusbar.cpp
	ui/statusbar/applystatusbar.cpp
	ui/warnings/warningsdialog.cpp
	ui/errordialog/errordialog.cpp

	controller/controller.cpp

	model/objects/object.cpp
	model/objects/section.cpp
	model/objects/edition.cpp
	model/objects/category.cpp
	model/objects/component.cpp
	model/objects/package.cpp
	model/objects/tag.cpp
	model/model.cpp
	model/componentfilterproxymodel.cpp
	model/proxymodel.cpp
	model/transactionfilterproxymodel.cpp
	model/ensembleproxymodel.cpp
	model/item.cpp
	model/modelbuilder.cpp

	repository/componentrepository.cpp
	repository/packagerepository.cpp

	service/transaction.cpp
	service/transactionservice.cpp

	dbus/dbusproxy.cpp
)

set(UI_FORMS
	ui/mainwindow/mainwindow.ui
	ui/componentswidget/componentswidget.ui
	ui/applydialog/applytabwidget.ui
	ui/applydialog/wizard.ui
	ui/applydialog/wizardpage.ui
	ui/aboutdialog/aboutdialog.ui
	ui/statusbar/statusbar.ui
	ui/warnings/warningsdialog.ui
	ui/errordialog/errordialog.ui
)
set(RESOURCES
	resources.qrc
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

add_translation_resource(RESOURCES_SRC "alt-components" ${QM_FILES})
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_wrap_ui(UI_SOURCES ${UI_FORMS})
	qt_add_translation(QM_FILES ${TS_FILES})
	add_translation_resource(RESOURCES_SRC "alt-components" ${QM_FILES})
	qt_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})
else()
	qt5_wrap_ui(UI_SOURCES ${UI_FORMS})
	qt5_add_translation(QM_FILES ${TS_FILES})
	add_translation_resource(RESOURCES_SRC "alt-components" ${QM_FILES})
	qt5_add_resources(RCC_SOURCES ${RESOURCES} ${RESOURCES_SRC})
endif()

add_alt_executable(
	alt-components ${SOURCES} ${HEADERS} ${UI_SOURCES} ${RCC_SOURCES}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(PROJECT_WARNINGS
		-Wall       # Common warnings
		-Wextra     # Additional warnings
		-Wpedantic  # Strict ISO C++ compliance

		-Wfloat-equal             # Float equality comparisons
		-Wlogical-op              # Suspicious logical operators
		-Wduplicated-cond         # Duplicate conditions in if chains
		-Wduplicated-branches     # Identical code in if/else
		-Wnull-dereference        # Null pointer dereferences
		-Wuseless-cast            # Redundant type casts
		-Wswitch-default          # Missing switch default
		-Wimplicit-fallthrough=3  # Switch fallthrough
		-Wconversion              # Implicit value-changing conversions

		-Wnon-virtual-dtor            # Non-virtual destructor with virtuals
		-Wold-style-cast              # C-style casts
		-Woverloaded-virtual          # Hidden virtual overrides
		-Wcast-qual                   # Qualifier-dropping casts
		-Wcast-align                  # Alignment-changing casts
		-Wundef                       # Undefined #if identifiers
		-Wmissing-declarations        # Missing global declarations
		-Wmissing-field-initializers  # Incomplete struct init
		-Wdeprecated-declarations     # Deprecated features
	)
endif()

if(PROJECT_WARNINGS)
	target_compile_options(alt-components PRIVATE ${PROJECT_WARNINGS})
endif()

set_property(TARGET alt-components PROPERTY CXX_STANDARD 20)

target_link_libraries(alt-components
	${CMAKE_THREAD_LIBS_INIT}
	qbase::logger
	Qt${QT_VERSION_MAJOR}::DBus
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
)
