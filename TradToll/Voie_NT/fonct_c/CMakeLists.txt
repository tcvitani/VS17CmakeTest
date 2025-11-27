cmake_minimum_required(VERSION 3.16)
project(fonct_c LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/trace.h
	${EXPORT_HEADERS_DIR}/str.h
	${EXPORT_HEADERS_DIR}/mem_c.h
	${EXPORT_HEADERS_DIR}/fic.h
	${EXPORT_HEADERS_DIR}/err.h
	${EXPORT_HEADERS_DIR}/buffer.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/fonct_c.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/trace.c
	${PROJECT_SOURCES_DIR}/str.c
	${PROJECT_SOURCES_DIR}/mem_c.c
	${PROJECT_SOURCES_DIR}/fic.c
	${PROJECT_SOURCES_DIR}/err.c
	${PROJECT_SOURCES_DIR}/buffer.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"				
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"			
)
 
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    TRACE_DEF # Must be PRIVATE to be defined only during BUILD
	STR_DEF
	MEM_DEF
	FIC_DEF
	ERR_DEF
	MOD_BUFFER
)



##-----------------------------------------------------------------------------------------------
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
     CSR::Noy
)

add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

