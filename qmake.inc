CONFIG		= warn_on debug qt thread
#CONFIG		= warn_on release qt thread
win32:CONFIG    += windows

win32:TMAKE_CXXFLAGS += /EHsc
win32:TMAKE_LFLAGS   += /nodefaultlib:libc.lib
win32:DEFINES        += WIN32 QT_DLL Q_TEMPLATE_EXTERN=extern

#win32:TMAKE_CXXFLAGS += -Zi -DEBUG

#Options needed for Rational Suite
#win32:TMAKE_CXXFLAGS += /Zi /W3
#win32:TMAKE_LFLAGS   += /debug:full /fixed:no

#Options suggested for Rational Suite
#win32:TMAKE_CXXFLAGS += /Ge
#win32:TMAKE_LFLAGS   +=

#DEFINES         += NDEBUG

linux-g++:TMAKE_CXXFLAGS += -Wwrite-strings
#linux-g++:TMAKE_CXXFLAGS += -fomit-frame-pointer
linux-g++:TMAKE_CXXFLAGS += -fmessage-length=0
#linux-g++:TMAKE_CXXFLAGS += -pg
#linux-g++:TMAKE_LFLAGS += -pg

