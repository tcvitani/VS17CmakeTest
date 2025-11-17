DEPENDPATH += ../../includes/
DEPENDPATH += ../../sources/
DEPENDPATH += ./sources/

TEMPLATE = app
 QT = core gui network xml script opengl
 CONFIG += qt thread uitools  
 CONFIG += debug_and_release
 
SOURCES += ./sources/main.cpp
SOURCES += ./sources/maindlg.cpp
SOURCES += ./sources/SimuSendReciveThread.cpp

HEADERS += ./sources/maindlg.h 
HEADERS += ./sources/SimuSendReciveThread.h


FORMS += ./ui/maindlg.ui

INCLUDEPATH += .
INCLUDEPATH += ./sources/
INCLUDEPATH += ../../includes/
INCLUDEPATH += L:/Commun_NT/declare/10.0.0
INCLUDEPATH += L:/Commun_NT/reg/10.0.0
INCLUDEPATH += L:/Voie_NT/commun/3.1.0
INCLUDEPATH += L:/Voie_NT/lwd/10.0.0
INCLUDEPATH += L:/Voie_NT/noy/10.0.0
INCLUDEPATH += L:/Voie_NT/run/10.0.0
INCLUDEPATH += L:/Commun_NT/srv/10.0.0

CONFIG(debug, debug|release) {

	TARGET = TEST_VIRT_KYB10_VS12_x64
	
	LIBS += L:\\Commun_NT\\reg\\10.0.0\\csr_reg10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"
 } else {
	TARGET = TEST_VIRT_KYB10_VS12_x64_R
	LIBS += L:\\Commun_NT\\reg\\10.0.0\\csr_reg10_VS12_x64_R.lib
	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64_r.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64_R.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }


