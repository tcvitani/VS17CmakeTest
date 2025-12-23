cmake_minimum_required(VERSION 3.16)
project(msg_lc_auth_VT LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
	${EXPORT_HEADERS_DIR}/Msg_Lc_Auth_VT.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_rep.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_rep_v2.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_rep_v3.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_req.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_req_v2.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_enl_req_v3.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_rep.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_rep_v2.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_rep_v3.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_req.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_req_v2.h
	${EXPORT_HEADERS_DIR}/msg_lc_auth_vt_exl_req_v3.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/Msg_Lc_Auth_VT.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/Msg_Lc_Auth_VT.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_rep.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_rep_v2.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_rep_v3.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_req.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_req_v2.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_enl_req_v3.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_rep.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_rep_v2.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_rep_v3.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_req.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_req_v2.c
	${PROJECT_SOURCES_DIR}/msg_lc_auth_vt_exl_req_v3.c

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
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/includes/"
)
  
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    MSG_LC_AUTH_VT_EXPORTS # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------

add_library(CSR::msg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::msg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/release/csr_msg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/release/csr_msg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/debug/csr_msg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/debug/csr_msg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::msg_lc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::msg_lc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg_lc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg_lc/out/build/release/csr_msg_lc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg_lc/out/build/release/csr_msg_lc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg_lc/out/build/debug/csr_msg_lc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg_lc/out/build/debug/csr_msg_lc_VS17_x64.dll"
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
        CSR::msg
		CSR::msg_lc
)

add_subdirectory(tests)



set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


# #Copy all runtime files (DLLs) to build directory
#add_custom_command(
#    TARGET ${PROJECT_LIB} POST_BUILD
#    COMMAND ${CMAKE_COMMAND}
#        -E copy
#        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
#        $<TARGET_RUNTIME_DLLS:${PROJECT_LIB}>
#    COMMAND_EXPAND_LISTS
#)

add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)


#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)
 
#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)