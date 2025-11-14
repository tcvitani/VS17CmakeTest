DEPENDPATH += .
INCLUDEPATH += .

QT = core gui xml
TEMPLATE = app

RC_FILE += ./resources/version.rc

SOURCES +=  ./sources/main.cpp
SOURCES +=  ./sources/maindlg.cpp
HEADERS +=  ./sources/maindlg.h
SOURCES +=  ./sources/affichage.cpp
HEADERS +=  ./sources/affichage.h
SOURCES +=  ./sources/config.cpp
HEADERS +=  ./sources/config.h
SOURCES +=  ./sources/SimuSendReciveThread.cpp
HEADERS +=  ./sources/SimuSendReciveThread.h
SOURCES +=  ./sources/IHMSimuDld.cpp
HEADERS +=  ./sources/IhmSimuDld.h
HEADERS +=  ./sources/constants.h
SOURCES +=  ./sources/touchedlg.cpp
HEADERS +=  ./sources/touchedlg.h
SOURCES +=  ./sources/policedlg.cpp
HEADERS +=  ./sources/policedlg.h
SOURCES +=  ./sources/saisiedlg.cpp
HEADERS +=  ./sources/saisiedlg.h
SOURCES +=  ./sources/detectiondlg.cpp
HEADERS +=  ./sources/detectiondlg.h
SOURCES +=  ./sources/videodlg.cpp
HEADERS +=  ./sources/videodlg.h
SOURCES +=  ./sources/authorization.cpp
HEADERS +=  ./sources/authorization.h
SOURCES +=  ./sources/linkdlg.cpp
HEADERS +=  ./sources/linkdlg.h
SOURCES +=  ./sources/saisieexdlg.cpp
HEADERS +=  ./sources/saisieexdlg.h
SOURCES +=  ./sources/AsyncInputDlg.cpp
HEADERS +=  ./sources/AsyncInputDlg.h
HEADERS += ./resources/resource.h

SOURCES +=  ./sources/MWaitUserDlg.cpp
HEADERS +=  ./sources/MWaitUserDlg.h

INCLUDEPATH += ./resources
INCLUDEPATH += ./sources
INCLUDEPATH += ../../includes
INCLUDEPATH += L:/Commun_NT/declare/10.0.0
INCLUDEPATH += L:/Commun_NT/reg/10.0.0
INCLUDEPATH += L:/Voie_NT/commun/3.1.0
INCLUDEPATH += L:/Voie_NT/noy/10.0.0
INCLUDEPATH += L:/Voie_NT/run/10.0.0
INCLUDEPATH += L:/Commun_NT/srv/10.0.0


FORMS +=  ./ui/maindlg.ui
FORMS +=  ./ui/affichage.ui
FORMS +=  ./ui/config.ui
FORMS +=  ./ui/touchedlg.ui
FORMS +=  ./ui/policedlg.ui
FORMS +=  ./ui/saisiedlg.ui
FORMS +=  ./ui/detectiondlg.ui
FORMS +=  ./ui/videodlg.ui
FORMS +=  ./ui/authorization.ui
FORMS +=  ./ui/linkdlg.ui
FORMS +=  ./ui/saisieexdlg.ui
FORMS +=  ./ui/AsyncInputDlg.ui
FORMS +=  ./ui/waitUser.ui

RC_FILE += ./resources/version.rc

 CONFIG(debug, debug|release) {
	TARGET = CSR_IHM_QTSimulator11_VS12_x64
	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 
	QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"
 } else {
	TARGET = CSR_IHM_QTSimulator11_VS12_x64_R

	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64_r.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64_R.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 
	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }











