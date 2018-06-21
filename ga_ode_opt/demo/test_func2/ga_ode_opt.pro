TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../../libnetwork/src /usr/local/include
TMAKE_LFLAGS    += -L/usr/local/lib -lgsl -lgslcblas
unix:LIBS       += ../../libnetwork/libnetwork.a
win32:LIBS      += ../../libnetwork/network.lib

TARGET		= ga_ode_opt
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= evolve_params.h \
                  individual.h \
                  population.h \
                  simplex.h \
                  version.h

SOURCES		= ga_ode_opt.cpp \
                  evolve_params.cpp \
                  individual.cpp \
                  population.cpp \
                  simplex.cpp
