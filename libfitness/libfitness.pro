TEMPLATE	= lib
#TEMPLATE	= app
include(../qmake.inc)
CONFIG		+= staticlib
#CONFIG		+= console debug
VERSION         = 0.01

#DEFINES += STAND_ALONE

INCLUDEPATH     += ../libnetwork/src

TARGET		= fitness

HEADERS		= AstNode.h \
                  STEClasses.h \
                  SymTab.h \
                  SymTabEntry.h \
                  SymTabMgr.h \
                  Value.h \
                  fitness_lang.h \
                  misc.h

SOURCES		= AstNode.cpp \
                  STEClasses.cpp \
                  Value.cpp \
                  demo.cpp \
                  fitness_lang.cpp

LEXSOURCES = FF.l
YACCSOURCES = FF.y

