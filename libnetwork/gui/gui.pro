TEMPLATE	= lib
include(../../qmake.inc)
CONFIG          += staticlib
VERSION         = 0.05

INCLUDEPATH     += ../src/sundials/cvode/include ../src/sundials/shared/include \
                   ../src/sundials/shared/source ../src/sundials/nvec_ser \
                   ../../libgui ../../libfitness ../src
VPATH           = ../src
DESTDIR         = ../

DEFINES		+= GUI

TARGET		= network_gui
DEPENDPATH      = $$INCLUDEPATH

HEADERS		= calculate.h \
                  calculate_cvode.h \
                  calculate_gillespie_direct.h \
                  calculate_stochastic_rates.h \
                  diag_matrix.h \
                  editor.h \
                  fitness.h \
                  fitness_functions.h \
                  image_utils.h \
                  log.h \
                  math_utils.h \
                  matrix.h \
                  net_utils.h \
                  nr.h \
                  ode_gen.h \
                  pathway_canvas_items.h \
                  pathway_canvas_utils.h \
                  pathway_load.h \
                  pathway_load_v18.h \
                  pathway_load_v19.h \
                  pathway_load_v20.h \
                  pathway_load_v21.h \
                  pathway_load_v22.h \
                  pathway_load_v23.h \
                  pathway_load_v24.h \
                  pathway_load_v25.h \
                  pathway_matrix.h \
                  pathway_reports.h \
                  pathway_save.h \
                  pne_assert.h \
                  random.h \
                  result.h \
                  simulation.h \
                  squarewave.h \
                  version.h

SOURCES		= calculate.cpp \
                  calculate_cvode.cpp \
                  calculate_gillespie_direct.cpp \
                  calculate_stochastic_rates.cpp \
                  editor.cpp \
                  fitness.cpp \
                  fitness_functions.cpp \
                  image_utils.cpp \
                  log.cpp \
                  net_utils.cpp \
                  nr.c \
                  ode_gen.cpp \
                  pathway_canvas_items.cpp \
                  pathway_canvas_utils.cpp \
                  pathway_load.cpp \
                  pathway_load_v18.cpp \
                  pathway_load_v19.cpp \
                  pathway_load_v20.cpp \
                  pathway_load_v21.cpp \
                  pathway_load_v22.cpp \
                  pathway_load_v23.cpp \
                  pathway_load_v24.cpp \
                  pathway_load_v25.cpp \
                  pathway_matrix.cpp \
                  pathway_reports.cpp \
                  pathway_save.cpp \
                  pne_assert.cpp \
                  random.cpp \
                  result.cpp \
                  simulation.cpp \
                  squarewave.cpp \
                  version.cpp


# CVODE sources

SOURCES        += sundials/cvode/fcmix/fcvode.c \
                  sundials/cvode/fcmix/fcvpreco.c \
                  sundials/cvode/fcmix/fcvpsol.c \
                  sundials/cvode/fcmix/fcvjtimes.c \
                  sundials/cvode/fcmix/fcvband.c \
                  sundials/cvode/fcmix/fcvdense.c \
                  sundials/cvode/fcmix/fcvbp.c \
                  sundials/cvode/fcmix/fcvbbd.c \
                  sundials/cvode/fcmix/fcvroot.c \
                  sundials/cvode/source/cvode.c \
                  sundials/cvode/source/cvodeio.c \
                  sundials/cvode/source/cvdense.c \
                  sundials/cvode/source/cvband.c \
                  sundials/cvode/source/cvdiag.c \
                  sundials/cvode/source/cvspgmr.c \
                  sundials/cvode/source/cvbandpre.c \
                  sundials/cvode/source/cvbbdpre.c \
                  sundials/shared/source/sundialsmath.c \
                  sundials/shared/source/nvector.c \
                  sundials/shared/source/dense.c \
                  sundials/shared/source/smalldense.c \
                  sundials/shared/source/band.c \
                  sundials/shared/source/spgmr.c \
                  sundials/shared/source/iterative.c \
                  sundials/nvec_ser/nvector_serial.c \
                  sundials/nvec_ser/fnvector_serial.c
