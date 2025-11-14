
DEPENDPATH += .
DEPENDPATH += sources/
DEPENDPATH += includes/

INCLUDEPATH += .
INCLUDEPATH += sources/
INCLUDEPATH += includes/

TARGET = CSR_ihmweb-configure_VS12_x64

QT = core

TEMPLATE = app

SOURCES += sources/main.cpp
SOURCES += sources/param_handler.cpp
SOURCES += sources/template_handler.cpp

HEADERS += includes/param_handler.h
HEADERS += includes/template_handler.h
HEADERS += resources/resource.h

RC_FILE += resources/version.rc

CONFIG += console

QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"