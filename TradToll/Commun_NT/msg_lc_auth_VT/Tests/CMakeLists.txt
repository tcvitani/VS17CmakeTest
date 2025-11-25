cmake_minimum_required(VERSION 3.15)
project(MsgLCAuthTest C)

set(PROJECT_TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")

set(TEST_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(TEST_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(TEST_RESOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/resources)

#-----------------------------------------------------
configure_file("${TEST_RESOURCES_DIR}/test.rc.in" TestResources.rc @ONLY)


# Create the library
add_executable(${PROJECT_TEST_EXE} 
	#${TEST_SOURCES_DIR}/testvar.c
	${TEST_SOURCES_DIR}/Test_auth_vt_v3.c
	${TEST_SOURCES_DIR}/Test_auth_vt_v2.c
	${TEST_SOURCES_DIR}/Test_auth_vt.c
	"${TEST_SOURCES_DIR}/test.c"

	${CMAKE_CURRENT_BINARY_DIR}/TestResources.rc 

) 

target_include_directories(${PROJECT_TEST_EXE} PRIVATE
	${EXPORT_HEADERS_DIR}
	${TEST_RESOURCES_DIR}
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
	"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/List/includes/"
)

target_link_libraries(${PROJECT_TEST_EXE} 
	PRIVATE 
	${PROJECT_LIB}
	 #CSR::msg
     #CSR::msg_lc
)

# add "_r" before .exe for Release (and similar builds)
set_target_properties(${PROJECT_TEST_EXE} PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_TEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_TEST_EXE}>
    COMMAND_EXPAND_LISTS
)
