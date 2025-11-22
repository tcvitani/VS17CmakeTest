cmake_minimum_required(VERSION 3.16)
project(BL_Consumer LANGUAGES CXX)

set(PROJECT_EXE "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Network
		Xml
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
)

qt_standard_project_setup()


set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
#set(PROJECT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
#set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

#set(PROJECT_ICON
#    "${PROJECT_RESOURCES_DIR}/icon.rc"
#)  
 
 
#configure_file("${PROJECT_RESOURCES_DIR}/Resources.rc.in" Resources.rc @ONLY)

#set_source_files_properties(QtWidgetsApplication1.cpp PROPERTIES COMPILE_OPTIONS "/Zc:strictStrings- /Zc:forScope /Zc:inline")
#set_source_files_properties(MInstTracer.cpp PROPERTIES COMPILE_OPTIONS "/Zc:strictStrings- /Zc:forScope /Zc:inline")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /Zc:strictStrings- /Zc:forScope /Zc:inline")
 

set(PROJECT_SOURCES
    ${PROJECT_SOURCES_DIR}/main_bl_test.cpp
 )



qt_add_executable(${PROJECT_EXE} 
					${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							#${PROJECT_HEADERS_DIR}
							#${PROJECT_RESOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

##-----------------------------------------------------------------------------------------------

							
##-----------------------------------------------------------------------------------------------
							
#function(set_debugger_environment)
#	cmake_parse_arguments(ARG "" "TARGET" "DEPENDENCIES" ${ARGN})
#	message(STATUS "Updated debugger environment: add ${ARG_DEPENDENCIES} to the path of ${ARG_TARGET}")
#	foreach(dependency IN LISTS ARG_DEPENDENCIES)
#		set_target_properties(${ARG_TARGET} 
#			PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:${dependency}>"
#		)
#	endforeach()
#endfunction()


set_target_properties(${PROJECT_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)

#set_target_properties(${PROJECT_EXE}
#    PROPERTIES
# 		VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;C:\\csr\\exe;"
#)
#
#get_target_property(MY_DEBUGGER_ENVIRONMENT ${PROJECT_EXE} VS_DEBUGGER_ENVIRONMENT)
#
#message(STATUS "Updated debugger environment: add ${MY_DEBUGGER_ENVIRONMENT}")

#get_target_property(third-party_interface_compile_options ${PROJECT_EXE} CMAKE_CXX_FLAGS)
#list(REMOVE_ITEM third-party_interface_compile_options "-permissive-")
#message("third-party_interface_compile_options: ${third-party_interface_compile_options}")
#set_target_properties(${PROJECT_EXE} PROPERTIES INTERFACE_COMPILE_OPTIONS "${third-party_interface_compile_options}")

#set_target_properties(Qt6::Core PROPERTIES INTERFACE_COMPILE_OPTIONS "")
#set_target_properties(Qt6::Gui PROPERTIES INTERFACE_COMPILE_OPTIONS "")
#set_target_properties(Qt6::Widgets PROPERTIES INTERFACE_COMPILE_OPTIONS "")
#set_target_properties(Qt6::Xml PROPERTIES INTERFACE_COMPILE_OPTIONS "")
#set_target_properties(CSR::Trc PROPERTIES INTERFACE_COMPILE_OPTIONS "")
#set_target_properties(CSR::Reg PROPERTIES INTERFACE_COMPILE_OPTIONS "")

target_link_libraries(${PROJECT_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Network
		Qt::Xml
		CSR::Trc
	    CSR::Reg
	PRIVATE
		${PROJECT_LIB}
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

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)