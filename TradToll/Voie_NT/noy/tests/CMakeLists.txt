cmake_minimum_required(VERSION 3.15)
project(NoyTest C)


# set(INCLUDES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/includes")
#-----------------------------------------------------

# Create the library
add_executable(NoyTest test.c ) 

target_include_directories(NoyTest PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	L:/Commun_NT/declare/10.0.0
	L:/Voie_NT/commun/3.1.0							
)

target_link_libraries(NoyTest PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(NoyTest PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET NoyTest POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:NoyTest>
    COMMAND_EXPAND_LISTS
)
#-----------------------------------------------------

add_executable(NoyTestBal test_bal.c ) 

target_include_directories(NoyTestBal PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	L:/Commun_NT/declare/10.0.0
	L:/Voie_NT/commun/3.1.0							
)

target_link_libraries(NoyTestBal PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(NoyTestBal PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)



add_custom_command(
    TARGET NoyTestBal POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:NoyTestBal>
    COMMAND_EXPAND_LISTS
)
#-----------------------------------------------------
add_executable(NoyTestChrono test_chrono.c ) 

target_include_directories(NoyTestChrono PRIVATE
    #${INCLUDES_DIR}
	${EXPORT_HEADERS_DIR}
	L:/Commun_NT/declare/10.0.0
	L:/Voie_NT/commun/3.1.0							
)

target_link_libraries(NoyTestChrono PRIVATE ${PROJECT_LIB})

# add "_r" before .exe for Release (and similar builds)
set_target_properties(NoyTestChrono PROPERTIES
    RELEASE_POSTFIX "_r"            
    MINSIZEREL_POSTFIX "_r"
    RELWITHDEBINFO_POSTFIX "_r"
)

add_custom_command(
    TARGET NoyTestChrono POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:NoyTestChrono>
    COMMAND_EXPAND_LISTS
)
#-----------------------------------------------------

