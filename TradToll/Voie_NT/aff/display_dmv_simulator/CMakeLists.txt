cmake_minimum_required(VERSION 3.16)
project(DmvSimulator VERSION 20.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(qt.cmake)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Quick
		Core
		Network
)
qt_standard_project_setup()

set(DMV_SIMULATOR_EXE "csr_DmvSimulator_vs22_x64")

set(DMV_SIMULATOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DMV_SIMULATOR_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cfg)
set(DMV_SIMULATOR_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(PROJECT_SOURCES
    ${DMV_SIMULATOR_SOURCE_DIR}/main.cpp
	${DMV_SIMULATOR_SOURCE_DIR}/UdpServer/UdpServer.cpp
    ${DMV_SIMULATOR_SOURCE_DIR}/Worker/Worker.cpp
)

set(PROJECT_INCLUDES
    ${DMV_SIMULATOR_SOURCE_DIR}/Worker/Worker.h
    ${DMV_SIMULATOR_SOURCE_DIR}/UdpServer/UdpServer.h
)

set(PROJECT_CONFIGS
    ${DMV_SIMULATOR_CONFIG_DIR}/cfg.json
)

set(PROJECT_ICON
    "${DMV_SIMULATOR_RESOURCE_DIR}/icon.rc"
) 

configure_file("${DMV_SIMULATOR_RESOURCE_DIR}/DmvSimulator.rc.in" DmvSimulator.rc @ONLY)

if(QT_VERSION VERSION_LESS 6.2)
    list(APPEND PROJECT_SOURCES qml.qrc)
endif()

qt_add_executable(${DMV_SIMULATOR_EXE} 
                  ${PROJECT_SOURCES}
                  ${PROJECT_INCLUDES}
                  ${CMAKE_CURRENT_BINARY_DIR}/DmvSimulator.rc
                  ${PROJECT_ICON}
)

target_include_directories(${DMV_SIMULATOR_EXE} PRIVATE
                           ${DMV_SIMULATOR_SOURCE_DIR}
                           ${DMV_SIMULATOR_RESOURCE_DIR}      # Used by version.rc to include resource.h
)


set_target_properties(${DMV_SIMULATOR_EXE} PROPERTIES
                      WIN32_EXECUTABLE TRUE # If you remove this, the app will launch as a windows command line and a separate gui
                      VERSION ${CMAKE_PROJECT_VERSION}
                      DESCRIPTION "DMV External display simulator"
                      LANGUAGES CXX
                      RELEASE_POSTFIX "_r"
                      MISIZEREL_POSTFIX "_r"
                      RELWITHDEBINFO_POSTFIX "_r"
)

# Pre/Post Build steps

## Copy all Qt runtime dependecies to build directory (including QtPlugins)
## If you are using qml, use the --qmldir flag
#set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")
#set(Qt6_BIN_DIR ${Qt6_DIR}/../../../bin)
#add_custom_command(
#    TARGET ${DMV_SIMULATOR_EXE} POST_BUILD
#    COMMAND ${Qt6_BIN_DIR}/windeployqt.exe --qmldir . --dir ${DEPLOY_DIR} "$<TARGET_FILE:${DMV_SIMULATOR_EXE}>" 
#            --no-translations 
#            --no-opengl-sw  
#            --no-system-d3d-compiler 
#)

# Copy executable to DEPLOY_DIR
#add_custom_command(
#	TARGET ${DMV_SIMULATOR_EXE} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${DMV_SIMULATOR_EXE}> ${DEPLOY_DIR}/$<TARGET_FILE_NAME:${DMV_SIMULATOR_EXE}>
#)
############################################################################################
add_library(CSR::dmv_oblak_protocol SHARED IMPORTED GLOBAL)
set_target_properties(CSR::dmv_oblak_protocol PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/release/dmv_oblak_protocol_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/release/dmv_oblak_protocol_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/debug/dmv_oblak_protocol_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../../Voie_NT/dmv_oblak_protocol/out/build/debug/dmv_oblak_protocol_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)	
############################################################################################
# Copy all config file to build directory

target_link_libraries(${DMV_SIMULATOR_EXE}
    PUBLIC
		Qt::Core
		Qt6::Network
        Qt6::Quick
        Qt6::QuickControls2
        CSR::dmv_oblak_protocol
)

if(Qt6_FOUND)
    qt_import_qml_plugins(${DMV_SIMULATOR_EXE})
endif()

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")

add_custom_command(
    TARGET ${DMV_SIMULATOR_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
		-E copy 
		-t ${CMAKE_INSTALL_PREFIX} 
		${CMAKE_SOURCE_DIR}/cfg/cfg.json
	COMMAND_EXPAND_LISTS
)

add_custom_command(
    TARGET ${DMV_SIMULATOR_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${DMV_SIMULATOR_EXE}>
    COMMAND_EXPAND_LISTS
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(${DMV_SIMULATOR_EXE}
        URI dmvsimulator
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml/"
        QML_FILES
            main.qml
    )
endif()
