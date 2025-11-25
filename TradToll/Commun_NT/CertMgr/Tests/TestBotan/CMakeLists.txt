cmake_minimum_required(VERSION 3.15)
project(TestBotan CXX)

set(TEST_BOTAN_EXE "csr_${PROJECT_NAME}_vs17_x64")

#set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

#configure_file("${TEST_RESOURCES_DIR}/aes_crypt_res.rc.in" TestResources.rc @ONLY)

# Create the library
add_executable(${TEST_BOTAN_EXE}  
	${CMAKE_CURRENT_SOURCE_DIR}/sources/TestBotan.cpp
	#${CMAKE_CURRENT_BINARY_DIR}/TestResources.rc 
) 

target_include_directories(${TEST_BOTAN_EXE} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/sources"
    #${TEST_RESOURCES_DIR}
	${EXPORT_HEADERS_DIR}
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
)

#message("Project_LIB is: ${CRYPT_LIB} target is:${TEST_BOTAN_EXE}")

target_link_libraries(${TEST_BOTAN_EXE} 
	PUBLIC 
		ThrdPty::Botan1
)

# add "_r" before .exe for Release (and similar builds)
set_target_properties(${TEST_BOTAN_EXE} PROPERTIES
	WIN32_EXECUTABLE FALSE
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${TEST_BOTAN_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${TEST_BOTAN_EXE}>
    COMMAND_EXPAND_LISTS
)
