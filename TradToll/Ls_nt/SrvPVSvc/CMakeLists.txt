cmake_minimum_required(VERSION 3.16)
project(SrvPvSvc LANGUAGES C )

##-----------------------------------------------------------------------------------------------
add_library(CSR::Aut SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Aut PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/aut/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/aut/out/build/release/csr_aut_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/aut/out/build/release/csr_aut_VS17_x64_r.lib"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/aut/out/build/debug/csr_aut_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/aut/out/build/debug/csr_aut_VS17_x64.lib"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)			

add_library(CSR::Excpt SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Excpt PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Excpt/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Excpt/out/build/release/csr_excpt_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Excpt/out/build/release/csr_excpt_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Excpt/out/build/debug/csr_excpt_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Excpt/out/build/debug/csr_excpt_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)			

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

add_library(CSR::Col SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Col PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/col/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/col/out/build/release/csr_col_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/col/out/build/release/csr_col_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/col/out/build/debug/csr_col_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/col/out/build/debug/csr_col_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)			


add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							

add_library(CSR::Trc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Trc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	
	
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

add_library(CSR::Acom SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Acom PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/acom/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/acom/out/build/release/csr_acom_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/acom/out/build/release/csr_acom_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/acom/out/build/debug/csr_acom_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/acom/out/build/debug/csr_acom_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)


add_library(CSR::Cmdl SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Cmdl PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/cmdl/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/cmdl/out/build/release/csr_cmdl_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/cmdl/out/build/release/csr_cmdl_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/cmdl/out/build/debug/csr_cmdl_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/cmdl/out/build/debug/csr_cmdl_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

add_library(CSR::Ntsvc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Ntsvc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Ntsvc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Ntsvc/out/build/release/csr_ntsvc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Ntsvc/out/build/release/csr_ntsvc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Ntsvc/out/build/debug/csr_ntsvc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Ntsvc/out/build/debug/csr_ntsvc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

add_library(CSR::MsgSV SHARED IMPORTED GLOBAL)
set_target_properties(CSR::MsgSV PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_SV/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_SV/out/build/release/csr_msg_sv_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_SV/out/build/release/csr_msg_sv_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_SV/out/build/debug/csr_msg_sv_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_SV/out/build/debug/csr_msg_sv_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

add_library(CSR::Dbifora SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Dbifora PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/dbif/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/dbif/out/build/release/csr_dbifora_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/dbif/out/build/release/csr_dbifora_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/dbif/out/build/debug/csr_dbifora_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/dbif/out/build/debug/csr_dbifora_VS17_x64.dll"
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

add_library(CSR::HashCalc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::HashCalc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/HashCalc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/HashCalc/out/build/release/csr_hashcalc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/HashCalc/out/build/release/csr_hashcalc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/HashCalc/out/build/debug/csr_hashcalc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/HashCalc/out/build/debug/csr_hashcalc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::MsgPV SHARED IMPORTED GLOBAL)
set_target_properties(CSR::MsgPV PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_PV/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_PV/out/build/release/csr_msg_pv_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_PV/out/build/release/csr_msg_pv_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_PV/out/build/debug/csr_msg_pv_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_SOURCE_DIR}/../../Ls_nt/Msg_PV/out/build/debug/csr_msg_pv_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

##-----------------------------------------------------------------------------------------------
 
message("-- 'DB Srvpv DBIF' INCLUDED in the build")
add_subdirectory("DB Srvpv DBIF")

message("-- 'DB Connection' INCLUDED in the build")
add_subdirectory("DB Connection")

message("-- 'DB Configuration' INCLUDED in the build")
add_subdirectory("DB Configuration")

#"DF Configuration"
#"DF Srvpv DBIF"

message("-- 'PVHook' INCLUDED in the build")
add_subdirectory("PVHook")



set(PROJECT_EXE "csr_${PROJECT_NAME}_vs17_x64")

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_HEADERS
    ${PROJECT_HEADERS_DIR}/srvpv_main.h
    ${PROJECT_HEADERS_DIR}/route_wm.h
    ${PROJECT_HEADERS_DIR}/route_msg_information.h
    ${PROJECT_HEADERS_DIR}/route_msg.h
    ${PROJECT_HEADERS_DIR}/route_aut.h
    ${PROJECT_HEADERS_DIR}/route_acom.h
    ${PROJECT_HEADERS_DIR}/pv_wm.h
    ${PROJECT_HEADERS_DIR}/pv_conf.h
    ${PROJECT_HEADERS_DIR}/pv_aut.h
    ${PROJECT_HEADERS_DIR}/pv_acom.h
    ${PROJECT_HEADERS_DIR}/plaza.h
    ${PROJECT_HEADERS_DIR}/lane.h
    ${PROJECT_HEADERS_DIR}/Icon.h
    ${PROJECT_HEADERS_DIR}/DB_wm.h
    ${PROJECT_HEADERS_DIR}/cmd_wm.h
    ${PROJECT_HEADERS_DIR}/cmd_acom.h
    ${PROJECT_HEADERS_DIR}/cmd.h
    ${PROJECT_HEADERS_DIR}/alarm_aut.h
    ${PROJECT_HEADERS_DIR}/alarm.h

)

 configure_file("${PROJECT_RESOURCES_DIR}/srvpvsvc.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/srvpv_main.c
	${PROJECT_SOURCES_DIR}/route_wm.c
	${PROJECT_SOURCES_DIR}/route_msg_vault.c
	${PROJECT_SOURCES_DIR}/route_msg_trs.c
	${PROJECT_SOURCES_DIR}/route_msg_trace.c
	${PROJECT_SOURCES_DIR}/route_msg_sv.c
	${PROJECT_SOURCES_DIR}/route_msg_status.c
	${PROJECT_SOURCES_DIR}/route_msg_partly.c
	${PROJECT_SOURCES_DIR}/route_msg_information.c
	${PROJECT_SOURCES_DIR}/route_msg_event.c
	${PROJECT_SOURCES_DIR}/route_msg_disk.c
	${PROJECT_SOURCES_DIR}/route_msg_counters.c
	${PROJECT_SOURCES_DIR}/route_msg_bowl.c
	${PROJECT_SOURCES_DIR}/route_aut.c
	${PROJECT_SOURCES_DIR}/route_acom.c
	${PROJECT_SOURCES_DIR}/pv_wm.c
	${PROJECT_SOURCES_DIR}/pv_conf.c
	${PROJECT_SOURCES_DIR}/pv_aut.c
	${PROJECT_SOURCES_DIR}/pv_acom.c
	${PROJECT_SOURCES_DIR}/plaza.c
	${PROJECT_SOURCES_DIR}/lane.c
	${PROJECT_SOURCES_DIR}/Icon.c
	${PROJECT_SOURCES_DIR}/DB_wm.c
	${PROJECT_SOURCES_DIR}/cmd_wm.c
	${PROJECT_SOURCES_DIR}/cmd_acom.c
	${PROJECT_SOURCES_DIR}/cmd.c
	${PROJECT_SOURCES_DIR}/alarm_aut.c
	${PROJECT_SOURCES_DIR}/alarm.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

add_executable(${PROJECT_EXE} 
					${PROJECT_SOURCES} #${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
							"${CMAKE_SOURCE_DIR}/DB Configuration/includes"
							"${CMAKE_SOURCE_DIR}/DB Connection/includes"
							"${CMAKE_SOURCE_DIR}/DB Srvpv DBIF/includes"
							"${CMAKE_SOURCE_DIR}/PVHook/includes"
							
)
 

							
set_target_properties(${PROJECT_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)



target_link_libraries(${PROJECT_EXE}
    PUBLIC
		CSR::Aut
		CSR::Excpt
		CSR::Trc
	    CSR::Reg
		CSR::Acom
		CSR::Cmdl
		CSR::Ntsvc
		CSR::MsgSV
		CSR::MsgPV
		CSR::Col
		CSR::Dbifora
		CSR::msg
		CSR::List
		CSR::msg_lc
		CSR::HashCalc
		CSR::SrvPVDBConf
		CSR::SrvPVDBConn
		CSR::SrvPVDBIFORA
		CSR::SrvPVHook
)
 
#add_subdirectory(test)



set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_EXE}>
    COMMAND_EXPAND_LISTS
)


