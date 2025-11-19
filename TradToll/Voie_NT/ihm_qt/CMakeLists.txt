cmake_minimum_required(VERSION 3.16)
project(ihm_qt LANGUAGES CXX C )

set(PROJECT_EXE "csr_${PROJECT_NAME}_vs17_x64")
 
include(qt.cmake)

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
		AxContainer
		Qml
PATHS
    $<$<PLATFORM_ID:Windows>:"C:/QT/6.5.2/lib/cmake"
)

qt_standard_project_setup()




set(PROJECT_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(EXPORT_HEADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#set(TestForm_CONFIG_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/others)
set(PROJECT_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(EXPORT_HEADERS
    ${EXPORT_HEADERS_DIR}/ihm.h
	    ${EXPORT_HEADERS_DIR}/MIHMPluginINT.h
	    ${EXPORT_HEADERS_DIR}/MIHMPluginDlgLogicINT.h

)

 configure_file("${PROJECT_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 

set(PROJECT_SOURCES
	${PROJECT_SOURCES_DIR}/main.cpp
	${PROJECT_SOURCES_DIR}/MHelpFuncs.cpp
	${PROJECT_SOURCES_DIR}/MIhmMainLogic.cpp
	${PROJECT_SOURCES_DIR}/MIhmAniThread.cpp
	${PROJECT_SOURCES_DIR}/MIhmAniInitData.cpp
	${PROJECT_SOURCES_DIR}/MIhmSrv.cpp
	${PROJECT_SOURCES_DIR}/MIhmMsg.cpp
	${PROJECT_SOURCES_DIR}/MFormMain.cpp
	${PROJECT_SOURCES_DIR}/MFormSplash.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigImages.cpp
	${PROJECT_SOURCES_DIR}/MIhmTCLInterfaces.cpp
	${PROJECT_SOURCES_DIR}/MIhmVirtualObject.cpp
	${PROJECT_SOURCES_DIR}/MIhmAniVirtObjects.cpp
	${PROJECT_SOURCES_DIR}/MIhmLanguages.cpp
	${PROJECT_SOURCES_DIR}/MIhmDesktopIntf.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigGeneral.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigWeb.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigColor.cpp
	${PROJECT_SOURCES_DIR}/MIhmLaneTypeSettings.cpp
	${PROJECT_SOURCES_DIR}/MIhmDskVisibleObject.cpp
	${PROJECT_SOURCES_DIR}/MDigitalClock.cpp
	${PROJECT_SOURCES_DIR}/MIhmKey.cpp
	${PROJECT_SOURCES_DIR}/MStringDetection.cpp
	${PROJECT_SOURCES_DIR}/MStrDetectionPair.cpp
	${PROJECT_SOURCES_DIR}/MClientSocket.cpp
	${PROJECT_SOURCES_DIR}/MIhmWebInterface.cpp
	${PROJECT_SOURCES_DIR}/MIhmHttpServer.cpp
	${PROJECT_SOURCES_DIR}/MIhmHttpRequest.cpp
	${PROJECT_SOURCES_DIR}/MIhmHttpResponse.cpp
	${PROJECT_SOURCES_DIR}/MIhmWebSession.cpp
	${PROJECT_SOURCES_DIR}/MIhmWebVisibleObject.cpp
	${PROJECT_SOURCES_DIR}/MTracer.cpp
	${PROJECT_SOURCES_DIR}/MInputDialogReq.cpp
	${PROJECT_SOURCES_DIR}/MInputDialogExReq.cpp
	${PROJECT_SOURCES_DIR}/MInputExField.cpp
	${PROJECT_SOURCES_DIR}/MFormInput.cpp
	${PROJECT_SOURCES_DIR}/MFormInputEx.cpp
	${PROJECT_SOURCES_DIR}/MInputExFieldObj.cpp
	${PROJECT_SOURCES_DIR}/MFormLogin.cpp
	${PROJECT_SOURCES_DIR}/MCtlList.cpp
	${PROJECT_SOURCES_DIR}/MCtlEdit.cpp
	${PROJECT_SOURCES_DIR}/MLineEdit.cpp
	${PROJECT_SOURCES_DIR}/MIhmWDThread.cpp
	${PROJECT_SOURCES_DIR}/MTableModelView.cpp
	${PROJECT_SOURCES_DIR}/MTableView.cpp
	${PROJECT_SOURCES_DIR}/MTableModel.cpp
	${PROJECT_SOURCES_DIR}/MLibToggleKeys.cpp
	${PROJECT_SOURCES_DIR}/MElideLabel.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigActions.cpp
	${PROJECT_SOURCES_DIR}/MIhmAction.cpp
	${PROJECT_SOURCES_DIR}/MFormAbout.cpp
	${PROJECT_SOURCES_DIR}/MIhmSessionUserData.cpp
	${PROJECT_SOURCES_DIR}/MButtonLabel.cpp
	${PROJECT_SOURCES_DIR}/MTabView.cpp
	${PROJECT_SOURCES_DIR}/MIhmAuthAutomate.cpp
	${PROJECT_SOURCES_DIR}/mautomat.cpp
	${PROJECT_SOURCES_DIR}/MFormTakeOverDlg.cpp
	${PROJECT_SOURCES_DIR}/MHMenuView.cpp
	${PROJECT_SOURCES_DIR}/MHMenuFileLoader.cpp
	${PROJECT_SOURCES_DIR}/MMenuModelItem.cpp
	#${PROJECT_SOURCES_DIR}/MIhmTaskBar.cpp
	#${PROJECT_SOURCES_DIR}/MIhmTaskBarButton.cpp
	#${PROJECT_SOURCES_DIR}/MIhmTaskBarDlg.cpp
	#${PROJECT_SOURCES_DIR}/shdocvw.cpp
	${PROJECT_SOURCES_DIR}/MKeyMapperWin.cpp
	${PROJECT_SOURCES_DIR}/MInputAsyncExReq.cpp
	${PROJECT_SOURCES_DIR}/MDskAsyncInputs.cpp
	${PROJECT_SOURCES_DIR}/MStringDetectionAtm.cpp
	${PROJECT_SOURCES_DIR}/MPoliceValue.cpp
	${PROJECT_SOURCES_DIR}/MFormClientDlg.cpp
	${PROJECT_SOURCES_DIR}/MHtmlCodec.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigCursors.cpp
	${PROJECT_SOURCES_DIR}/MListWidget.cpp
	${PROJECT_SOURCES_DIR}/MIhmConfigDynamic.cpp
	${PROJECT_SOURCES_DIR}/MFormFileOpen.cpp
	${PROJECT_SOURCES_DIR}/MFormInputGen.cpp
	${PROJECT_SOURCES_DIR}/MClock.cpp
	${PROJECT_SOURCES_DIR}/MIhmComboBox.cpp
	${PROJECT_SOURCES_DIR}/MIhmComboBox.cpp
	${PROJECT_SOURCES_DIR}/MInputKbdInterface.cpp
	${PROJECT_SOURCES_DIR}/MInputExFiledsKbdInterface.cpp
	${PROJECT_SOURCES_DIR}/MInputExReq.cpp
	${PROJECT_SOURCES_DIR}/MIhmListWidget.cpp
	#${PROJECT_SOURCES_DIR}/MRVideoView.cpp
	#${PROJECT_SOURCES_DIR}/Cs_MJPG_Ctrl.cpp
	#${PROJECT_SOURCES_DIR}/MRVideoCmdThread.cpp
	#${PROJECT_SOURCES_DIR}/MRVideoMsg.cpp
	#${PROJECT_SOURCES_DIR}/MRVideoCmdAutomate.cpp
	#${PROJECT_SOURCES_DIR}/MRVideoControlObject.cpp
	${PROJECT_SOURCES_DIR}/MIhmMsgVideo.cpp
	${PROJECT_SOURCES_DIR}/MTableModelRow.cpp
	${PROJECT_SOURCES_DIR}/MRowsCommand.cpp
	${PROJECT_SOURCES_DIR}/MIhmVirtObjectsModel.cpp
	${PROJECT_SOURCES_DIR}/MIhmWebCommonObjLogic.cpp
	${PROJECT_SOURCES_DIR}/MWebVisibleObjSessionLink.cpp
	${PROJECT_SOURCES_DIR}/MFormTestKeyboard.cpp
	${PROJECT_SOURCES_DIR}/MParamList.cpp
	${PROJECT_SOURCES_DIR}/MIhmMiniWebSession.cpp
	${PROJECT_SOURCES_DIR}/MIhmVirtKyb.cpp
	${PROJECT_SOURCES_DIR}/MIhmStatusSetCfg.cpp
	${PROJECT_SOURCES_DIR}/MFormCustomPluginDlg.cpp
	${PROJECT_SOURCES_DIR}/MIhmLoadedPlugins.cpp

	${CMAKE_CURRENT_BINARY_DIR}/Resources.rc
)

set(PROJECT_HEADERS
	${PROJECT_SOURCES_DIR}/ihm_loc.h
	${PROJECT_SOURCES_DIR}/MHelpFuncs.h
	${PROJECT_SOURCES_DIR}/MIhmMainLogic.h
	${PROJECT_SOURCES_DIR}/MIhmAniThread.h
	${PROJECT_SOURCES_DIR}/MIhmAniInitData.h
	${PROJECT_SOURCES_DIR}/MIhmSrv.h
	${PROJECT_SOURCES_DIR}/MIhmMsg.h
	${PROJECT_SOURCES_DIR}/MFormMain.h
	${PROJECT_SOURCES_DIR}/MFormSplash.h
	${PROJECT_SOURCES_DIR}/MIhmConfigImages.h
	${PROJECT_SOURCES_DIR}/MIhmTCLInterfaces.h
	${PROJECT_SOURCES_DIR}/MIhmVirtualObject.h
	${PROJECT_SOURCES_DIR}/MIhmAniVirtObjects.h
	${PROJECT_SOURCES_DIR}/MIhmLanguages.h
	${PROJECT_SOURCES_DIR}/MIhmDesktopIntf.h
	${PROJECT_SOURCES_DIR}/MIhmConfigGeneral.h
	${PROJECT_SOURCES_DIR}/MIhmConfigWeb.h
	${PROJECT_SOURCES_DIR}/MIhmConfigColor.h
	${PROJECT_SOURCES_DIR}/MIhmLaneTypeSettings.h
	${PROJECT_SOURCES_DIR}/MIhmDskVisibleObject.h
	${PROJECT_SOURCES_DIR}/MDigitalClock.h
	${PROJECT_SOURCES_DIR}/MIhmKey.h
	${PROJECT_SOURCES_DIR}/MStringDetection.h
	${PROJECT_SOURCES_DIR}/MStrDetectionPair.h
	${PROJECT_SOURCES_DIR}/MClientSocket.h
	${PROJECT_SOURCES_DIR}/MIhmWebInterface.h
	${PROJECT_SOURCES_DIR}/MIhmHttpServer.h
	${PROJECT_SOURCES_DIR}/MIhmHttpRequest.h
	${PROJECT_SOURCES_DIR}/MIhmHttpResponse.h
	${PROJECT_SOURCES_DIR}/MIhmWebSession.h
	${PROJECT_SOURCES_DIR}/MIhmWebVisibleObject.h
	${PROJECT_SOURCES_DIR}/MTracer.h
	${PROJECT_SOURCES_DIR}/MInputDialogReq.h
	${PROJECT_SOURCES_DIR}/MInputDialogExReq.h
	${PROJECT_SOURCES_DIR}/MInputExField.h
	${PROJECT_SOURCES_DIR}/MFormInput.h
	${PROJECT_SOURCES_DIR}/MFormInputEx.h
	${PROJECT_SOURCES_DIR}/MInputExFieldObj.h
	${PROJECT_SOURCES_DIR}/MFormLogin.h
	${PROJECT_SOURCES_DIR}/MCtlList.h
	${PROJECT_SOURCES_DIR}/MCtlEdit.h
	${PROJECT_SOURCES_DIR}/MLineEdit.h
	${PROJECT_SOURCES_DIR}/MIhmWDThread.h
	${PROJECT_SOURCES_DIR}/MTableModelView.h
	${PROJECT_SOURCES_DIR}/MTableView.h
	${PROJECT_SOURCES_DIR}/MTableModel.h
	${PROJECT_SOURCES_DIR}/MLibToggleKeys.h
	${PROJECT_SOURCES_DIR}/MElideLabel.h
	${PROJECT_SOURCES_DIR}/MIhmConfigActions.h
	${PROJECT_SOURCES_DIR}/MIhmAction.h
	${PROJECT_SOURCES_DIR}/MFormAbout.h
	${PROJECT_SOURCES_DIR}/MIhmSessionUserData.h
	${PROJECT_SOURCES_DIR}/MButtonLabel.h
	${PROJECT_SOURCES_DIR}/MTabView.h
	${PROJECT_SOURCES_DIR}/mautomat.h
	${PROJECT_SOURCES_DIR}/mautomatfunctor.h
	${PROJECT_SOURCES_DIR}/MIhmAuthAutomate.h
	${PROJECT_SOURCES_DIR}/MFormTakeOverDlg.h
	${PROJECT_SOURCES_DIR}/MHMenuView.h
	${PROJECT_SOURCES_DIR}/MHMenuFileLoader.h
	${PROJECT_SOURCES_DIR}/MMenuModelItem.h
	#${PROJECT_SOURCES_DIR}/MIhmTaskBar.h
	#${PROJECT_SOURCES_DIR}/MIhmTaskBarButton.h
	#${PROJECT_SOURCES_DIR}/MIhmTaskBarDlg.h
	#${PROJECT_SOURCES_DIR}/shdocvw.h
	${PROJECT_SOURCES_DIR}/MKeyMapperWin.h
	${PROJECT_SOURCES_DIR}/MInputAsyncExReq.h
	${PROJECT_SOURCES_DIR}/MDskAsyncInputs.h
	${PROJECT_SOURCES_DIR}/MStringDetectionAtm.h
	${PROJECT_SOURCES_DIR}/MPoliceValue.h
	${PROJECT_SOURCES_DIR}/MFormClientDlg.h
	${PROJECT_SOURCES_DIR}/MHtmlCodec.h
	${PROJECT_SOURCES_DIR}/MIhmConfigCursors.h
	${PROJECT_SOURCES_DIR}/MListWidget.h
	${PROJECT_SOURCES_DIR}/MIhmConfigDynamic.h
	${PROJECT_SOURCES_DIR}/MFormFileOpen.h
	${PROJECT_SOURCES_DIR}/MFormInputGen.h
	${PROJECT_SOURCES_DIR}/MClock.h
	${PROJECT_SOURCES_DIR}/MIhmComboBox.h
	${PROJECT_SOURCES_DIR}/MInputKbdInterface.h
	${PROJECT_SOURCES_DIR}/MInputExFiledsKbdInterface.h
	${PROJECT_SOURCES_DIR}/MInputExReq.h
	${PROJECT_SOURCES_DIR}/MIhmListWidget.h
	#${PROJECT_SOURCES_DIR}/MRVideoView.h 
	#${PROJECT_SOURCES_DIR}/Cs_MJPG_Ctrl.h
	#${PROJECT_SOURCES_DIR}/MRVideoCmdThread.h
	#${PROJECT_SOURCES_DIR}/MRVideoMsg.h
	#${PROJECT_SOURCES_DIR}/MRVideoCmdAutomate.h
	#${PROJECT_SOURCES_DIR}/MRVideoControlObject.h
	${PROJECT_SOURCES_DIR}/MIhmMsgVideo.h
	${PROJECT_SOURCES_DIR}/MTableModelRow.h
	${PROJECT_SOURCES_DIR}/MRowsCommand.h
	${PROJECT_SOURCES_DIR}/MIhmVirtObjectsModel.h
	${PROJECT_SOURCES_DIR}/MIhmWebCommonObjLogic.h
	${PROJECT_SOURCES_DIR}/MWebVisibleObjSessionLink.h
	${PROJECT_SOURCES_DIR}/MFormTestKeyboard.h
	${PROJECT_SOURCES_DIR}/MParamList.h
	${PROJECT_SOURCES_DIR}/MIhmMiniWebSession.h
	${PROJECT_SOURCES_DIR}/MIhmVirtKyb.h
	${PROJECT_SOURCES_DIR}/MIhmStatusSetCfg.h
	${PROJECT_SOURCES_DIR}/MFormCustomPluginDlg.h
	${PROJECT_SOURCES_DIR}/MIhmLoadedPlugins.h
)

qt_add_executable(${PROJECT_EXE} 
					${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_EXE} PRIVATE
							${PROJECT_SOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${PROJECT_RESOURCES_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/run/includes"	
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/virtualKYB/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/lwd/startup/includes"
)
 

##-----------------------------------------------------------------------------------------------

add_library(CSR::Reg SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Reg PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/release/csr_reg_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Reg/out/build/debug/csr_reg_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							

add_library(CSR::Trc SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Trc PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/release/csr_trc_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/Trc/out/build/debug/csr_trc_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
		
add_library(CSR::Noy SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Noy PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Voie_NT/Noy/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE	
					  	"${CMAKE_SOURCE_DIR}/../../Voie_NT/Noy/out/build/release/csr_noy_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG		
					  	"${CMAKE_SOURCE_DIR}/../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG	
					  	"${CMAKE_SOURCE_DIR}/../../Voie_NT/Noy/out/build/debug/csr_noy_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
	 						
							
add_library(CSR::Srv SHARED IMPORTED GLOBAL)
set_target_properties(CSR::Srv PROPERTIES
					  INTERFACE_INCLUDE_DIRECTORIES
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/includes/"
					  IMPORTED_IMPLIB_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.lib"
					  IMPORTED_LOCATION_RELEASE
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/release/csr_srv_VS17_x64_r.dll"
					  IMPORTED_IMPLIB_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.lib"
					  IMPORTED_LOCATION_DEBUG
					  	"${CMAKE_SOURCE_DIR}/../../Commun_NT/srv/out/build/debug/csr_srv_VS17_x64.dll"
					  MAP_IMPORTED_CONFIG_MINSIZEREL
					  	Release
					  MAP_IMPORTED_CONFIG_RELWITHDEBINFO
					  	Release
)							
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${PROJECT_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)



target_link_libraries(${PROJECT_EXE}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Network
		Qt::Xml
		Qt::UiTools
		Qt::Core5Compat
		Qt::AxContainer
		Qt::Qml
		CSR::Trc
	    CSR::Reg
	    CSR::Noy
	    CSR::Srv

)
 
add_subdirectory(tests)



set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


# #Copy all runtime files (DLLs) to build directory
#add_custom_command(
#    TARGET ${PROJECT_LIB} POST_BUILD
#    COMMAND ${CMAKE_COMMAND}
#        -E copy
#        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
#        $<TARGET_RUNTIME_DLLS:${PROJECT_LIB}>
#    COMMAND_EXPAND_LISTS
#)

add_custom_command(
    TARGET ${PROJECT_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} #$<TARGET_FILE_DIR:${PROJECT_LIB}>
        $<TARGET_FILE:${PROJECT_EXE}>
    COMMAND_EXPAND_LISTS
)


