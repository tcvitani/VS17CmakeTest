cmake_minimum_required(VERSION 3.16)
project(common_tools_classes LANGUAGES CXX)

set(PROJECT_LIB "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

#set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
		Xml
		Gui
		Widgets
		Network
 		Core5Compat
#PATHS
#    $<$<PLATFORM_ID:Windows>:"C:/QT/6.10.1/lib/cmake"
) 

qt_standard_project_setup()

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

#qt_wrap_ui(PROJECT_UI_GEN_HEADERS
#	${EXPORT_HEADERS_DIR}/MAboutWidgetB.ui
#)

set(PROJECT_HEADERS

		${EXPORT_HEADERS_DIR}/MAboutWidget.h
		${EXPORT_HEADERS_DIR}/MConfig.h
		${EXPORT_HEADERS_DIR}/MDefines.h
		${EXPORT_HEADERS_DIR}/MHelpFuncs.h
		${EXPORT_HEADERS_DIR}/MLangPickConfig.h
		${EXPORT_HEADERS_DIR}/MStyledLangPick.h
		${EXPORT_HEADERS_DIR}/MStyledTitleBar.h
		${EXPORT_HEADERS_DIR}/MStyledToolBox.h
		${EXPORT_HEADERS_DIR}/MTracer.h
)

configure_file("${PROJECT_RESOURCES_DIR}/CommonToolsClasses.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/MAboutWidgetB.ui
	
	${PROJECT_SOURCES_DIR}/MAboutWidget.cpp
	${PROJECT_SOURCES_DIR}/MConfig.cpp
	${PROJECT_SOURCES_DIR}/MHelpFuncs.cpp
	${PROJECT_SOURCES_DIR}/MLangPickConfig.cpp
	${PROJECT_SOURCES_DIR}/MStyledLangPick.cpp
	${PROJECT_SOURCES_DIR}/MStyledTitleBar.cpp
	${PROJECT_SOURCES_DIR}/MStyledToolBox.cpp
	${PROJECT_SOURCES_DIR}/MTracer.cpp

	${PROJECT_RESOURCES_DIR}/ImgResources.qrc
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc

)

qt6_add_library(${PROJECT_LIB} SHARED
					${PROJECT_SOURCES} ${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							${AUTOGEN_BUILD_DIR}
)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
    COMMON_TOOLS_CLASSES_EXPORT # Must be PRIVATE to be defined only during BUILD
)

##-----------------------------------------------------------------------------------------------
add_library(CSR::qt_translator SHARED IMPORTED GLOBAL)
set_target_properties(CSR::qt_translator PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/qt_translator/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/qt_translator/out/build/release/qt_translator_vs17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/qt_translator/out/build/release/qt_translator_vs17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/qt_translator/out/build/debug/qt_translator_vs17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/qt_translator/out/build/debug/qt_translator_vs17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	


##-----------------------------------------------------------------------------------------------
							
set_target_properties(${PROJECT_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)


target_link_libraries(${PROJECT_LIB}
    PUBLIC
        Qt::Core
		Qt::Gui
		Qt::Widgets
		Qt::Xml
		Qt::Core5Compat
		CSR::qt_translator
		version
 
)


#add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

