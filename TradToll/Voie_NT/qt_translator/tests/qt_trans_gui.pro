DEPENDPATH += .
DEPENDPATH += ../Includes
DEPENDPATH += ../Sources
INCLUDEPATH += .
INCLUDEPATH += ./Includes
INCLUDEPATH += ./Sources
INCLUDEPATH += ../Includes


SOURCES += ./Sources/main.cpp
SOURCES += ./Sources/FormMainTranslTest.cpp

HEADERS += ./Includes/FormMainTranslTest.h

#HEADERS += ./Resources/resource.h
#RC_FILE += ./Resources/version.rc

FORMS	+= ./UI/FormMainTranslTest.ui

QT = core gui xml 

TEMPLATE = app

CONFIG += qt thread 
CONFIG += debug_and_release
 

CONFIG(debug, debug|release) {
	TARGET = csr_QtTransGui_vs12_x64
	
	LIBS += ../debug/csr_qt_translator_vs12_x64.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib

	QMAKE_POST_LINK = "xcopy .\\debug\\*.exe c:\\csr\\exe /f/y"
} else {
	TARGET = csr_QtTransGui_vs12_x64
	
	LIBS += ../release/csr_qt_translator_vs12_x64_r.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }


