DEPENDPATH += ./includes/
DEPENDPATH += ./sources/
INCLUDEPATH += ./includes/
INCLUDEPATH += ./sources/
QT = core network gui xml

TEMPLATE = lib
RC_FILE = ./resources/CommonToolsClasses.rc
RESOURCES = ./Resources/ImgResources.qrc

SOURCES += ./sources/MConfig.cpp
SOURCES += ./sources/MHelpFuncs.cpp
SOURCES += ./sources/MLangPickConfig.cpp
SOURCES += ./sources/MStyledLangPick.cpp
SOURCES += ./sources/MStyledTitleBar.cpp
SOURCES += ./sources/MLangPickConfig.cpp
SOURCES += ./sources/MStyledToolBox.cpp
SOURCES += ./sources/MTracer.cpp
SOURCES += ./Sources/MAboutWidget.cpp

HEADERS += ./includes/MConfig.h
HEADERS += ./includes/MHelpFuncs.h
HEADERS += ./includes/MLangPickConfig.h
HEADERS += ./includes/MStyledLangPick.h
HEADERS += ./includes/MStyledTitleBar.h
HEADERS += ./includes/MStyledToolBox.h
HEADERS += ./includes/MTracer.h
HEADERS += ./Includes/MDefines.h
HEADERS += ./Includes/MAboutWidget.h

FORMS	+= ./UI/MAboutWidgetB.ui

INCLUDEPATH += L:\\Voie_NT\\qt_translator\\10.0.0

DEFINES += COMMON_TOOLS_CLASSES_EXPORT

UI_DIR = ./Includes

CONFIG += debug_and_release

CONFIG(debug, debug|release) {

	TARGET = csr_common_tools_classes_vs12_x64
	
	LIBS += L:\\Voie_NT\\qt_translator\\10.0.0\\csr_qt_translator_vs12_x64.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib
	LIBS += version.lib

	QMAKE_POST_LINK = "xcopy .\\debug\\*.dll c:\\csrTestTools\\exe /f/y"
 } else {
	TARGET = csr_common_tools_classes_vs12_x64_r
	
	LIBS += L:\\Voie_NT\\qt_translator\\10.0.0\\csr_qt_translator_vs12_x64_r.lib
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib
	LIBS += version.lib

	QMAKE_POST_LINK = "xcopy .\\release\\*.dll c:\\csrTestTools\\exe /f/y"

 }

