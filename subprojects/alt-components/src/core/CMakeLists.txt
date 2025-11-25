set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(MODULE_NAME "${PROJECT_NAME}-core")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus REQUIRED)

set(CMAKE_AUTOMOC ON)

set(SOURCES
	entity/object.cpp
	entity/section.cpp
	entity/edition.cpp
	entity/category.cpp
	entity/component.cpp
	entity/tag.cpp
	repository/editionrepository.cpp
	repository/componentrepository.cpp
	repository/packagerepository.cpp
	service/transaction.cpp
	service/transactionservice.cpp
	dbus/dbusmanager.cpp
	utility/scopeguard.cpp
)

set(PUBLIC_HEADERS
	entity/object.h
	entity/section.h
	entity/edition.h
	entity/category.h
	entity/component.h
	entity/tag.h
	repository/editionrepository.h
	repository/componentrepository.h
	repository/packagerepository.h
	service/transaction.h
	service/transactionservice.h
	dbus/dbusmanager.h
	utility/scopeguard.h
	interface/ilogger.h
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

add_library(${MODULE_NAME} STATIC ${SOURCES})

target_include_directories(${MODULE_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

if(PROJECT_WARNINGS)
	target_compile_options(${MODULE_NAME} PRIVATE ${PROJECT_WARNINGS})
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY CXX_STANDARD 20)

set_target_properties(${MODULE_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_link_libraries(${MODULE_NAME}
	Qt${QT_VERSION_MAJOR}::DBus
	Qt${QT_VERSION_MAJOR}::Core
)
