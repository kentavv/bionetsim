TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= report
TARGETDEPS      = $$LIB
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= pathway_reports.h

SOURCES		= pathway_reports.cpp \
                  report.cpp

