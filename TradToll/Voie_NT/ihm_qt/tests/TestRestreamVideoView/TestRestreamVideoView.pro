DEPENDPATH += .
DEPENDPATH += ../../includes/
DEPENDPATH += ../../sources/
DEPENDPATH += ./sources/


TEMPLATE = app
 QT = core gui network xml script opengl
 CONFIG += qt thread uitools qaxcontainer 
 CONFIG += debug_and_release
 #TYPELIBS = $$system(dumpcpp {DE625294-70E6-45ED-B895-CFFA13AEB044} -o ./sources/AxAxis)
 #TYPELIBS = $$system(dumpcpp {592F8FCA-57A5-4FC7-8273-3FA504B64A10} -o ./sources/Cs_MJPG_Ctrl)
	
SOURCES += ./sources/main.cpp
SOURCES += ./sources/TestVideoDialog.cpp
SOURCES += ../../sources/MTracer.cpp
SOURCES += ../../sources/mautomat.cpp

SOURCES += ../../sources/MRVideoView.cpp
SOURCES += ../../sources/Cs_MJPG_Ctrl.cpp
SOURCES += ../../sources/MRVideoCmdThread.cpp
SOURCES += ../../sources/MRVideoMsg.cpp
SOURCES += ../../sources/MRVideoCmdAutomate.cpp
SOURCES += ../../sources/MRVideoControlObject.cpp

HEADERS += ./sources/TestVideoDialog.h 
HEADERS += ../../sources/MTracer.h 
HEADERS += ../../sources/mautomat.h
HEADERS += ../../sources/mautomatfunctor.h

HEADERS += ../../sources/MRVideoView.h 
HEADERS += ../../sources/Cs_MJPG_Ctrl.h
HEADERS += ../../sources/MRVideoCmdThread.h
HEADERS += ../../sources/MRVideoMsg.h
HEADERS += ../../sources/MRVideoCmdAutomate.h
HEADERS += ../../sources/MRVideoControlObject.h

FORMS += ./ui/TestVideoDialog.ui

INCLUDEPATH += .
INCLUDEPATH += ./sources/
INCLUDEPATH += ../../includes/
INCLUDEPATH += ../../sources/
INCLUDEPATH += L:/Voie_NT/commun/3.1.0
INCLUDEPATH += L:/Commun_NT/declare/10.0.0
INCLUDEPATH += L:/Voie_NT/run/10.0.0
INCLUDEPATH += L:/Voie_NT/noy/10.0.0

CONFIG(debug, debug|release) {

	TARGET = TestRestreamVideoView10_VS12_x64

	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\Debug\\*.exe c:\\csr\\exe /f/y"
 } else {
	TARGET = TestRestreamVideoView10_VS12_x64_R
	LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib 

	QMAKE_POST_LINK = "xcopy .\\release\\*.exe c:\\csr\\exe /f/y"
 }


