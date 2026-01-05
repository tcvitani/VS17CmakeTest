cmake_minimum_required(VERSION 3.16)
project(TestDioMain LANGUAGES C)


set(TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")
 

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)


 configure_file("${PROJECT_RESOURCES_DIR}/dio_test.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/main.c

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)


add_executable(${TEST_EXE} ${PROJECT_SOURCES} ${PROJECT_HEADERS} 	

)

target_include_directories(${TEST_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"		
							"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dio_gen/dio_interface/includes"
)
 

##-----------------------------------------------------------------------------------------------

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
	  ${PROJECT_LIB}
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

