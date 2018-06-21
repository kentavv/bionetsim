TEMPLATE	= lib
include(../qmake.inc)
CONFIG		+= staticlib
VERSION         = 0.01

INCLUDEPATH     += ../libnetwork/src ../libfitness ../librxnparse

TARGET		= gui

HEADERS		= annotation_dialog.h \
                  concentration_dialog.h \
                  dynamic_dialog.h \
                  editor_params_dialog.h \
                  environments_dialog.h \
                  fitness_params_dialog.h \
                  histogram_dialog.h \
                  histogram_curve.h \
                  histogram_view.h \
                  interpolated_cv_dialog.h \
                  lgv_config_dialog.h \
                  line_graph_view.h \
                  molecule_dialog.h \
                  network_editor.h \
                  network_editor_mw.h \
                  parameter_scan_dialog.h \
                  parameter_scan_2d_dialog.h \
                  phase_plane_view.h \
                  plot_dialog.h \
                  plot_view.h \
                  reaction_dialog.h \
                  reaction_entry_dialog.h \
                  reaction_view.h \
                  report_dialog.h \
                  scaled_matrix_view.h \
                  simulation_params_dialog.h \
                  slider_dialog.h \
                  splash_screen.h \
                  squarewave_cv_dialog.h \
                  subnetwork_dialog.h \
                  textviewer.h \
                  vcrcontrols.h

SOURCES		= annotation_dialog.cpp \
                  concentration_dialog.cpp \
                  dynamic_dialog.cpp \
                  editor_params_dialog.cpp \
                  environments_dialog.cpp \
                  fitness_params_dialog.cpp \
                  histogram_curve.cpp \
                  histogram_dialog.cpp \
                  histogram_view.cpp \
                  interpolated_cv_dialog.cpp \
                  lgv_config_dialog.cpp \
                  line_graph_view.cpp \
                  molecule_dialog.cpp \
                  network_editor.cpp \
                  network_editor_mw.cpp \
                  parameter_scan_dialog.cpp \
                  parameter_scan_2d_dialog.cpp \
                  phase_plane_view.cpp \
                  plot_dialog.cpp \
                  plot_view.cpp \
                  reaction_dialog.cpp \
                  reaction_entry_dialog.cpp \
                  reaction_view.cpp \
                  report_dialog.cpp \
                  scaled_matrix_view.cpp \
                  simulation_params_dialog.cpp \
                  slider_dialog.cpp \
                  splash_screen.cpp \
                  squarewave_cv_dialog.cpp \
                  subnetwork_dialog.cpp \
                  textviewer.cpp \
                  vcrcontrols.cpp


VPATH             += qwt
INCLUDEPATH       += qwt/include
DEPENDPATH        += qwt/include qwt/src $$INCLUDEPATH

HEADERS += \
    examples/realtime_plot/scrollbar.h \
    examples/realtime_plot/scrollzoomer.h \

SOURCES += \
    examples/realtime_plot/scrollbar.cpp \
    examples/realtime_plot/scrollzoomer.cpp \

# Qwt Source files (from qwt.pro)

HEADERS += \
    include/qwt_global.h \
    include/qwt.h \
    include/qwt_array.h \
    include/qwt_autoscl.h \
    include/qwt_curve.h \
    include/qwt_data.h \
    include/qwt_dimap.h \
    include/qwt_double_rect.h \
    include/qwt_event_pattern.h \
    include/qwt_grid.h \
    include/qwt_layout_metrics.h \
    include/qwt_legend.h \
    include/qwt_marker.h \
    include/qwt_math.h \
    include/qwt_painter.h \
    include/qwt_paint_buffer.h \
    include/qwt_picker.h \
    include/qwt_picker_machine.h \
    include/qwt_plot.h \
    include/qwt_plot_layout.h \
    include/qwt_plot_printfilter.h \
    include/qwt_plot_classes.h \
    include/qwt_plot_item.h \
    include/qwt_plot_dict.h \
    include/qwt_plot_canvas.h \
    include/qwt_plot_picker.h \
    include/qwt_plot_zoomer.h \
    include/qwt_push_button.h \
    include/qwt_rect.h \
    include/qwt_scale.h \
    include/qwt_scldiv.h \
    include/qwt_scldraw.h \
    include/qwt_spline.h \
    include/qwt_symbol.h \
    include/qwt_dyngrid_layout.h

SOURCES += \
    src/qwt_autoscl.cpp \
    src/qwt_curve.cpp \
    src/qwt_data.cpp \
    src/qwt_dimap.cpp \
    src/qwt_double_rect.cpp \
    src/qwt_event_pattern.cpp \
    src/qwt_grid.cpp \
    src/qwt_layout_metrics.cpp \
    src/qwt_legend.cpp \
    src/qwt_marker.cpp \
    src/qwt_math.cpp \
    src/qwt_painter.cpp \
    src/qwt_paint_buffer.cpp \
    src/qwt_picker.cpp \
    src/qwt_picker_machine.cpp \
    src/qwt_plot.cpp \
    src/qwt_plot_axis.cpp \
    src/qwt_plot_curve.cpp \
    src/qwt_plot_grid.cpp \
    src/qwt_plot_item.cpp \
    src/qwt_plot_print.cpp \
    src/qwt_plot_marker.cpp \
    src/qwt_plot_layout.cpp \
    src/qwt_plot_printfilter.cpp \
    src/qwt_plot_canvas.cpp \
    src/qwt_plot_picker.cpp \
    src/qwt_plot_zoomer.cpp \
    src/qwt_push_button.cpp \
    src/qwt_rect.cpp \
    src/qwt_scale.cpp \
    src/qwt_scldiv.cpp \
    src/qwt_scldraw.cpp \
    src/qwt_spline.cpp \
    src/qwt_symbol.cpp \
    src/qwt_dyngrid_layout.cpp
 
##############################################
# If you are interested in the plot widget
# only, you can remove the following 
# HEADERS/SOURCES lists.
##############################################

HEADERS += \
    include/qwt_arrbtn.h \
    include/qwt_analog_clock.h \
    include/qwt_compass.h \
    include/qwt_compass_rose.h \
    include/qwt_dial.h \
    include/qwt_dial_needle.h \
    include/qwt_counter.h \
    include/qwt_drange.h \
    include/qwt_knob.h \
    include/qwt_sclif.h \
    include/qwt_sldbase.h \
    include/qwt_slider.h \
    include/qwt_text.h \
    include/qwt_thermo.h \
    include/qwt_wheel.h

SOURCES += \
    src/qwt_arrbtn.cpp \
    src/qwt_analog_clock.cpp \
    src/qwt_compass.cpp \
    src/qwt_compass_rose.cpp \
    src/qwt_dial.cpp \
    src/qwt_dial_needle.cpp \
    src/qwt_counter.cpp \
    src/qwt_drange.cpp \
    src/qwt_knob.cpp \
    src/qwt_sclif.cpp \
    src/qwt_sldbase.cpp \
    src/qwt_slider.cpp \
    src/qwt_text.cpp \
    src/qwt_thermo.cpp \
    src/qwt_wheel.cpp

#debug:DEFINES += QWT_DEBUG
