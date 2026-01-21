DEPENDPATH += .
DEPENDPATH += ./Includes
DEPENDPATH += ./Sources
INCLUDEPATH += .
INCLUDEPATH += ./Includes
INCLUDEPATH += ./Sources

SOURCES += ./Sources/MTranslator.cpp
SOURCES += ./Sources/TranslatorImpl.cpp
SOURCES += ./Sources/LanguageList.cpp

HEADERS += ./Includes/MTranslator.h
HEADERS+= ./Sources/TranslatorImpl.h
HEADERS+= ./Sources/LanguageList.h

HEADERS += ./Resources/resource.h
RC_FILE += ./Resources/version.rc

QT = core xml 

TEMPLATE = lib

CONFIG += qt
CONFIG += debug_and_release

DEFINES += MTRANS_DLLEXPORT

	       
CONFIG(debug, debug|release) {
	TARGET = csr_qt_translator_vs12_x64
	
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\debug\\*.dll c:\\csr\\exe /f/y"
} 
else {
	TARGET = csr_qt_translator_vs12_x64_r
	
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.dll c:\\csr\\exe /f/y"
 }


