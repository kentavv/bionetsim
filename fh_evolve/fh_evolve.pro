TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= fh_evolve
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= individual.h

SOURCES		= fh_evolve.cpp \
                  individual.cpp

