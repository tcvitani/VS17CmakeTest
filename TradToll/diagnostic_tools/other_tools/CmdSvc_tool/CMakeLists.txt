cmake_minimum_required(VERSION 3.16)
project(CmdSvcTool LANGUAGES CXX)

set(PROJECT_EXE "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
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
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.10.1/lib/cmake"
) 

qt_standard_project_setup()


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

 
configure_file("${PROJECT_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 
qt_wrap_ui(PROJECT_UI_GEN_HEADERS
	${PROJECT_SOURCES_DIR}/MMainDialogB.ui

)

set(PROJECT_HEADERS
 	${PROJECT_HEADERS_DIR}/MGlobalStructs.h
 	${PROJECT_HEADERS_DIR}/MDefines.h
 	${PROJECT_HEADERS_DIR}/MCmdACOM.h
 	${PROJECT_HEADERS_DIR}/MCmdSvcToolConfig.h
)


set(PROJECT_SOURCES
	${PROJECT_UI_GEN_HEADERS}
	${PROJECT_SOURCES_DIR}/main.cpp
	${PROJECT_SOURCES_DIR}/MMainDialog.cpp
	${PROJECT_SOURCES_DIR}/MCmdSvcToolConfig.cpp
	${PROJECT_SOURCES_DIR}/MCmdACOM.cpp
	
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
	${PROJECT_RESOURCES_DIR}/ImgResources.qrc
)

qt_add_executable(${PROJECT_EXE} ${PROJECT_SOURCES} ${PROJECT_HEADERS} )

#qt_add_resources(${PROJECT_EXE} ${PROJECT_RESOURCES_DIR}/ImgResources.qrc)



target_include_directories(${PROJECT_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							${PROJECT_HEADERS_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"
)

##-----------------------------------------------------------------------------------------------
add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							


add_library(CSR::Acom SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Acom PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/acom/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/acom/out/build/release/csr_acom_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/acom/out/build/release/csr_acom_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/acom/out/build/debug/csr_acom_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/acom/out/build/debug/csr_acom_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

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

add_library(CSR::common_tools_classes SHARED IMPORTED GLOBAL)
set_target_properties(CSR::common_tools_classes PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/common_tools_classes/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/common_tools_classes/out/build/release/common_tools_classes_vs17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/common_tools_classes/out/build/release/common_tools_classes_vs17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/common_tools_classes/out/build/debug/common_tools_classes_vs17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../diagnostic_tools/_common/common_tools_classes/out/build/debug/common_tools_classes_vs17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	
 										
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${PROJECT_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)


target_link_libraries(${PROJECT_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Core5Compat
		CSR::Acom
		CSR::Reg
		CSR::common_tools_classes
		CSR::qt_translator
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${PROJECT_EXE}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)