cmake_minimum_required(VERSION 3.16)
project(cmras LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_HEADERS
	${PROJECT_HEADERS_DIR}/cmras.h
)


configure_file("${PROJECT_RESOURCES_DIR}/cmras_res.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/cmras.c
	
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
)
  

##-----------------------------------------------------------------------------------------------
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
		CSR::Trc
	    CSR::Reg
		IF_CM_Hosts
		rasapi32
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


