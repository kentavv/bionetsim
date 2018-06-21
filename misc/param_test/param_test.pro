TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libplugin ../libnetwork/src
unix:LIBS       += ../libplugin/libplugin.a ../libnetwork/libnetwork.a
win32:LIBS      += ../libplugin/plugin.lib ../libnetwork/network.lib

TARGET		= param_test
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		=

SOURCES		= param_test.cpp
