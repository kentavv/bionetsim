#############################################################################
# Makefile for building: librxnparse.a
# Generated by qmake (1.07a) (Qt 3.3.8b) on: Sat Mar 14 19:14:29 2009
# Project:  librxnparse.pro
# Template: lib
# Command: $(QMAKE) -o Makefile librxnparse.pro
#############################################################################

####### Compiler, tools and options

CC       = gcc
CXX      = g++
LEX      = flex
YACC     = yacc
CFLAGS   = -pipe -Wall -W -g -D_REENTRANT  -DQT_THREAD_SUPPORT
CXXFLAGS = -pipe -Wall -W -g -D_REENTRANT  -DQT_THREAD_SUPPORT
LEXFLAGS = 
YACCFLAGS= -d
INCPATH  = -I/usr/lib64/qt-3.3/mkspecs/default -I. -I../libnetwork/src -I$(QTDIR)/include
AR       = ar cqs
RANLIB   = 
MOC      = $(QTDIR)/bin/moc
UIC      = $(QTDIR)/bin/uic
QMAKE    = qmake
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### Output directory

OBJECTS_DIR = ./

####### Files

HEADERS = RP_AstNode.h \
		rxn_parser.h \
		RP_misc.h
SOURCES = RP_AstNode.cpp \
		RP_demo.cpp \
		rxn_parser.cpp \
		RP_lex.cpp \
		RP_yacc.cpp
OBJECTS = RP_AstNode.o \
		RP_demo.o \
		rxn_parser.o \
		RP_lex.o \
		RP_yacc.o
FORMS = 
UICDECLS = 
UICIMPLS = 
SRCMOC   = 
OBJMOC = 
DIST	   = ../qmake.inc \
		librxnparse.pro
QMAKE_TARGET = rxnparse
DESTDIR  = 
TARGET   = librxnparse.a

first: all
####### Implicit rules

.SUFFIXES: .c .o .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: Makefile $(TARGET) 

staticlib: $(TARGET)

$(TARGET):  $(UICDECLS) $(OBJECTS) $(OBJMOC) $(OBJCOMP) 
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)


mocables: $(SRCMOC)
uicables: $(UICDECLS) $(UICIMPLS)

$(MOC): 
	( cd $(QTDIR)/src/moc && $(MAKE) )

Makefile: librxnparse.pro  /usr/lib64/qt-3.3/mkspecs/default/qmake.conf ../qmake.inc
	$(QMAKE) -o Makefile librxnparse.pro
qmake: 
	@$(QMAKE) -o Makefile librxnparse.pro

dist: 
	@mkdir -p .tmp/rxnparse && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) .tmp/rxnparse/ && ( cd `dirname .tmp/rxnparse` && $(TAR) rxnparse.tar rxnparse && $(GZIP) rxnparse.tar ) && $(MOVE) `dirname .tmp/rxnparse`/rxnparse.tar.gz . && $(DEL_FILE) -r .tmp/rxnparse

mocclean:

uiclean:

yaccclean:
	-$(DEL_FILE) RP_yacc.cpp RP_yacc.h
lexclean:
	-$(DEL_FILE) RP_lex.cpp
clean: yaccclean lexclean
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) $(TARGET)


FORCE:

####### Compile

RP_AstNode.o: RP_AstNode.cpp RP_AstNode.h

RP_demo.o: RP_demo.cpp RP_AstNode.h

rxn_parser.o: rxn_parser.cpp RP_AstNode.h \
		rxn_parser.h

RP_lex.o: RP_lex.cpp RP_AstNode.h \
		RP_yacc.h

RP_yacc.o: RP_yacc.cpp RP_yacc.h \
		RP_AstNode.h

RP_yacc.cpp: RP.y
	$(YACC) $(YACCFLAGS) -p RP -b RP RP.y
	-$(DEL_FILE) RP_yacc.cpp RP_yacc.h
	-$(MOVE) RP.tab.h RP_yacc.h
	-$(MOVE) RP.tab.c RP_yacc.cpp

RP_yacc.h: RP_yacc.cpp

RP_lex.cpp: RP.l 
	$(LEX) $(LEXFLAGS) -PRP RP.l
	-$(DEL_FILE) RP_lex.cpp 
	-$(MOVE) lex.RP.c RP_lex.cpp

####### Install

install:  

uninstall:  

