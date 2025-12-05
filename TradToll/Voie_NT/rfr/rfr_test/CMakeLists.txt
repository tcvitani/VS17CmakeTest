cmake_minimum_required(VERSION 3.16)
project(TestRfr LANGUAGES C)


set(TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)


 configure_file("${PROJECT_RESOURCES_DIR}/rfr_test.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/simu0.c
	${PROJECT_SOURCES_DIR}/simu_rfr.c
	${PROJECT_SOURCES_DIR}/simu_rec.c
	${PROJECT_SOURCES_DIR}/rfr_test.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	${PROJECT_HEADERS_DIR}/simu0.h
	${PROJECT_HEADERS_DIR}/simu_glo.h
	${PROJECT_HEADERS_DIR}/rfr_test.h
)


add_executable(${TEST_EXE} ${PROJECT_SOURCES} ${PROJECT_HEADERS} 	

)

target_include_directories(${TEST_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/run/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/rfr/rfr_interface/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/lan/lan_interface"
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
							
set_target_properties(${TEST_EXE}
    PROPERTIES
        WIN32_EXECUTABLE FALSE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${TEST_EXE}
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
    TARGET ${TEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${TEST_EXE}>
        $<TARGET_FILE:${TEST_EXE}>
    COMMAND_EXPAND_LISTS
)

