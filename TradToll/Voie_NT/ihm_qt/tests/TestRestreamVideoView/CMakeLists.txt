cmake_minimum_required(VERSION 3.16)
project(TestRestreamVideoView LANGUAGES CXX)

set(TEST_RVIDEO_EXE "${PROJECT_NAME}_vs17_x64")

include(qt.cmake)

set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Network
		Xml
		UiTools
		Core5Compat
		Qml
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
)
qt_standard_project_setup()


#set(SIMULATOR_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

 
#configure_file("${SIMULATOR_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 
#qt_wrap_ui(SIMULATOR_UI_GEN_HEADERS
#	${SIMULATOR_SOURCES_DIR}/maindlg.ui	
#	${SIMULATOR_SOURCES_DIR}/affichage.ui		
#	${SIMULATOR_SOURCES_DIR}/waitUser.ui
#	${SIMULATOR_SOURCES_DIR}/videodlg.ui
#	${SIMULATOR_SOURCES_DIR}/touchedlg.ui
#	${SIMULATOR_SOURCES_DIR}/scriptdlg.ui
#	${SIMULATOR_SOURCES_DIR}/saisieexdlg.ui
#	${SIMULATOR_SOURCES_DIR}/saisiedlg.ui
#	${SIMULATOR_SOURCES_DIR}/policedlg.ui
#	${SIMULATOR_SOURCES_DIR}/LinkDlg.ui
#	${SIMULATOR_SOURCES_DIR}/detectiondlg.ui
#	${SIMULATOR_SOURCES_DIR}/config.ui
#	${SIMULATOR_SOURCES_DIR}/authorization.ui
#	${SIMULATOR_SOURCES_DIR}/AsyncInputDlg.ui
#
#)

set(PROJECT_SOURCES
	#${SIMULATOR_UI_GEN_HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sources/TestVideoDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sources/TestVideoDialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/sources/TestVideoDialog.ui


	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MTracer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/mautomat.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoView.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoCmdThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoMsg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoCmdAutomate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoControlObject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/mjpegstreamer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MTracer.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/mautomat.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoView.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoCmdThread.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoMsg.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoCmdAutomate.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/MRVideoControlObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../sources/mjpegstreamer.h

)


qt_add_executable(${TEST_RVIDEO_EXE} 
					${PROJECT_SOURCES} 
)

target_include_directories(${TEST_RVIDEO_EXE} PRIVATE
							${CMAKE_CURRENT_SOURCE_DIR}/sources
							${CMAKE_CURRENT_BINARY_DIR}
							${CMAKE_CURRENT_SOURCE_DIR}/../../sources/
							${CMAKE_CURRENT_SOURCE_DIR}/../../includes/
							#${EXPORT_HEADERS_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

##-----------------------------------------------------------------------------------------------
															
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${TEST_RVIDEO_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)
 

target_link_libraries(${TEST_RVIDEO_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Network
		Qt::Core5Compat
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${TEST_RVIDEO_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${TEST_RVIDEO_EXE}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)