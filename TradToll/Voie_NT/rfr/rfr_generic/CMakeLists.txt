cmake_minimum_required(VERSION 3.16)
project(rfr_generic LANGUAGES C)


set(PROJECT_LIB "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(EXPORT_HEADERS_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/includes"	)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
		${EXPORT_HEADERS_DIR}/rfr_generic.h

)


 configure_file("${PROJECT_RESOURCES_DIR}/rfr_generic.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/rfr_generic.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_library(${PROJECT_LIB} SHARED ${PROJECT_SOURCES} ${EXPORT_HEADERS} 			 		
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							#${PROJECT_HEADERS_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/run/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/rfr_interface/includes"
)
 
target_compile_definitions(${PROJECT_LIB}
PRIVATE
    RFR_GENERIC_EXPORTS # Must be PRIVATE to be defined only during BUILD
	
)



##-----------------------------------------------------------------------------------------------
add_library(CSR::Noy SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Noy PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	 						
add_library(CSR::fonct_c SHARED IMPORTED GLOBAL)
set_target_properties(CSR::fonct_c PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fonct_c/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fonct_c/out/build/x64-release/csr_fonct_c_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fonct_c/out/build/x64-release/csr_fonct_c_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fonct_c/out/build/x64-debug/csr_fonct_c_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fonct_c/out/build/x64-debug/csr_fonct_c_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)		

add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							

add_library(CSR::Trc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Trc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::hrd SHARED IMPORTED GLOBAL)
set_target_properties(CSR::hrd PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/hrd/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/hrd/out/build/x64-release/csr_hrd_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/hrd/out/build/x64-release/csr_hrd_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/hrd/out/build/x64-debug/csr_hrd_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/hrd/out/build/x64-debug/csr_hrd_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::fic_conf SHARED IMPORTED GLOBAL)
set_target_properties(CSR::fic_conf PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_conf/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_conf/out/build/x64-release/csr_fic_conf_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_conf/out/build/x64-release/csr_fic_conf_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_conf/out/build/x64-debug/csr_fic_conf_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_conf/out/build/x64-debug/csr_fic_conf_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::fic_gere SHARED IMPORTED GLOBAL)
set_target_properties(CSR::fic_gere PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_gere/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_gere/out/build/x64-release/csr_fic_gere_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_gere/out/build/x64-release/csr_fic_gere_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_gere/out/build/x64-debug/csr_fic_gere_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/fic_gere/out/build/x64-debug/csr_fic_gere_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

add_library(CSR::files_tools SHARED IMPORTED GLOBAL)
set_target_properties(CSR::files_tools PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/files_tools/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/files_tools/out/build/x64-release/csr_files_tools_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/files_tools/out/build/x64-release/csr_files_tools_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/files_tools/out/build/x64-debug/csr_files_tools_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/files_tools/out/build/x64-debug/csr_files_tools_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)

add_library(CSR::List STATIC IMPORTED GLOBAL)
set_target_properties(CSR::List PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/List/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/List/out/build/release/csr_List_VS17_x64_r.lib"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/List/out/build/debug/csr_List_VS17_x64.lib"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	

add_library(CSR::referenc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::referenc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/rfr_interface/includes"
					  IMPORTED_IMPLIB_RELEASE				 
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/referenc/out/build/x64-release/csr_referenc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE				 
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/referenc/out/build/x64-release/csr_referenc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG					 
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/referenc/out/build/x64-debug/csr_referenc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG				 
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/referenc/out/build/x64-debug/csr_referenc_VS17_x64.dll"
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
	  CSR::List
      CSR::Noy
	  CSR::fonct_c
	  CSR::Trc
	  CSR::Reg
	  CSR::hrd
	  CSR::fic_conf
	  CSR::fic_gere	  
	  CSR::files_tools
	  CSR::referenc
)

#add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

