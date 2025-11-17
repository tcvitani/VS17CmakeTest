DEPENDPATH += .
DEPENDPATH += ./includes/
DEPENDPATH += ./sources/
INCLUDEPATH += .
INCLUDEPATH += ./includes/
INCLUDEPATH += ./sources/

SOURCES += ./sources/main.cpp
SOURCES += ./sources/MTracer.cpp
SOURCES += ./sources/MKybWDThread.cpp
SOURCES += ./sources/MKybConfigGeneral.cpp
SOURCES += ./sources/MKybMainLogic.cpp
SOURCES += ./sources/MKybFormMain.cpp
SOURCES += ./sources/MKybAniInitData.cpp
SOURCES += ./sources/MKybAniThread.cpp
SOURCES += ./sources/MKybSrv.cpp
SOURCES += ./sources/MKybMsg.cpp
SOURCES += ./sources/MKybVisibleObject.cpp
SOURCES += ./sources/MKybTypeSettings.cpp

HEADERS += ./includes/virt_kyb.h
HEADERS += ./sources/MTracer.h
HEADERS += ./sources/MKybWDThread.h
HEADERS += ./sources/MKybConfigGeneral.h
HEADERS += ./sources/MKybMainLogic.h
HEADERS += ./sources/MKybFormMain.h
HEADERS += ./sources/MKybAniInitData.h
HEADERS += ./sources/MKybAniThread.h
HEADERS += ./sources/MKybSrv.h
HEADERS += ./sources/MKybMsg.h
HEADERS += ./sources/MKybVisibleObject.h
HEADERS += ./sources/MKybTypeSettings.h

HEADERS += ./resources/resource.h

RC_FILE += ./resources/virtualKB.rc

QT = core gui
TEMPLATE = app

CONFIG += qt thread uitools
CONFIG += debug_and_release

INCLUDEPATH += .
INCLUDEPATH += ./includes/
INCLUDEPATH += L:/Commun_NT/declare/10.0.0
INCLUDEPATH += L:/Commun_NT/reg/10.0.0
INCLUDEPATH += L:/Voie_NT/commun/3.1.0
INCLUDEPATH += L:/Voie_NT/lwd/10.0.0
INCLUDEPATH += L:/Voie_NT/noy/10.0.0
INCLUDEPATH += L:/Voie_NT/run/10.0.0
INCLUDEPATH += L:/Commun_NT/srv/10.0.0

	
CONFIG(debug, debug|release) {
	TARGET = CSR_VIRT_KYB10_VS12_x64
	
	LIBS += L:\\Commun_NT\\reg\\10.0.0\\csr_reg10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"
 } else {
	TARGET = CSR_VIRT_KYB10_VS12_x64_r
	
	LIBS += L:\\Commun_NT\\reg\\10.0.0\\csr_reg10_VS12_x64_R.lib
	LIBS += L:\\Commun_NT\\srv\\10.0.0\\csr_srv10_VS12_x64_r.lib 
	LIBS += L:\\Voie_NT\\noy\\10.0.0\\csr_noy10_VS12_x64_R.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }


