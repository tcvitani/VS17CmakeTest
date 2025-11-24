cmake_minimum_required(VERSION 3.15)
project(ListTest C)



# Create the library
add_executable(ListTest  ${CMAKE_CURRENT_SOURCE_DIR}/sources/test_list.c ) 

target_include_directories(ListTest PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"		
	"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"		
					
)

target_link_libraries(ListTest PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(ListTest PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ListTest POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:ListTest>
    COMMAND_EXPAND_LISTS
)
