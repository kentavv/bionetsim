TEMPLATE	= app
include(../qmake.inc)
CONFIG += console

CONFIG += embed_manifest_exe

INCLUDEPATH     += ../libnetwork/src ../libgui ../librxnparse
unix:LIBS       += ../libnetwork/libnetwork_gui.a ../libfitness/libfitness.a ../libgui/libgui.a ../libnetwork/libnetwork_gui.a ../libfitness/libfitness.a ../libgui/libgui.a ../librxnparse/librxnparse.a
win32:LIBS      += ../libnetwork/network_gui.lib ../libfitness/fitness.lib ../libgui/gui.lib ../librxnparse/rxnparse.lib

TARGET		= network_editor
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

#unix:LIBS       += -lgsl -lgslcblas

HEADERS		= 

SOURCES		= main.cpp
