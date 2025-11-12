cmake_minimum_required(VERSION 3.16)
project(reg LANGUAGES C)


#set(CMAKE_GENERATOR_TOOLSET   version=143)

#message(${CMAKE_GENERATOR_TOOLSET} "- Toolset selected")

set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/reg.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/reg.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/reg.c
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	#no private headers
)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}	${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							L:/Commun_NT/declare/10.0.0						
)
 
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    REG_DEF # Must be PRIVATE to be defined only during BUILD
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

#target_link_libraries(${PROJECT_LIB}
#    PUBLIC
#       "dbghelp.lib"
#	   "psapi.lib"
#)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

