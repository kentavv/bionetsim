TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src
unix:LIBS       += ../libnetwork/libnetwork.a
win32:LIBS      += ../libnetwork/network.lib

TARGET		= ode_gen
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		=

SOURCES		= ode_gen.cpp

