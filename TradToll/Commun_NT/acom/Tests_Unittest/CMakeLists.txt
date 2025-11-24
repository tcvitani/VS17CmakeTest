cmake_minimum_required(VERSION 3.15)
project(ACOMUnitTest C)

set(PROJECT_UNITTEST_EXE "csr_${PROJECT_NAME}_vs17_x64")

# set(INCLUDES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/includes")
#-----------------------------------------------------

# Create the library
add_executable(${PROJECT_UNITTEST_EXE} "${CMAKE_CURRENT_SOURCE_DIR}/Sources/main.c") 

target_include_directories(${PROJECT_UNITTEST_EXE} PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
	"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

target_link_libraries(${PROJECT_UNITTEST_EXE} PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(${PROJECT_UNITTEST_EXE} PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_UNITTEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_UNITTEST_EXE}>
    COMMAND_EXPAND_LISTS
)
