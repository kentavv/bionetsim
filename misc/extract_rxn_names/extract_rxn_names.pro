TEMPLATE	= app
include(../../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../../libnetwork/src
unix:LIBS       += ../../libnetwork/libnetwork.a ../../libfitness/libfitness.a
win32:LIBS      += ../../libnetwork/network.lib ../../libfitness/fitness.lib

TARGET		= extract_rxn_names
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		=

SOURCES		= extract_rxn_names.cpp
