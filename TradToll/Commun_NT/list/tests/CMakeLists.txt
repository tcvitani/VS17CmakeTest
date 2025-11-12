cmake_minimum_required(VERSION 3.15)
project(ListTest C)



# Create the library
add_executable(ListTest  ${CMAKE_CURRENT_SOURCE_DIR}/sources/test_list.c ) 

target_include_directories(ListTest PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	L:/Commun_NT/declare/10.0.0
	L:/Voie_NT/commun/3.1.0							
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
