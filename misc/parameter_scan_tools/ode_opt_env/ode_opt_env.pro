TEMPLATE	= app
include(../../qmake.inc)

INCLUDEPATH     += ../../libplugin ../../libnetwork/src
unix:LIBS       += ../../libplugin/libplugin.a ../../libnetwork/libnetwork.a
win32:LIBS      += ../../libplugin/plugin.lib ../../libnetwork/network.lib

TARGET		= ode_opt_env
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		=

SOURCES		= ode_opt_env.cpp

