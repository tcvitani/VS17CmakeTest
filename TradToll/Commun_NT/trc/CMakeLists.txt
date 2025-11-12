cmake_minimum_required(VERSION 3.16)
project(trc LANGUAGES C)


#set(CMAKE_GENERATOR_TOOLSET   version=143)

#message(${CMAKE_GENERATOR_TOOLSET} "- Toolset selected")

set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/trc.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/trc.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/trc.c	
	${PROJECT_SOURCES_DIR}/htrc.c
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	${PROJECT_SOURCES_DIR}/htrc.h
	${PROJECT_SOURCES_DIR}/loc_trc.h
	${PROJECT_SOURCES_DIR}/trcdbg.h
)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}	${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							L:/Commun_NT/declare/10.0.0		
							L:/voie_nt/commun/3.2.0
)
 
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    TRC_DEF # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	 						
							
add_library(CSR::Excpt SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Excpt PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../Excpt/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../Excpt/out/build/release/csr_excpt_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../Excpt/out/build/release/csr_excpt_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../Excpt/out/build/debug/csr_excpt_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../Excpt/out/build/debug/csr_excpt_VS17_x64.dll"
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
        CSR::Excpt
	    CSR::Reg
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

