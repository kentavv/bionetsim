TEMPLATE	= app
include(../qmake.inc)

INCLUDEPATH     += ../libnetwork/src ../libgui
unix:LIBS       += ../libnetwork/libnetwork_gui.a ../libgui/libgui.a
win32:LIBS      += ../libnetwork/network_gui.lib ../libgui/gui.lib

DEFINES         += GUI

TARGET		= network_evolve
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= evolve_thread.h \
                  network_evolve.h \
                  network_evolve_app.h \
                  pathway_editor.h \
                  splash.h \
                  version.h

SOURCES		= evolve_thread.cpp \
                  main.cpp \
                  network_evolve.cpp \
                  network_evolve_app.cpp \
                  pathway_editor.cpp \
                  splash.cpp
