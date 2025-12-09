cmake_minimum_required(VERSION 3.16)
project(tlm_dmv LANGUAGES CXX C)

set(PROJECT_LIB "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

#set(CMAKE_AUTOUIC ON)
set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Network
		Xml
		Core5Compat
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
) 


qt_standard_project_setup()

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_HEADERS
		${PROJECT_HEADERS_DIR}/tlm_dmv.h
)



configure_file("${PROJECT_RESOURCES_DIR}/Resource.rc.in" Resources.rc @ONLY)
 
set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/UdpClient.h
	${PROJECT_SOURCES_DIR}/TlmDmvWorker.h
	${PROJECT_SOURCES_DIR}/TlmDmvUtil.h
	${PROJECT_SOURCES_DIR}/TlmDmvServ.h
	${PROJECT_SOURCES_DIR}/TlmDmvGlobal.h
	${PROJECT_SOURCES_DIR}/TlmDmvAutomate.h
	${PROJECT_SOURCES_DIR}/TlmDmvAni.h
	${PROJECT_SOURCES_DIR}/MQEventLoop.h
	${PROJECT_SOURCES_DIR}/MQApplication.h
	${PROJECT_SOURCES_DIR}/mbx2slot.h
	${PROJECT_SOURCES_DIR}/InstTracer.h
	${PROJECT_SOURCES_DIR}/HelpFuncs.h
	${PROJECT_SOURCES_DIR}/AutomatFunctor.h
	${PROJECT_SOURCES_DIR}/Automat.h
	${PROJECT_SOURCES_DIR}/UdpClient.cpp
	${PROJECT_SOURCES_DIR}/TlmDmvWorker.cpp
	${PROJECT_SOURCES_DIR}/TlmDmvAutomate.cpp
	${PROJECT_SOURCES_DIR}/MQEventLoop.cpp
	${PROJECT_SOURCES_DIR}/MQApplication.cpp
	${PROJECT_SOURCES_DIR}/InstTracer.cpp
	${PROJECT_SOURCES_DIR}/HelpFuncs.cpp
	${PROJECT_SOURCES_DIR}/Automat.cpp
	${PROJECT_SOURCES_DIR}/TlmDmvUtil.c
	${PROJECT_SOURCES_DIR}/TlmDmvServ.c
	${PROJECT_SOURCES_DIR}/TlmDmvInit.c
	${PROJECT_SOURCES_DIR}/TlmDmvAni.c
	${PROJECT_SOURCES_DIR}/mbx2slot.c
)


add_library(${PROJECT_LIB} SHARED
					${PROJECT_SOURCES} ${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/run/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/aff/aff_interface"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/tlm/tlm_interface"

)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
    CSR_TLM_EXPORTS # Must be PRIVATE to be defined only during BUILD
)

##-----------------------------------------------------------------------------------------------
add_library(CSR::Noy SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Noy PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	
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

add_library(CSR::Trc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Trc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)		


add_library(CSR::Srv SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Srv PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/srv/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::dmv_oblak_protocol SHARED IMPORTED GLOBAL)
set_target_properties(CSR::dmv_oblak_protocol PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/release/dmv_oblak_protocol_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/release/dmv_oblak_protocol_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/debug/dmv_oblak_protocol_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/debug/dmv_oblak_protocol_VS17_x64.dll"
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
        Qt::Network
		Qt::Xml
		Qt::Core5Compat
		CSR::Noy
		CSR::Reg
		CSR::Trc
		CSR::Srv
		CSR::dmv_oblak_protocol
)

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

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)