cmake_minimum_required(VERSION 3.16)
project(SrvPV_PVHOOK LANGUAGES C)


set(SRVPV_PVHOOK_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(EXPORT_HEADERS_DIR   	${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
		${EXPORT_HEADERS_DIR}/pvhook.h

)

 configure_file("${PROJECT_RESOURCES_DIR}/resources.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/pvhook.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${SRVPV_PVHOOK_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${SRVPV_PVHOOK_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							#${PROJECT_HEADERS_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"		
)
 
#target_compile_definitions(${SRVPV_PVHOOK_LIB}
#PRIVATE
#    CONF_SRVPV_EXPORTS # Must be PRIVATE to be defined only during BUILD
#	
#)



##-----------------------------------------------------------------------------------------------

add_library(CSR::SrvPVHook SHARED IMPORTED GLOBAL)
set_target_properties(CSR::SrvPVHook PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${EXPORT_HEADERS_DIR}"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_PVHOOK_LIB}_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_PVHOOK_LIB}_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_PVHOOK_LIB}.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_PVHOOK_LIB}.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)


##-----------------------------------------------------------------------------------------------
							
set_target_properties(${SRVPV_PVHOOK_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${SRVPV_PVHOOK_LIB}
    PUBLIC
		CSR::Excpt
		CSR::Trc
	    CSR::Reg
		CSR::Cmdl
		CSR::Ntsvc
)


set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${SRVPV_PVHOOK_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${SRVPV_PVHOOK_LIB}>
    COMMAND_EXPAND_LISTS
)

