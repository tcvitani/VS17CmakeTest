cmake_minimum_required(VERSION 3.16)
project(WTestVirtualKYB LANGUAGES CXX)

set(TEST_EXE "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

set(CMAKE_AUTOUIC ON)
set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Xml
		Core5Compat
		UiTools
		Qml
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.10.1/lib/cmake"
) 

qt_standard_project_setup()


set(TEST_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

 
configure_file("${TEST_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 
set(TEST_SOURCES
	${TEST_SOURCES_DIR}/main.cpp
	${TEST_SOURCES_DIR}/maindlg.cpp
	${TEST_SOURCES_DIR}/maindlg.h
	${TEST_SOURCES_DIR}/maindlg.ui

	${TEST_SOURCES_DIR}/SimuSendReciveThread.cpp
	${TEST_SOURCES_DIR}/SimuSendReciveThread.h

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc

)


qt_add_executable(${TEST_EXE} 
					${TEST_SOURCES} 
)

target_include_directories(${TEST_EXE} PRIVATE
							${TEST_SOURCES_DIR}
							${TEST_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							#${EXPORT_HEADERS_DIR}
							"${CMAKE_SOURCE_DIR}/includes" #to be able to use the headers from virtualKyb
							"${CMAKE_SOURCE_DIR}/sources"  #to be able to use the headers from virtualKyb
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/run/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/lwd/startup/includes"				
)

##-----------------------------------------------------------------------------------------------
#add_library(CSR::Noy SHARED IMPORTED GLOBAL)
#set_target_properties(CSR::Noy PROPERTIES
#					  INTERFACE_INCLUDE_DIRECTORIES
#					  	"${CMAKE_SOURCE_DIR}/../Noy/includes/"
#					  IMPORTED_IMPLIB_RELEASE
#					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/release/csr_noy_VS17_x64_r.lib"
#					  IMPORTED_LOCATION_RELEASE	
#					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/release/csr_noy_VS17_x64_r.dll"
#					  IMPORTED_IMPLIB_DEBUG		
#					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/debug/csr_noy_VS17_x64.lib"
#					  IMPORTED_LOCATION_DEBUG	
#					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/debug/csr_noy_VS17_x64.dll"
#					  MAP_IMPORTED_CONFIG_MINSIZEREL
#					  	Release
#					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
#					  	Release
#)							
#	 						
#							
#add_library(CSR::Srv SHARED IMPORTED GLOBAL)
#set_target_properties(CSR::Srv PROPERTIES
#					  INTERFACE_INCLUDE_DIRECTORIES
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/includes/"
#					  IMPORTED_IMPLIB_RELEASE
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.lib"
#					  IMPORTED_LOCATION_RELEASE
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.dll"
#					  IMPORTED_IMPLIB_DEBUG
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.lib"
#					  IMPORTED_LOCATION_DEBUG
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.dll"
#					  MAP_IMPORTED_CONFIG_MINSIZEREL
#					  	Release
#					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
#					  	Release
#)																		
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${TEST_EXE}
    PROPERTIES
        WIN32_EXECUTABLE FALSE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)


target_link_libraries(${TEST_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Core5Compat
		Qt::UiTools
		Qt::Qml
		CSR::Reg
		CSR::Srv
		CSR::Noy

)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${TEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${TEST_EXE}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)