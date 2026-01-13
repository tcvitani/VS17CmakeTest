DEPENDPATH += .
DEPENDPATH += ./Includes
DEPENDPATH += ./Sources
INCLUDEPATH += .
INCLUDEPATH += ./Includes
INCLUDEPATH += ./Sources

SOURCES += ./Sources/main.cpp
SOURCES += ./Sources/MMainDialog.cpp
SOURCES += ./Sources/MCmdACOM.cpp
SOURCES += ./Sources/MTracer.cpp
SOURCES += ./Sources/MHelpFuncs.cpp


HEADERS += ./Includes/MMainDialog.h
HEADERS += ./Includes/MDefines.h
HEADERS += ./Includes/MHelpFuncs.h
HEADERS += ./Includes/MTracer.h
HEADERS += ./Includes/MGlobalStructs.h
HEADERS += ./Includes/MCmdACOM.h



HEADERS += ./Resources/resource.h
RC_FILE += ./Resources/version.rc

FORMS	+= ./UI/MMainDialogB.ui


QT = core gui xml sql

TEMPLATE = app

CONFIG += qt thread 
CONFIG += debug_and_release

INCLUDEPATH += L:\\Commun_NT\\declare\\10.0.0
INCLUDEPATH += L:\\Voie_NT\\Commun\\3.3.0
INCLUDEPATH += L:\\Commun_NT\\Reg\\10.0.3
INCLUDEPATH += L:\\Commun_NT\\Srv\\10.0.0
INCLUDEPATH += L:\\Commun_NT\\acom\\11.0.1
INCLUDEPATH += n:\\Commun_nt\\Europe\\Pl\\Modules\\Msg_LC_Auth_VT\\11.1.0
INCLUDEPATH += l:\\commun_nt\\msg\\10.0.2
INCLUDEPATH += l:\\commun_nt\\msg_lc\\11.0.0
INCLUDEPATH += l:\\ls_nt\\msg_sv\\10.0.1
INCLUDEPATH += l:\\commun_nt\\list\\10.0.0

CONFIG(debug, debug|release) {
	TARGET = AuthSvc_VT_test_vs12_x64

	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib
	LIBS += L:\\Commun_NT\\Reg\\10.0.3\\csr_reg10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\acom\\11.0.1\\CSR_ACOM11_VS12_x64.lib
	LIBS += n:\\Commun_nt\\Europe\\Pl\\Modules\\Msg_LC_Auth_VT\\11.1.0\\CSR_MSGLC11_AUTH_VT_VS12_x64.lib
	LIBS += L:\\COMMUN_NT\\MSG\\10.0.2\\CSR_MSG10_VS12_x64.LIB
	LIBS += L:\\LS_NT\\MSG_SV\\10.0.1\\CSR_MSGSV10_VS12_x64.LIB
	LIBS += version.lib

	
	QMAKE_POST_LINK = "xcopy .\\debug\\*.exe c:\\csr\\exe /f/y"
} 
else {
	TARGET = AuthSvc_VT_test_vs12_x64_r
	
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib
	LIBS += L:\\Commun_NT\\Reg\\10.0.3\\csr_reg10_VS12_x64_r.lib
	LIBS += L:\\Commun_NT\\acom\\11.0.1\\CSR_ACOM11_VS12_x64_r.lib
	LIBS += n:\\Commun_nt\\Europe\\Pl\\Modules\\Msg_LC_Auth_VT\\11.1.0\\CSR_MSGLC11_AUTH_VT_VS12_x64_r.lib
	LIBS += L:\\COMMUN_NT\\MSG\\10.0.2\\CSR_MSG10_VS12_x64_r.LIB
	LIBS += L:\\LS_NT\\MSG_SV\\10.0.1\\CSR_MSGSV10_VS12_x64_r.LIB
	LIBS += version.lib

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }


