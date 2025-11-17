
DEPENDPATH += ./sources/

TEMPLATE = app
 QT = core gui network xml
  CONFIG += debug_and_release
 
SOURCES += ./sources/main.cpp
SOURCES += ./sources/maindlg.cpp

HEADERS += ./sources/maindlg.h 


FORMS += ./ui/maindlg.ui

INCLUDEPATH += .
INCLUDEPATH += ./sources/



CONFIG(debug, debug|release) {

	TARGET = WFindScanCode
	
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"
 } else {
	TARGET = WFindScanCode_R
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"

 }


