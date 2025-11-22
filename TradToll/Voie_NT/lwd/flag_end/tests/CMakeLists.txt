cmake_minimum_required(VERSION 3.15)
project(lwd_test C)

set(PROJECT_EXE "csr_${PROJECT_NAME}_vs17_x64")
 
configure_file("${PROJECT_RESOURCES_DIR}/flag_end.rc.in" Resources.rc @ONLY)
 

# Create the library
add_executable(${PROJECT_EXE}  
	${CMAKE_CURRENT_SOURCE_DIR}/sources/test.c 
	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
) 

target_include_directories(${PROJECT_EXE} PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	${PROJECT_RESOURCES_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../startup/includes
	${CMAKE_CURRENT_SOURCE_DIR}/../../startup/sources
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
	"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

target_link_libraries(${PROJECT_EXE} PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(${PROJECT_EXE} PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
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
