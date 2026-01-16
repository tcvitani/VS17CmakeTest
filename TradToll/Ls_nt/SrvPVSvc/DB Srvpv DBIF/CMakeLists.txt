cmake_minimum_required(VERSION 3.16)
project(SrvPV_DBIFORA LANGUAGES C)


set(SRVPV_DBIFORA_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(EXPORT_HEADERS_DIR   	${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
		${EXPORT_HEADERS_DIR}/srvpv_dbif.h

)


 configure_file("${PROJECT_RESOURCES_DIR}/resources.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/srvpv_dbif.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${SRVPV_DBIFORA_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS}		 		
)

target_include_directories(${SRVPV_DBIFORA_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							#${PROJECT_HEADERS_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"		
)
 
target_compile_definitions(${SRVPV_DBIFORA_LIB}
PRIVATE
    SRVPV_DBIF_EXPORTS  # Must be PRIVATE to be defined only during BUILD
	
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::SrvPVDBIFORA SHARED IMPORTED GLOBAL)
set_target_properties(CSR::SrvPVDBIFORA PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${EXPORT_HEADERS_DIR}"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_DBIFORA_LIB}_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_DBIFORA_LIB}_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_DBIFORA_LIB}.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_DBIFORA_LIB}.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${SRVPV_DBIFORA_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${SRVPV_DBIFORA_LIB}
    PUBLIC
		CSR::Excpt
		CSR::Trc
	    CSR::Reg
		CSR::Cmdl
		CSR::Ntsvc
		CSR::MsgPV
		CSR::msg
		CSR::List
		CSR::Dbifora
)


set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${SRVPV_DBIFORA_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${SRVPV_DBIFORA_LIB}>
    COMMAND_EXPAND_LISTS
)

