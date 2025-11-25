cmake_minimum_required(VERSION 3.16)
project(msg LANGUAGES C)


set(MSG_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/csr_msg.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/msg.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/msg_write.c
	${PROJECT_SOURCES_DIR}/msg_record.c
	${PROJECT_SOURCES_DIR}/msg_read.c
	${PROJECT_SOURCES_DIR}/msg_new.c
	${PROJECT_SOURCES_DIR}/msg_access.c
	${PROJECT_SOURCES_DIR}/msg_loc.h

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${MSG_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${MSG_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"				
)
 
target_compile_definitions(${MSG_LIB}
PRIVATE
    MSG_EXPORTS # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::List STATIC IMPORTED GLOBAL)
set_target_properties(CSR::List PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)			
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${MSG_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${MSG_LIB}
    PUBLIC
	CSR::List
		
)

#add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${MSG_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${MSG_LIB}>
    COMMAND_EXPAND_LISTS
)

