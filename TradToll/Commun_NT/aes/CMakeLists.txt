cmake_minimum_required(VERSION 3.16)
project(aes LANGUAGES C)


set(AES_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/csr_aes.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/csr_aes.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/Aes.c
	${PROJECT_SOURCES_DIR}/csr_aes_main.c
	${PROJECT_SOURCES_DIR}/csr_aes_md5.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	 ${EXPORT_HEADERS_DIR}/CpuArch.h
	 ${EXPORT_HEADERS_DIR}/csr_aes.h
	 ${EXPORT_HEADERS_DIR}/Types.h

)

add_library(${AES_LIB} SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}	${EXPORT_HEADERS} 			 		
)

target_include_directories(${AES_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"				
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"	
)
 
target_compile_definitions(${AES_LIB}
PRIVATE
    CSR_AES_DEF # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${AES_LIB}
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
    TARGET ${AES_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${AES_LIB}>
    COMMAND_EXPAND_LISTS
)

