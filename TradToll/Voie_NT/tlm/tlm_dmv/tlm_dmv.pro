DEPENDPATH += .
DEPENDPATH += ./Includes/
DEPENDPATH += ./Sources/
INCLUDEPATH += ./Includes/
INCLUDEPATH += ./Sources/
INCLUDEPATH += ./Resources/

INCLUDEPATH += L:/Commun_NT/Declare/10.0.0
INCLUDEPATH += L:/Commun_NT/Srv/10.0.0
INCLUDEPATH += L:/Voie_NT/Commun/3.3.0
INCLUDEPATH += L:/Commun_NT/Reg/10.0.3
INCLUDEPATH += L:/Voie_NT/Noy/10.0.4
INCLUDEPATH += L:/Voie_NT/Run/10.0.2
INCLUDEPATH += L:/Commun_NT/Trc/10.0.4
INCLUDEPATH += L:/Voie_NT/TLM/10.0.x
INCLUDEPATH += L:/Voie_NT/dmv_oblak_protocol/10.0.2

QMAKE_CXXFLAGS_WARN_ON -= -w34100
		
QT = core network xml

TEMPLATE = lib
RC_FILE = ./Resources/Resource.rc

SOURCES += ./Sources/mbx2slot.c
SOURCES += ./Sources/TlmDmvAni.c
SOURCES += ./Sources/TlmDmvInit.c
SOURCES += ./Sources/TlmDmvServ.c
SOURCES += ./Sources/TlmDmvUtil.c
SOURCES += ./Sources/TlmDmvWorker.cpp
SOURCES += ./Sources/TlmDmvAutomate.cpp
SOURCES += ./Sources/Automat.cpp
SOURCES += ./Sources/UdpClient.cpp
SOURCES += ./Sources/HelpFuncs.cpp
SOURCES += ./Sources/InstTracer.cpp
SOURCES += ./Sources/MQEventLoop.cpp
SOURCES += ./Sources/MQApplication.cpp

HEADERS += ./Includes/mbx2slot.h
HEADERS += ./Includes/tlm_dmv.h
HEADERS += ./Includes/TlmDmvAni.h
HEADERS += ./Includes/TlmDmvGlobal.h
HEADERS += ./Includes/TlmDmvServ.h
HEADERS += ./Includes/TlmDmvUtil.h
HEADERS += ./Includes/TlmDmvAutomate.h
HEADERS += ./Includes/TlmDmvWorker.h
HEADERS += ./Includes/Automat.h
HEADERS += ./Includes/AutomatFunctor.h
HEADERS += ./Includes/HelpFuncs.h
HEADERS += ./Includes/UdpClient.h
HEADERS += ./Includes/InstTracer.h
HEADERS += ./Includes/MQEventLoop.h
HEADERS += ./Includes/MQApplication.h

DEFINES -= UNICODE

DEFINES += CSR_TLM_EXPORTS

CONFIG += qt thread
CONFIG += debug_and_release
 
CONFIG(debug, debug|release) {

	TARGET = csr_tlm10_dmv_vs12_x64
	
	LIBS += L:\\Commun_NT\\REG\\10.0.3\\CSR_REG10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64.lib
	LIBS += L:\\Voie_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64.lib
	LIBS += L:\\Voie_NT\\dmv_oblak_protocol\\10.0.2\\dmv_oblak_protocol_vs12_x64.lib
	LIBS += kernel32.lib user32.lib advapi32.lib shell32.lib imagehlp.lib
	QMAKE_POST_LINK = "xcopy .\\Debug\\*.dll c:\\csr\\exe /f/y"	
 } else {
	TARGET = csr_tlm10_dmv_vs12_x64_r
	
	LIBS += L:\\Commun_NT\\REG\\10.0.3\\CSR_REG10_VS12_x64_r.lib
	LIBS += L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64_r.lib
	LIBS += L:\\Commun_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\dmv_oblak_protocol\\10.0.2\\dmv_oblak_protocol_vs12_x64_r.lib
	LIBS += kernel32.lib user32.lib advapi32.lib shell32.lib imagehlp.lib
	QMAKE_POST_LINK = "xcopy .\\Release\\*.dll c:\\csr\\exe /f/y"
 }