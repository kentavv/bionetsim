TEMPLATE	= app
CONFIG		= qt thread warn_on debug
#CONFIG		= qt thread release
win32:CONFIG	+= windows

win32:TMAKE_CXXFLAGS += /GX
win32:TMAKE_LFLAGS += /nodefaultlib:libc.lib

HEADERS		= 

SOURCES		= main.cpp

TARGET		= guet_agent

win32:DEFINES	+= QT_DLL
unix:DEFINES	+=

