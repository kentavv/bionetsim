TEMPLATE	= app
include(../../qmake.inc)

INCLUDEPATH     += ../../libplugin ../../libnetwork/src
unix:LIBS       += ../../libplugin/libplugin.a ../../libnetwork/libnetwork.a ../../libfitness/libfitness.a
win32:LIBS      += ../../libplugin/plugin.lib ../../libnetwork/network.lib ../../libfitness/fitness.lib

TARGET		= ode_opt_1d
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= scan_params.h

SOURCES		= ode_opt_1d.cpp \
                  scan_params.cpp

