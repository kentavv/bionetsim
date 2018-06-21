TEMPLATE	= app
include(../qmake.inc)
win32:CONFIG    += console

INCLUDEPATH     += ../libnetwork/src /usr/local/include
unix:LIBS       += ../libnetwork/libnetwork.a ../libfitness/libfitness.a
win32:LIBS      += ../libnetwork/network.lib ../libfitness/fitness.lib

TARGET		= ga_ode_opt
TARGETDEPS      = $$LIBS
DEPENDPATH      = $$INCLUDEPATH
QMAKE_CLEAN     += $$TARGET

unix:LIBS       += -lgsl -lgslcblas

HEADERS		= evolve_params.h \
                  individual.h \
                  population.h \
                  simplex.h \
                  simulated_annealing.h

SOURCES		= ga_ode_opt.cpp \
                  evolve_params.cpp \
                  individual.cpp \
                  population.cpp \
                  simplex.cpp \
                  simulated_annealing.cpp

