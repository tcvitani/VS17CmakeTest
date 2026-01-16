cmake_minimum_required(VERSION 3.16)
project(SrvPV_Conf LANGUAGES C)


set(SRVPV_CONF_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(EXPORT_HEADERS_DIR   	${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
		${EXPORT_HEADERS_DIR}/conf_srvpv.h
		${EXPORT_HEADERS_DIR}/conf_define.h

)


 configure_file("${PROJECT_RESOURCES_DIR}/resources.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Profile.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Login.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Forbidden_Function.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Command_Lane_Type.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Auth_Fun.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Auth_Cmd.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_User_Allowed_Command.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Plaza.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Lane_Device.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Lane.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Display_Zone_Lane.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Display_Zone.c
	${PROJECT_SOURCES_DIR}/CONF_PV_Get_Display.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Plaza.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Payment_Sub_Type.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Payment.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Lane.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Display.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Currency.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Coin.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Class.c
	${PROJECT_SOURCES_DIR}/CONF_PLAZA_Get_Anomaly.c
	${PROJECT_SOURCES_DIR}/CONF_Main.c
	${PROJECT_SOURCES_DIR}/CONF_LANE_Get_Counter.c
	${PROJECT_SOURCES_DIR}/CONF_LANE_Get__Eqt.c
	${PROJECT_SOURCES_DIR}/CONF_ALARM_Get_Shift_And_Canopy_Events.c
	${PROJECT_SOURCES_DIR}/CONF_ALARM_Get_Event.c
	${PROJECT_SOURCES_DIR}/CONF_ALARM_Get_Alarm.c
	${PROJECT_SOURCES_DIR}/CONF_ALARM_Ack.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${SRVPV_CONF_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${SRVPV_CONF_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							#${PROJECT_HEADERS_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"	
							"${CMAKE_SOURCE_DIR}/DB Connection/includes"
							"${CMAKE_SOURCE_DIR}/DB Srvpv DBIF/includes"
							"${CMAKE_SOURCE_DIR}/includes"
)
 
target_compile_definitions(${SRVPV_CONF_LIB}
PRIVATE
    CONF_SRVPV_EXPORTS # Must be PRIVATE to be defined only during BUILD
	
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::SrvPVDBConf SHARED IMPORTED GLOBAL)
set_target_properties(CSR::SrvPVDBConf PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${EXPORT_HEADERS_DIR}"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_CONF_LIB}_r.lib"
					  IMPORTED_LOCATION_RELEASE								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_CONF_LIB}_r.dll"
					  IMPORTED_IMPLIB_DEBUG									
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_CONF_LIB}.lib"
					  IMPORTED_LOCATION_DEBUG								
					  	"${CMAKE_CURRENT_BINARY_DIR}/${SRVPV_CONF_LIB}.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${SRVPV_CONF_LIB}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${SRVPV_CONF_LIB}
    PUBLIC
		CSR::Des
		CSR::Excpt
		CSR::Acom
		CSR::Trc
	    CSR::Reg
		CSR::Cmdl
		CSR::Ntsvc
		CSR::MsgPV
		CSR::msg
		CSR::List
		CSR::Dbifora
		CSR::SrvPVDBConn
		CSR::SrvPVDBIFORA
)


set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${SRVPV_CONF_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${SRVPV_CONF_LIB}>
    COMMAND_EXPAND_LISTS
)

