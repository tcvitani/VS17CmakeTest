cmake_minimum_required(VERSION 3.16)
project(acom LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${PROJECT_HEADERS_DIR}/ACOM.H
)


 configure_file("${PROJECT_RESOURCES_DIR}/Resources.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/acom.c
	${PROJECT_SOURCES_DIR}/acom_block.c
	${PROJECT_SOURCES_DIR}/acom_clbk.c
	${PROJECT_SOURCES_DIR}/acom_dbg.c
	${PROJECT_SOURCES_DIR}/acom_init.c
	${PROJECT_SOURCES_DIR}/acom_io.c
	${PROJECT_SOURCES_DIR}/acom_key.c
	${PROJECT_SOURCES_DIR}/acom_mail.c
	${PROJECT_SOURCES_DIR}/acom_pipe.c
	${PROJECT_SOURCES_DIR}/acom_queue.c
	${PROJECT_SOURCES_DIR}/acom_sec.c
	${PROJECT_SOURCES_DIR}/acom_serial.c
	${PROJECT_SOURCES_DIR}/acom_stats.c
	${PROJECT_SOURCES_DIR}/acom_tcp.c
	${PROJECT_SOURCES_DIR}/acom_timer.c
	${PROJECT_SOURCES_DIR}/acom_worker.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	${PROJECT_HEADERS_DIR}/acom_block.h
	${PROJECT_HEADERS_DIR}/acom_clbk.h
	${PROJECT_HEADERS_DIR}/acom_dbg.h
	${PROJECT_HEADERS_DIR}/acom_dmem.h
	${PROJECT_HEADERS_DIR}/acom_init.h
	${PROJECT_HEADERS_DIR}/acom_io.h
	${PROJECT_HEADERS_DIR}/acom_key.h
	${PROJECT_HEADERS_DIR}/acom_mail.h
	${PROJECT_HEADERS_DIR}/acom_pipe.h
	${PROJECT_HEADERS_DIR}/acom_priv.h
	${PROJECT_HEADERS_DIR}/acom_queue.h
	${PROJECT_HEADERS_DIR}/acom_sec.h
	${PROJECT_HEADERS_DIR}/acom_serial.h
	${PROJECT_HEADERS_DIR}/acom_stats.h
	${PROJECT_HEADERS_DIR}/acom_tcp.h
	${PROJECT_HEADERS_DIR}/acom_timer.h
	${PROJECT_HEADERS_DIR}/acom_worker.h

)

add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${PROJECT_HEADERS}	${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)
  
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    ACOM_EXPORTS # Must be PRIVATE to be defined only during BUILD
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
        CSR::Excpt
		CSR::Trc
	    CSR::Reg
		wsock32 
		ws2_32

)

add_subdirectory(tests)
add_subdirectory(Tests_Unittest)


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