cmake_minimum_required(VERSION 3.16)
project(TestTlmDmv LANGUAGES C)


set(TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")
 

set(TEST_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(TEST_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)


 configure_file("${TEST_RESOURCES_DIR}/Resource.rc.in" TestResources.rc @ONLY)
 

set(PROJECT_SOURCES
	${TEST_SOURCES_DIR}/Test_ani.c
	${TEST_SOURCES_DIR}/Test_main.c

	${CMAKE_CURRENT_BINARY_DIR}/TestResources.rc
)

set(PROJECT_HEADERS
	${TEST_HEADERS_DIR}/test.h
	${TEST_HEADERS_DIR}/Test_ani.h
)


add_executable(${TEST_EXE} ${PROJECT_SOURCES} ${PROJECT_HEADERS} 	

)

target_include_directories(${TEST_EXE} PRIVATE
							${TEST_SOURCES_DIR}
							${TEST_HEADERS_DIR}
							${TEST_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/run/includes"
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/tlm/tlm_interface"
)
 

##-----------------------------------------------------------------------------------------------

##-----------------------------------------------------------------------------------------------
							
set_target_properties(${TEST_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

target_link_libraries(${TEST_EXE}
    PRIVATE
		${PROJECT_LIB}
      CSR::Noy
	  CSR::Reg

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

