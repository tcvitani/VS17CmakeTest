cmake_minimum_required(VERSION 3.16)
project(tag LANGUAGES C)



set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
	${EXPORT_HEADERS_DIR}/CSR_TAG_TRSP.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_SER.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_QFSL7.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_QFL2.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_QFDMI.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_ISO.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_ISA.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_IPGEA.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_CENL7.h
	${EXPORT_HEADERS_DIR}/CSR_TAG_BAC.h
	${EXPORT_HEADERS_DIR}/CSR_TAG.h
)


 configure_file("${PROJECT_RESOURCES_DIR}/TAG_res.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/serudp.h
	${PROJECT_SOURCES_DIR}/sercom.h
	${PROJECT_SOURCES_DIR}/ser.h
	${PROJECT_SOURCES_DIR}/QFDMI_http_client.h
	${PROJECT_SOURCES_DIR}/Dll_L7.h
	${PROJECT_SOURCES_DIR}/DLL_I2.h
	${PROJECT_SOURCES_DIR}/AUTO.h
	${PROJECT_SOURCES_DIR}/TRSP.c
	${PROJECT_SOURCES_DIR}/TAG.c
	${PROJECT_SOURCES_DIR}/serudp.c
	${PROJECT_SOURCES_DIR}/sercom.c
	${PROJECT_SOURCES_DIR}/SER.c
	${PROJECT_SOURCES_DIR}/QFSL7.c
	${PROJECT_SOURCES_DIR}/QFL2.c
	${PROJECT_SOURCES_DIR}/QFDMI_http_client.c
	${PROJECT_SOURCES_DIR}/QFDMI.c
	${PROJECT_SOURCES_DIR}/iso.c
	${PROJECT_SOURCES_DIR}/ISA.c
	${PROJECT_SOURCES_DIR}/csr_tag_ipgea.c
	${PROJECT_SOURCES_DIR}/cenl7security.c
	${PROJECT_SOURCES_DIR}/cenl7build.c
	${PROJECT_SOURCES_DIR}/cenl7analyse.c
	${PROJECT_SOURCES_DIR}/CENL7.c
	${PROJECT_SOURCES_DIR}/BAC.c
	${PROJECT_SOURCES_DIR}/AUTO.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
  #no private headers
)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
							"${CMAKE_SOURCE_DIR}/../../_Third_Party_Libraries/GEA_IP_BEACON/BeaconManager/3.0.0"
)
 
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    TAG_EXPORTS # Must be PRIVATE to be defined only during BUILD
)



##-----------------------------------------------------------------------------------------------
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


							
add_library(CSR::BitBl SHARED IMPORTED GLOBAL)
set_target_properties(CSR::BitBl PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/bitbl/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/bitbl/out/build/x64-release/csr_bitbl_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/bitbl/out/build/x64-release/csr_bitbl_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/bitbl/out/build/x64-debug/csr_bitbl_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/bitbl/out/build/x64-debug/csr_bitbl_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

							
add_library(CSR::Des SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Des PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/des/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/des/out/build/x64-release/csr_des_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/des/out/build/x64-release/csr_des_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/des/out/build/x64-debug/csr_des_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/des/out/build/x64-debug/csr_des_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

							
add_library(CSR::Queuemt SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Queuemt PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/queue/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/queue/out/build/x64-release/csr_queuemt_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/queue/out/build/x64-release/csr_queuemt_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/queue/out/build/x64-debug/csr_queuemt_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/queue/out/build/x64-debug/csr_queuemt_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::Timermt SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Timermt PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/timer/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/timer/out/build/x64-release/csr_timermt_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/timer/out/build/x64-release/csr_timermt_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/timer/out/build/x64-debug/csr_timermt_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/timer/out/build/x64-debug/csr_timermt_VS17_x64.dll"
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
        CSR::Trc
		CSR::Reg
		CSR::Excpt
		CSR::BitBl
		CSR::Des
		CSR::Queuemt
		CSR::Timermt
		winmm.lib
		#Ws2_32.lib
		Mswsock.lib

)

add_subdirectory("tests")


set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

