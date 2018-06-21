TEMPLATE	= app
include(../qmake.inc)

INCLUDEPATH     += ../libnetwork/src ../libgui
unix:LIBS       += ../libnetwork/libnetwork.a ../libgui/libgui.a
win32:LIBS      += ../libnetwork/network.lib ../libgui/gui.lib

TARGET		= fitness_landscape
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= fitness_landscape_app.h \
                  fitness_landscape_view.h \
                  version.h

SOURCES		= main.cpp \
                  fitness_landscape_app.cpp \
                  fitness_landscape_view.cpp
