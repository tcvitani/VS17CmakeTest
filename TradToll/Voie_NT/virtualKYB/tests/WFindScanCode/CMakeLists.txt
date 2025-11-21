cmake_minimum_required(VERSION 3.16)
project(WFindScanCode LANGUAGES CXX)

set(WFindScanCode_EXE "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Network
		Xml
		UiTools
		Core5Compat
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
)
qt_standard_project_setup()


set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
 
configure_file("${PROJECT_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	#${SIMULATOR_UI_GEN_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sources/maindlg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sources/maindlg.h
	${CMAKE_CURRENT_SOURCE_DIR}/sources/maindlg.ui

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


qt_add_executable(${WFindScanCode_EXE} 
					${PROJECT_SOURCES} 
)

target_include_directories(${WFindScanCode_EXE} PRIVATE
							${CMAKE_CURRENT_SOURCE_DIR}/sources
							${CMAKE_CURRENT_BINARY_DIR}
							${PROJECT_RESOURCES_DIR}
)

##-----------------------------------------------------------------------------------------------
															
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${WFindScanCode_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)
 

target_link_libraries(${WFindScanCode_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Network
		Qt::Core5Compat
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${WFindScanCode_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${WFindScanCode_EXE}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)