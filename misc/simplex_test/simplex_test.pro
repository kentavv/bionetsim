TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src /usr/local/include
TMAKE_LFLAGS    += -L/usr/local/lib -lgsl -lgslcblas
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= simplex_test
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		=

SOURCES		= simplex_test.cpp
