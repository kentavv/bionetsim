TEMPLATE	= app
include(../../qmake.inc)

INCLUDEPATH     += ../../libnetwork/src ../../libgui
unix:LIBS       += ../../libnetwork/libnetwork_gui.a ../../libgui/libgui.a ../../libnetwork/libnetwork_gui.a ../../libgui/libgui.a
win32:LIBS      += ../../libnetwork/network_gui.lib ../../libgui/gui.lib

TARGET		= viewer
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= 

SOURCES		= main.cpp
