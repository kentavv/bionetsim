TEMPLATE	= app
win32:CONFIG    += console
CONFIG		= warn_on debug qt thread
#CONFIG		= warn_on release qt thread
win32:CONFIG    += windows

win32:TMAKE_CXXFLAGS += -GX
win32:TMAKE_LFLAGS   += /nodefaultlib:libc.lib
win32:DEFINES        += WIN32 QT_DLL Q_TEMPLATE_EXTERN=extern

#DEFINES         += NDEBUG

linux-g++:TMAKE_CXXFLAGS += -Wwrite-strings
linux-g++:TMAKE_CXXFLAGS += -rdynamic 
linux-g++:TMAKE_LFLAGS += -rdynamic 
#linux-g++:TMAKE_CXXFLAGS += -fomit-frame-pointer
linux-g++:TMAKE_CXXFLAGS += -fmessage-length=0
#linux-g++:TMAKE_CXXFLAGS += -pg
#linux-g++:TMAKE_LFLAGS += -pg

TARGET		= comm_test
QMAKE_CLEAN     += $$TARGET

HEADERS		= matrix.h \
                  net_utils.h \
                  nr.h \
                  pne_assert.h \
                  random.h

SOURCES		= comm_test.cpp \
                  net_utils.cpp \
                  nr.c \
                  pne_assert.cpp \
                  random.cpp
