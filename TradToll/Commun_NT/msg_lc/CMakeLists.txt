cmake_minimum_required(VERSION 3.16)
project(msg_lc LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
	${EXPORT_HEADERS_DIR}/msg_lc_vlt_withdraw.h
	${EXPORT_HEADERS_DIR}/msg_lc_vault_stat.h
	${EXPORT_HEADERS_DIR}/msg_lc_vault_ins.h
	${EXPORT_HEADERS_DIR}/msg_lc_transaction_ves_altair.h
	${EXPORT_HEADERS_DIR}/msg_lc_transaction.h
	${EXPORT_HEADERS_DIR}/msg_lc_trace.h
	${EXPORT_HEADERS_DIR}/msg_lc_tab_ack.h
	${EXPORT_HEADERS_DIR}/msg_lc_status.h
	${EXPORT_HEADERS_DIR}/msg_lc_startshift.h
	${EXPORT_HEADERS_DIR}/msg_lc_reference.h
	${EXPORT_HEADERS_DIR}/msg_lc_perm_send_file.h
	${EXPORT_HEADERS_DIR}/msg_lc_perm_rq_send_file.h
	${EXPORT_HEADERS_DIR}/msg_lc_payment.h
	${EXPORT_HEADERS_DIR}/msg_lc_partly.h
	${EXPORT_HEADERS_DIR}/msg_lc_hourly.h
	${EXPORT_HEADERS_DIR}/msg_lc_header.h
	${EXPORT_HEADERS_DIR}/msg_lc_event.h
	${EXPORT_HEADERS_DIR}/msg_lc_endshift.h
	${EXPORT_HEADERS_DIR}/msg_lc_counter_status.h
	${EXPORT_HEADERS_DIR}/msg_lc_comp_inf_tr.h
	${EXPORT_HEADERS_DIR}/msg_lc_command.h
	${EXPORT_HEADERS_DIR}/msg_lc_coin_listing.h
	${EXPORT_HEADERS_DIR}/msg_lc_bowl_stat.h
	${EXPORT_HEADERS_DIR}/msg_lc_bowl_movement.h
	${EXPORT_HEADERS_DIR}/msg_lc_backup_file.h
	${EXPORT_HEADERS_DIR}/msg_lc_backup_date.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/msg_lc.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/msg_vlt_withdraw.c
	${PROJECT_SOURCES_DIR}/msg_vault_stat.c
	${PROJECT_SOURCES_DIR}/msg_vault_ins.c
	${PROJECT_SOURCES_DIR}/msg_transaction_ves_altair.c
	${PROJECT_SOURCES_DIR}/msg_transaction.c
	${PROJECT_SOURCES_DIR}/msg_trace.c
	${PROJECT_SOURCES_DIR}/msg_tab_ack.c
	${PROJECT_SOURCES_DIR}/msg_status.c
	${PROJECT_SOURCES_DIR}/msg_startshift.c
	${PROJECT_SOURCES_DIR}/msg_reference.c
	${PROJECT_SOURCES_DIR}/msg_perm_send_file.c
	${PROJECT_SOURCES_DIR}/msg_perm_rq_send_file.c
	${PROJECT_SOURCES_DIR}/msg_payment.c
	${PROJECT_SOURCES_DIR}/msg_partly.c
	${PROJECT_SOURCES_DIR}/msg_lc_counter_status.c
	${PROJECT_SOURCES_DIR}/msg_lc.h
	${PROJECT_SOURCES_DIR}/msg_lc.c
	${PROJECT_SOURCES_DIR}/msg_hourly.c
	${PROJECT_SOURCES_DIR}/msg_header.c
	${PROJECT_SOURCES_DIR}/msg_event.c
	${PROJECT_SOURCES_DIR}/msg_endshift.c
	${PROJECT_SOURCES_DIR}/msg_comp_inf_tr.c
	${PROJECT_SOURCES_DIR}/msg_command.c
	${PROJECT_SOURCES_DIR}/msg_coin_listing.c
	${PROJECT_SOURCES_DIR}/msg_bowl_stat.c
	${PROJECT_SOURCES_DIR}/msg_bowl_movement.c
	${PROJECT_SOURCES_DIR}/msg_backup_file.c
	${PROJECT_SOURCES_DIR}/msg_backup_date.c

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
    MSG_LC_EXPORTS # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------

add_library(CSR::msg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::msg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/x64-release/csr_msg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/x64-release/csr_msg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/x64-debug/csr_msg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/msg/out/build/x64-debug/csr_msg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

#add_library(CSR::List STATIC IMPORTED GLOBAL)
#set_target_properties(CSR::List PROPERTIES
#					  INTERFACE_INCLUDE_DIRECTORIES
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/includes/"
#					  IMPORTED_IMPLIB_RELEASE
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
#					  IMPORTED_LOCATION_RELEASE
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
#					  IMPORTED_IMPLIB_DEBUG
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
#					  IMPORTED_LOCATION_DEBUG
#					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
#					  MAP_IMPORTED_CONFIG_MINSIZEREL
#					  	Release
#					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
#					  	Release
#)			
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