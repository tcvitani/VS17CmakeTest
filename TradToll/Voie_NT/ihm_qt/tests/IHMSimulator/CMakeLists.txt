cmake_minimum_required(VERSION 3.16)
project(IHMSimulator LANGUAGES CXX)

set(SIMULATOR_EXE "${PROJECT_NAME}_vs22_x64")

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Xml
		Core5Compat
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
) 

qt_standard_project_setup()


set(SIMULATOR_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(SIMULATOR_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

 
configure_file("${SIMULATOR_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)


set(PROJECT_SOURCES
	${SIMULATOR_SOURCES_DIR}/waitUser.ui
	${SIMULATOR_SOURCES_DIR}/videodlg.ui
	${SIMULATOR_SOURCES_DIR}/touchedlg.ui
	${SIMULATOR_SOURCES_DIR}/scriptdlg.ui
	${SIMULATOR_SOURCES_DIR}/saisieexdlg.ui
	${SIMULATOR_SOURCES_DIR}/saisiedlg.ui
	${SIMULATOR_SOURCES_DIR}/policedlg.ui
	${SIMULATOR_SOURCES_DIR}/maindlg.ui
	${SIMULATOR_SOURCES_DIR}/LinkDlg.ui
	${SIMULATOR_SOURCES_DIR}/detectiondlg.ui
	${SIMULATOR_SOURCES_DIR}/config.ui
	${SIMULATOR_SOURCES_DIR}/authorization.ui
	${SIMULATOR_SOURCES_DIR}/AsyncInputDlg.ui
	${SIMULATOR_SOURCES_DIR}/affichage.ui	
	${SIMULATOR_SOURCES_DIR}/videodlg.cpp
	${SIMULATOR_SOURCES_DIR}/touchedlg.cpp
	${SIMULATOR_SOURCES_DIR}/SimuSendReciveThread.cpp
	${SIMULATOR_SOURCES_DIR}/saisieexdlg.cpp
	${SIMULATOR_SOURCES_DIR}/saisiedlg.cpp
	${SIMULATOR_SOURCES_DIR}/policedlg.cpp
	${SIMULATOR_SOURCES_DIR}/MWaitUserDlg.cpp
	${SIMULATOR_SOURCES_DIR}/maindlg.cpp
	${SIMULATOR_SOURCES_DIR}/main.cpp
	${SIMULATOR_SOURCES_DIR}/linkdlg.cpp
	${SIMULATOR_SOURCES_DIR}/IHMSimuDld.cpp
	${SIMULATOR_SOURCES_DIR}/detectiondlg.cpp
	${SIMULATOR_SOURCES_DIR}/config.cpp
	${SIMULATOR_SOURCES_DIR}/authorization.cpp
	${SIMULATOR_SOURCES_DIR}/AsyncInputDlg.cpp
	${SIMULATOR_SOURCES_DIR}/affichage.cpp
 	${SIMULATOR_SOURCES_DIR}/videodlg.h
	${SIMULATOR_SOURCES_DIR}/touchedlg.h
	${SIMULATOR_SOURCES_DIR}/SimuSendReciveThread.h
	${SIMULATOR_SOURCES_DIR}/saisieexdlg.h
	${SIMULATOR_SOURCES_DIR}/saisiedlg.h
	${SIMULATOR_SOURCES_DIR}/policedlg.h
	${SIMULATOR_SOURCES_DIR}/MWaitUserDlg.h
	${SIMULATOR_SOURCES_DIR}/maindlg.h
	${SIMULATOR_SOURCES_DIR}/linkdlg.h
	${SIMULATOR_SOURCES_DIR}/IHMSimuDld.h
	${SIMULATOR_SOURCES_DIR}/detectiondlg.h
	${SIMULATOR_SOURCES_DIR}/constants.h
	${SIMULATOR_SOURCES_DIR}/config.h
	${SIMULATOR_SOURCES_DIR}/authorization.h
	${SIMULATOR_SOURCES_DIR}/AsyncInputDlg.h
	${SIMULATOR_SOURCES_DIR}/affichage.h

)


qt_add_executable(${SIMULATOR_EXE} 
					${PROJECT_SOURCES} 
)

target_include_directories(${SIMULATOR_EXE} PRIVATE
							${SIMULATOR_SOURCES_DIR}
							${SIMULATOR_RESOURCES_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							#${EXPORT_HEADERS_DIR}
							${CMAKE_SOURCE_DIR}/includes
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/run/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/virtualKYB/includes"
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/lwd/startup/includes"				
)

##-----------------------------------------------------------------------------------------------
												
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${SIMULATOR_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)


target_link_libraries(${SIMULATOR_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Core5Compat
		CSR::Noy
		CSR::Srv
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${SIMULATOR_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${SIMULATOR_EXE}>
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${PROJECT_EXE} $<TARGET_FILE_DIR:${PROJECT_EXE}>)