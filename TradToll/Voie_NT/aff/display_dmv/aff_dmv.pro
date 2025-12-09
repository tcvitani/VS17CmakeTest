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
INCLUDEPATH += L:/Voie_NT/Aff/11.0.x
INCLUDEPATH += L:/Voie_nt/AFF/_aff_util/11.0.1
INCLUDEPATH += L:/Voie_NT/fic_gere/10.1.3
INCLUDEPATH += L:/Voie_NT/fonct_c/11.0.2
INCLUDEPATH += L:/Voie_NT/TLM/10.1.x
INCLUDEPATH += L:/Voie_NT/TLM/tlm_dmv/10.0.0
INCLUDEPATH += L:/Voie_NT/dmv_oblak_protocol/10.1.0

QMAKE_CXXFLAGS_WARN_ON -= -w34100
		
QT = core network xml

TEMPLATE = lib
QMAKE_PRE_LINK = rc ./Resources/Resource.rc
RES_FILE = ./Resources/Resource.res

#RC_FILE += ./Resources/Resource.rc

SOURCES += ./Sources/mbx2slot.c
SOURCES += ./Sources/DmvAni.c
SOURCES += ./Sources/DmvInit.c
SOURCES += ./Sources/AffService.c
SOURCES += ./Sources/AffMessage.c
SOURCES += ./Sources/TlmService.c
SOURCES += ./Sources/TlmMessage.c
SOURCES += ./Sources/DmvWorker.cpp
SOURCES += ./Sources/DmvAutomate.cpp
SOURCES += ./Sources/Automat.cpp
SOURCES += ./Sources/UdpClient.cpp
SOURCES += ./Sources/HelpFuncs.cpp
SOURCES += ./Sources/InstTracer.cpp
SOURCES += ./Sources/MQEventLoop.cpp
SOURCES += ./Sources/MQApplication.cpp

HEADERS += ./Includes/mbx2slot.h
HEADERS += ./Includes/DmvAni.h
HEADERS += ./Includes/DmvGlobal.h
HEADERS += ./Includes/AffService.h
HEADERS += ./Includes/AffMessage.h
HEADERS += ./Includes/TlmService.h
HEADERS += ./Includes/TlmMessage.h
HEADERS += ./Includes/DmvWorker.h
HEADERS += ./Includes/DmvAutomate.h
HEADERS += ./Includes/Automat.h
HEADERS += ./Includes/AutomatFunctor.h
HEADERS += ./Includes/HelpFuncs.h
HEADERS += ./Includes/UdpClient.h
HEADERS += ./Includes/InstTracer.h
HEADERS += ./Includes/MQEventLoop.h
HEADERS += ./Includes/MQApplication.h

DEFINES -= UNICODE

DEFINES += AFF_DEF

CONFIG += qt thread
CONFIG += debug_and_release
 
CONFIG(debug, debug|release) {

	TARGET = csr_aff11_dmv_vs12_x64
	
	LIBS += L:\\Commun_NT\\REG\\10.0.3\\CSR_REG10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64.lib
	LIBS += L:\\Commun_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64.lib
	LIBS += L:\\Voie_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64.lib
	LIBS += L:\\Voie_NT\\fonct_c\\11.0.2\\csr_Fonct_c11_VS12_x64.lib
	LIBS += L:\\Voie_NT\\fic_gere\\10.1.3\\csr_Fic_gere10_VS12_x64.lib
	LIBS += L:\\Voie_NT\\msg_fic\\11.1.0\\CSR_MSG_FIC11_VS12_x64.lib
	LIBS += L:\\Voie_NT\\Aff\\_aff_util\\11.0.1\\CSR_AFF_UTIL11_VS12_x64.lib
	LIBS += L:\\Voie_NT\\dmv_oblak_protocol\\10.1.0\\dmv_oblak_protocol_vs12_x64.lib
	LIBS += kernel32.lib user32.lib advapi32.lib shell32.lib imagehlp.lib
	QMAKE_POST_LINK = "xcopy .\\Debug\\*.dll c:\\csr\\exe /f/y"	
 } else {
	TARGET = csr_aff11_dmv_vs12_x64_r
	
	LIBS += L:\\Commun_NT\\REG\\10.0.3\\CSR_REG10_VS12_x64_r.lib
	LIBS += L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64_r.lib
	LIBS += L:\\Commun_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\fonct_c\\11.0.2\\csr_Fonct_c11_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\fic_gere\\10.1.3\\csr_Fic_gere10_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\msg_fic\\11.1.0\\CSR_MSG_FIC11_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\Aff\\_aff_util\\11.0.1\\CSR_AFF_UTIL11_VS12_x64_r.lib
	LIBS += L:\\Voie_NT\\dmv_oblak_protocol\\10.1.0\\dmv_oblak_protocol_vs12_x64_r.lib
	LIBS += kernel32.lib user32.lib advapi32.lib shell32.lib imagehlp.lib
	QMAKE_POST_LINK = "xcopy .\\Release\\*.dll c:\\csr\\exe /f/y"
 }