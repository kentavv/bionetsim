TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src
TMAKE_LFLAGS    += -L/usr/local/lib
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= random_simplex
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

unix:LIBS       += -lgsl -lgslcblas

HEADERS		= evolve_params.h \
                  individual.h \
                  population.h \
                  simplex.h \
                  simulated_annealing.h \
                  version.h

SOURCES		= random_simplex.cpp \
                  evolve_params.cpp \
                  individual.cpp \
                  population.cpp \
                  simplex.cpp \
                  simulated_annealing.cpp
