cmake_minimum_required(VERSION 3.15)
project(PipeTest C)

set(TEST_EXE "csr_${PROJECT_NAME}_vs17_x64")

#set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

#configure_file("${TEST_RESOURCES_DIR}/des_res.rc.in" TestResources.rc @ONLY)

# Create the library
add_executable(${TEST_EXE}  
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu.h
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu_glo.h
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu_init.c
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu_menu.c
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu_rec.c
	${CMAKE_CURRENT_SOURCE_DIR}/sources/simu_srv.c
	${CMAKE_CURRENT_SOURCE_DIR}/sources/test_lan.c
	${CMAKE_CURRENT_SOURCE_DIR}/sources/test_lan.h

	#${CMAKE_CURRENT_BINARY_DIR}/TestResources.rc 
) 

target_include_directories(${TEST_EXE} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/sources"
	${EXPORT_HEADERS_DIR}
	"${CMAKE_SOURCE_DIR}/../../../Commun_NT/declare/includes"	
	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/commun/includes"
	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/run/includes"

)


target_link_libraries(${TEST_EXE} 
	PUBLIC 
		${PROJECT_LIB}
		CSR::Noy


)

#target_compile_definitions(${TEST_EXE}
#PRIVATE
#    INIT_DEF # Must be PRIVATE to be defined only during BUILD
#)




# add "_r" before .exe for Release (and similar builds)
set_target_properties(${TEST_EXE} PROPERTIES
	WIN32_EXECUTABLE FALSE
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
