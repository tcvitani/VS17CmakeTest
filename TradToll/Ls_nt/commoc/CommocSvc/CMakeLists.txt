cmake_minimum_required(VERSION 3.16)
project(CommocSvc LANGUAGES C )

set(PROJECT_EXE "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_HEADERS
    ${PROJECT_HEADERS_DIR}/cm_callback.h
    ${PROJECT_HEADERS_DIR}/cm_cmd.h
    ${PROJECT_HEADERS_DIR}/cm_dirs.h
    ${PROJECT_HEADERS_DIR}/cm_glob.h
    ${PROJECT_HEADERS_DIR}/cm_hosts.h
    ${PROJECT_HEADERS_DIR}/cm_main.h
    ${PROJECT_HEADERS_DIR}/cm_text.h
    ${PROJECT_HEADERS_DIR}/cm_threads.h
    ${PROJECT_HEADERS_DIR}/cm_utl.h
    ${PROJECT_HEADERS_DIR}/cm_workers.h
)

 configure_file("${PROJECT_RESOURCES_DIR}/cm_res.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/cm_callback.c
	${PROJECT_SOURCES_DIR}/cm_cmd.c
	${PROJECT_SOURCES_DIR}/cm_dirs.c
	${PROJECT_SOURCES_DIR}/cm_hosts.c
	${PROJECT_SOURCES_DIR}/cm_main.c
	${PROJECT_SOURCES_DIR}/cm_text.c
	${PROJECT_SOURCES_DIR}/cm_threads.c
	${PROJECT_SOURCES_DIR}/cm_utl.c
	${PROJECT_SOURCES_DIR}/cm_wokers.c

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
)
 

##-----------------------------------------------------------------------------------------------



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

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${PROJECT_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)



target_link_libraries(${PROJECT_EXE}
    PUBLIC
		CSR::Excpt
		CSR::Trc
	    CSR::Reg
		CSR::Acom
		CSR::Cmdl
		CSR::Ntsvc
		CSR::Col
		IF_CM_Hosts
		IF_CM_Workers
)



set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_EXE}>
    COMMAND_EXPAND_LISTS
)


