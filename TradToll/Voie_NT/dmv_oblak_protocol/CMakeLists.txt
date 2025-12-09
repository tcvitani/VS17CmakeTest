cmake_minimum_required(VERSION 3.16)
project(dmv_oblak_protocol LANGUAGES CXX)

set(PROJECT_LIB "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

#set(CMAKE_AUTOUIC ON)
set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
 		Core5Compat
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
) 

qt_standard_project_setup()

set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_HEADERS
		${PROJECT_HEADERS_DIR}/dmv_oblak.h
		${PROJECT_HEADERS_DIR}/OblakMsg.h
		${PROJECT_HEADERS_DIR}/HelpFuncs.h
)



configure_file("${PROJECT_RESOURCES_DIR}/Resource.rc.in" Resources.rc @ONLY)
 
set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/dmv_oblak_main.c
	${PROJECT_SOURCES_DIR}/OblakMsg.cpp
	${PROJECT_SOURCES_DIR}/HelpFuncs.cpp

)


add_library(${PROJECT_LIB} SHARED
					${PROJECT_SOURCES} ${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_LIB} PRIVATE
							${PROJECT_SOURCES_DIR}
							${PROJECT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(${PROJECT_LIB}
PRIVATE
    DMV_OBLAK_MSG_LIB_EXPORT # Must be PRIVATE to be defined only during BUILD
)

##-----------------------------------------------------------------------------------------------

##-----------------------------------------------------------------------------------------------
							
#set_target_properties(${PROJECT_LIB}
#    PROPERTIES
#        WIN32_EXECUTABLE TRUE
#		RELEASE_POSTFIX "_r"            
#		MINSIZEREL_POSTFIX "_r"
#		RELWITHDEBINFO_POSTFIX "_r"
#)


target_link_libraries(${PROJECT_LIB}
    PUBLIC
        Qt::Core
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${PROJECT_LIB} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${PROJECT_LIB}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)