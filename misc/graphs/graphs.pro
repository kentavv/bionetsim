TEMPLATE	= app
include(../qmake.inc)

INCLUDEPATH     += ../libnetwork/src
unix:LIBS       += ../libnetwork/libnetwork.a
win32:LIBS      += ../libnetwork/network.lib

TARGET		= graphs
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

unix:LIBS       += -lgsl -lgslcblas

HEADERS		= 

SOURCES		= main.cpp
