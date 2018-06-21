TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

unix:LIBS       += -lgsl -lgslcblas

TARGET		= de_ode_opt
TARGETDEPS      = $$LIB
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

HEADERS		= de.h \
                  evolve_params.h \
                  individual.h \
                  population.h \
                  simplex.h \
                  simulated_annealing.h

SOURCES		= de36.cpp \
                  evolve_params.cpp \
                  individual.cpp \
                  population.cpp \
                  simplex.cpp \
                  simulated_annealing.cpp
