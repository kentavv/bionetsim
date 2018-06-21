TEMPLATE	= lib
#TEMPLATE	= app
include(../qmake.inc)
CONFIG		+= staticlib
#CONFIG		+= console debug
VERSION         = 0.01

#DEFINES += STAND_ALONE

INCLUDEPATH     += ../libnetwork/src

TARGET		= rxnparse

HEADERS		= RP_AstNode.h \
                  rxn_parser.h \
                  RP_misc.h

SOURCES		= RP_AstNode.cpp \
                  RP_demo.cpp \
                  rxn_parser.cpp

LEXSOURCES = RP.l
YACCSOURCES = RP.y

