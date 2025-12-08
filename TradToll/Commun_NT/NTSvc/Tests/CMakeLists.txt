cmake_minimum_required(VERSION 3.15)
project(NtSvcTest C)

set(TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")

set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

configure_file("${TEST_RESOURCES_DIR}/resource.rc.in" TestResources.rc @ONLY)

# Create the library
add_executable(${TEST_EXE}  
	${CMAKE_CURRENT_SOURCE_DIR}/sources/testsvc_main.c

	${CMAKE_CURRENT_BINARY_DIR}/TestResources.rc 
) 

target_include_directories(${TEST_EXE} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/sources"
	${EXPORT_HEADERS_DIR}
	${TEST_RESOURCES_DIR}
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
)

 
target_link_libraries(${TEST_EXE} 
	PUBLIC 
		${PROJECT_LIB}
		CSR::Trc
		CSR::Reg
		CSR::Excpt
		CSR::Cmdl

)

# add "_r" before .exe for Release (and similar builds)
set_target_properties(${TEST_EXE} PROPERTIES
	WIN32_EXECUTABLE TRUE	
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${TEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${TEST_EXE}>
    COMMAND_EXPAND_LISTS
)
