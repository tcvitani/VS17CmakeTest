cmake_minimum_required(VERSION 3.16)
project(queue LANGUAGES C)


set(QUEUE_MT_LIB "csr_${PROJECT_NAME}mt_vs17_x64")

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/csr_queue.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/queue.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/queue.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

#set(PROJECT_HEADERS
#	 ${PROJECT_SOURCES_DIR}/BITBL_loc.h
#)

add_library(${QUEUE_MT_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${QUEUE_MT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"				
)
 
target_compile_definitions(${QUEUE_MT_LIB}
PRIVATE
    QUEUE_EXPORTS # Must be PRIVATE to be defined only during BUILD
	QUEUE_PROTECTION_ACTIVATED
)



##-----------------------------------------------------------------------------------------------

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${QUEUE_MT_LIB}
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
add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${QUEUE_MT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${QUEUE_MT_LIB}>
    COMMAND_EXPAND_LISTS
)

