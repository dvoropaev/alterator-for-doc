cmake_minimum_required(VERSION 3.16)

project(alt-services VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5      REQUIRED COMPONENTS Core Widgets DBus LinguistTools Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets DBus LinguistTools Xml)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(KF6                   REQUIRED COMPONENTS WidgetsAddons ItemModels)

find_package(range-v3 REQUIRED)

set(TS_FILES
    ts/en_US.ts
    ts/ru_RU.ts
)

include_directories("src" ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SOURCES
    src/main.cpp

# APPLICATION
    src/app/ServicesApp.h
    src/app/ServicesApp.cpp

    src/app/AppSettings.h
    src/app/AppSettings.cpp

    src/app/RichToolTipEventFilter.h
    src/app/RichToolTipEventFilter.cpp

    src/controller/Controller.h
    src/controller/Controller.cpp
    src/controller/ControllerPrivate.h
    src/controller/ControllerPrivate.cpp

    src/controller/DBusProxy.h
    src/controller/DBusProxy.cpp

# DATA CLASSES
    src/data/TranslatableObject.h
    src/data/TranslatableObject.cpp

    src/data/Property.h
    src/data/Property.cpp

    src/data/Parameter.h
    src/data/Parameter.cpp

    src/data/Resource.h
    src/data/Resource.cpp

    src/data/Service.h
    src/data/Service.cpp

    src/data/Action.h
    src/data/Action.cpp

# DATA MODELS
    src/data/models/ParameterModel.h
    src/data/models/ParameterModel.cpp

    src/data/models/ServiceModel.h
    src/data/models/ServiceModel.cpp

    src/data/models/ResourceModel.h
    src/data/models/ResourceModel.cpp

    src/data/models/ResourceOwnersModel.h
    src/data/models/ResourceOwnersModel.cpp

    src/data/models/DiagModel.h
    src/data/models/DiagModel.cpp

    src/data/models/DiagModeFilterModel.h
    src/data/models/DiagModeFilterModel.cpp

    src/data/models/DiagSelectionModel.h
    src/data/models/DiagSelectionModel.cpp

# MAIN WIDGETS
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.ui

    src/ui/AboutDialog.h
    src/ui/AboutDialog.cpp
    src/ui/AboutDialog.ui

    src/ui/ServiceWidget.h
    src/ui/ServiceWidget.cpp
    src/ui/ServiceWidget.ui

# WIZARD
    src/ui/wizard/ActionWizard.h
    src/ui/wizard/ActionWizard.cpp

# WIZARD PAGES
    src/ui/wizard/Page.h

    src/ui/wizard/01-initial/InitialPage.h
    src/ui/wizard/01-initial/InitialPage.cpp
    src/ui/wizard/01-initial/InitialPage.ui

    src/ui/wizard/02-diag-selection/DiagSelectionForm.h
    src/ui/wizard/02-diag-selection/DiagSelectionForm.cpp
    src/ui/wizard/02-diag-selection/DiagSelectionForm.ui

    src/ui/wizard/02-diag-selection/DiagSelectionPage.h
    src/ui/wizard/02-diag-selection/DiagSelectionPage.cpp
    src/ui/wizard/02-diag-selection/DiagSelectionPage.ui

    src/ui/wizard/03-parameters/ParametersPage.h
    src/ui/wizard/03-parameters/ParametersPage.cpp
    src/ui/wizard/03-parameters/ParametersPage.ui

    src/ui/wizard/04-confirmation/ConfirmationPage.h
    src/ui/wizard/04-confirmation/ConfirmationPage.cpp
    src/ui/wizard/04-confirmation/ConfirmationPage.ui

    src/ui/wizard/05-progress/LogWidget.h
    src/ui/wizard/05-progress/LogWidget.cpp

    src/ui/wizard/05-progress/ProgressPage.h
    src/ui/wizard/05-progress/ProgressPage.cpp
    src/ui/wizard/05-progress/ProgressPage.ui


# MISC WIDGETS
    src/ui/misc/ObjectInfoWidget.h
    src/ui/misc/ObjectInfoWidget.cpp
    src/ui/misc/ObjectInfoWidget.ui

    src/ui/misc/CustomTreeView.h
    src/ui/misc/CustomTreeView.cpp

    src/ui/misc/ObjectInfoDelegate.h
    src/ui/misc/ObjectInfoDelegate.cpp

    src/ui/misc/SearchAdapter.h
    src/ui/misc/ItemModelSearcher.h
    src/ui/misc/ItemModelSearcher.cpp
    src/ui/misc/ParameterSearcher.h

    src/ui/misc/SearchBar.h
    src/ui/misc/SearchBar.cpp
    src/ui/misc/SearchBar.ui

# FORM VARIANTS
    src/ui/wizard/03-parameters/forms/BaseForm.h

    src/ui/wizard/03-parameters/forms/CompactForm.h
    src/ui/wizard/03-parameters/forms/CompactForm.cpp

    src/ui/wizard/03-parameters/forms/DetailedForm.h
    src/ui/wizard/03-parameters/forms/DetailedForm.cpp
    src/ui/wizard/03-parameters/forms/DetailedForm.ui

# FORM EDITORS
    src/ui/wizard/03-parameters/forms/editors/Editor.h
    src/ui/wizard/03-parameters/forms/editors/Editor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/CompactArrayFace.h
    src/ui/wizard/03-parameters/forms/editors/compact/CompactArrayFace.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/PrimitiveEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/PrimitiveEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/IntEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/IntEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/StringEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/StringEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/PasswordEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/PasswordEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/EnumEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/EnumEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/compact/BoolEditor.h
    src/ui/wizard/03-parameters/forms/editors/compact/BoolEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/detailed/CompositeEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/CompositeEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/detailed/GroupEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/GroupEditor.cpp

    src/ui/wizard/03-parameters/forms/editors/detailed/CompositeArrayEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/CompositeArrayEditor.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/CompositeArrayEditor.ui

    src/ui/wizard/03-parameters/forms/editors/detailed/ArrayEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/ArrayEditor.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/ArrayEditor.ui

    src/ui/wizard/03-parameters/forms/editors/detailed/misc/EditorTitle.h
    src/ui/wizard/03-parameters/forms/editors/detailed/misc/EditorTitle.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/misc/EditorTitle.ui

    src/ui/wizard/03-parameters/forms/editors/detailed/misc/CollapseButton.h
    src/ui/wizard/03-parameters/forms/editors/detailed/misc/CollapseButton.cpp

    src/ui/wizard/03-parameters/forms/editors/detailed/misc/ValidationIndicator.h
    src/ui/wizard/03-parameters/forms/editors/detailed/misc/ValidationIndicator.cpp

    src/ui/wizard/03-parameters/forms/editors/detailed/DetailedEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/EditorWrapper.h

    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/InlineEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/InlineEditor.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/InlineEditor.ui

    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CollapsibleEditor.h
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CollapsibleEditor.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CollapsibleEditor.ui

    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CompositeArrayItem.h
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CompositeArrayItem.cpp
    src/ui/wizard/03-parameters/forms/editors/detailed/wrappers/CompositeArrayItem.ui
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
    add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_translation_resource(RESOURCES_SRC ${PROJECT_NAME} ${QM_FILES})
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${RESOURCES_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
    KF6::WidgetsAddons
    KF6::ItemModels
    Qt6Solutions_SingleApplication-2.6
)


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
