cmake_minimum_required(VERSION 3.16)
project(dbifora LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")

set(PROJECT_LIB11 "csr_${PROJECT_NAME}_OCI11_vs17_x64")
set(PROJECT_LIB19 "csr_${PROJECT_NAME}_OCI19_vs17_x64")


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
	${EXPORT_HEADERS_DIR}/dbif.h
)

 configure_file("${PROJECT_RESOURCES_DIR}/resource.rc.in" Resources.rc @ONLY)
 
set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/dbifora.c

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
							"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/include"
)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
	DBIF_EXPORTS
    DBIF_MAP # Must be PRIVATE to be defined only during BUILD
)

set_target_properties(${PROJECT_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::OCI11 SHARED IMPORTED GLOBAL)
set_target_properties(CSR::OCI11 PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/include/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/11.2.0.1.0_64bit/lib/MSVC/oci.lib"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::OCI19 SHARED IMPORTED GLOBAL)
set_target_properties(CSR::OCI19 PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/19.3.0.0.0_64bit/include/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/19.3.0.0.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/19.3.0.0.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/19.3.0.0.0_64bit/lib/MSVC/oci.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/OCI/19.3.0.0.0_64bit/lib/MSVC/oci.lib"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	
##-----------------------------------------------------------------------------------------------


target_link_libraries(${PROJECT_LIB}
    PUBLIC
        CSR::OCI11
)

##-----------------------------------------------------------------------------------------------

add_library(${PROJECT_LIB11} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB11} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
	DBIF_EXPORTS    # Must be PRIVATE to be defined only during BUILD
)

target_link_libraries(${PROJECT_LIB11}
    PUBLIC
        CSR::OCI11
)


set_target_properties(${PROJECT_LIB11}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)
##-----------------------------------------------------------------------------------------------
set(PROJECT_SOURCES19
	${PROJECT_SOURCES_DIR}/dbifora19.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

add_library(${PROJECT_LIB19} SHARED ${PROJECT_SOURCES19} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB19} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)


target_compile_definitions(${PROJECT_LIB19}
PRIVATE
	DBIF_EXPORTS    # Must be PRIVATE to be defined only during BUILD
)

set_target_properties(${PROJECT_LIB19}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${PROJECT_LIB19}
    PUBLIC
        CSR::OCI19
)

##-----------------------------------------------------------------------------------------------

	
##-----------------------------------------------------------------------------------------------
							

#add_subdirectory(test)



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
    TARGET ${PROJECT_LIB11} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX}
        $<TARGET_FILE:${PROJECT_LIB11}>
    COMMAND_EXPAND_LISTS
)

add_custom_command(
    TARGET ${PROJECT_LIB19} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX}
        $<TARGET_FILE:${PROJECT_LIB19}>
    COMMAND_EXPAND_LISTS
)
