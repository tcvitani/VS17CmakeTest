cmake_minimum_required(VERSION 3.16)
project(VIRT_KYB LANGUAGES CXX)

set(PROJECT_EXE "csr_${PROJECT_NAME}_vs17_x64")

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
		Core5Compat
		UiTools
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
) 

qt_standard_project_setup()


set(SIMULATOR_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(SIMULATOR_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/virt_kyb.h
)

 
configure_file("${SIMULATOR_RESOURCES_DIR}/virtualkb.rc.in" Resources.rc @ONLY)

set(PROJECT_SOURCES
	${SIMULATOR_SOURCES_DIR}/main.cpp
	${SIMULATOR_SOURCES_DIR}/MTracer.cpp
	${SIMULATOR_SOURCES_DIR}/MKybWDThread.cpp
	${SIMULATOR_SOURCES_DIR}/MKybConfigGeneral.cpp
	${SIMULATOR_SOURCES_DIR}/MKybMainLogic.cpp
	${SIMULATOR_SOURCES_DIR}/MKybFormMain.cpp
	${SIMULATOR_SOURCES_DIR}/MKybAniInitData.cpp
	${SIMULATOR_SOURCES_DIR}/MKybAniThread.cpp
	${SIMULATOR_SOURCES_DIR}/MKybSrv.cpp
	${SIMULATOR_SOURCES_DIR}/MKybMsg.cpp
	${SIMULATOR_SOURCES_DIR}/MKybVisibleObject.cpp
	${SIMULATOR_SOURCES_DIR}/MKybTypeSettings.cpp
	${SIMULATOR_SOURCES_DIR}/MTracer.h
	${SIMULATOR_SOURCES_DIR}/MKybWDThread.h
	${SIMULATOR_SOURCES_DIR}/MKybConfigGeneral.h
	${SIMULATOR_SOURCES_DIR}/MKybMainLogic.h
 	${SIMULATOR_SOURCES_DIR}/MKybFormMain.h
	${SIMULATOR_SOURCES_DIR}/MKybAniInitData.h
	${SIMULATOR_SOURCES_DIR}/MKybAniThread.h
	${SIMULATOR_SOURCES_DIR}/MKybSrv.h
	${SIMULATOR_SOURCES_DIR}/MKybMsg.h
	${SIMULATOR_SOURCES_DIR}/MKybVisibleObject.h
	${SIMULATOR_SOURCES_DIR}/MKybTypeSettings.h

)


qt_add_executable(${PROJECT_EXE} 
					${PROJECT_SOURCES}  
					${EXPORT_HEADERS}  
)

target_include_directories(${PROJECT_EXE} PRIVATE
							${SIMULATOR_SOURCES_DIR}
							${SIMULATOR_RESOURCES_DIR}
							#${CMAKE_CURRENT_BINARY_DIR}
							${EXPORT_HEADERS_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/run/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/lwd/startup/includes"				
)

##-----------------------------------------------------------------------------------------------
add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							



	add_library(CSR::Noy SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Noy PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../Noy/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/release/csr_noy_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/release/csr_noy_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/debug/csr_noy_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../Noy/out/build/debug/csr_noy_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	 						
							
add_library(CSR::Srv SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Srv PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.dll"
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
		Qt::Core5Compat
		Qt::UiTools
		CSR::Noy
		CSR::Srv
		CSR::Reg
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


add_subdirectory(tests)
