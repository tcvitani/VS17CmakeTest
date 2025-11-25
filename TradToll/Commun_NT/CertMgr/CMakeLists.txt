cmake_minimum_required(VERSION 3.16)
project(CertMgr LANGUAGES CXX)


set(CERTMGR_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/CertMgr.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/CertMgr.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/CertMgr.cpp

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${CERTMGR_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${CERTMGR_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"				
)
 
target_compile_definitions(${CERTMGR_LIB}
PRIVATE
    BITBL_EXPORTS # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------
add_library(ThrdPty::Botan1 SHARED IMPORTED GLOBAL)
set_target_properties(ThrdPty::Botan1 PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/Botan/botan_1.10.17_x64/include/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/Botan/botan_1.10.17_x64/Release/msvc-2017/botan.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/Botan/botan_1.10.17_x64/Release/msvc-2017/botan.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/Botan/botan_1.10.17_x64/Debug/msvc-2017/botan.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/Botan/botan_1.10.17_x64/Debug/msvc-2017/botan.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${CERTMGR_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${CERTMGR_LIB}
    PUBLIC
       ThrdPty::Botan1
)

add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${CERTMGR_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${CERTMGR_LIB}>
    COMMAND_EXPAND_LISTS
)

