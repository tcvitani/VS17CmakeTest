cmake_minimum_required(VERSION 3.16)
project(cmwork LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)

set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

configure_file("${PROJECT_RESOURCES_DIR}/cmwork_res.rc.in" Resources.rc @ONLY)

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/cmwork.c
	
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} 	 		
)


target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
    CMWORK_EXPORTS # Must be PRIVATE to be defined only during BUILD
)

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

		IF_CM_Workers
)


##----------------------------------  

set(PROJECT_LIB_EX "csr_${PROJECT_NAME}ex_vs17_x64")

add_library(${PROJECT_LIB_EX} SHARED ${PROJECT_SOURCES} 	 		
)


target_include_directories(${PROJECT_LIB_EX} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
)

target_compile_definitions(${PROJECT_LIB_EX}
PRIVATE
    CMWORKEX_EXPORTS # Must be PRIVATE to be defined only during BUILD
)


set_target_properties(${PROJECT_LIB_EX}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)


target_link_libraries(${PROJECT_LIB_EX}
    PUBLIC
		CSR::Excpt
		CSR::Trc
	    CSR::Reg

		IF_CM_Workers
)



##-----------------------------------------------------------------------------------------------
##-----------------------------------------------------------------------------------------------

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")

add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

add_custom_command(
    TARGET ${PROJECT_LIB_EX} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${PROJECT_LIB_EX}>
    COMMAND_EXPAND_LISTS
)

