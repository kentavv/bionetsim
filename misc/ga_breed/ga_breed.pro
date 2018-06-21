TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libplugin ../libnetwork/src ../libnetwork/src/ranlib.c/src
unix:LIBS       += ../libplugin/libplugin.a ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libplugin/plugin.lib ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= ga_breed
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= evolve_params.h \
                  version.h

SOURCES		= evolve_params.cpp \
                  ga_breed.cpp

