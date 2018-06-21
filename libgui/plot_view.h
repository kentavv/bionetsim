// This file is part of GenoDYN.
//
// GenoDYN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GenoDYN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenoDYN.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008 Kent A. Vander Velden

#ifndef _PLOT_VIEW_H_
#define _PLOT_VIEW_H_

#include <qstringlist.h>

#include "qwt/include/qwt_plot.h"

class QPainter;

class QPopupMenu;

class PlotView : public QwtPlot {
  Q_OBJECT
public:
  PlotView(QWidget *parent);
  ~PlotView();

  bool absoluteScale();
  void setAbsoluteScale(bool);

  void draw(QPainter*);

signals:
  void fullScreenRequested(PlotView*);

protected slots:
  virtual void graphMenu(int);
  virtual void updateGraphMenu();
 
  void plotMousePressed(const QMouseEvent &);
 
protected:
  int nvars_;
  long *curves_;
  QPopupMenu *graph_menu_;
  QStringList labels_;

  bool zoom_;
  bool zoomed_;
  QPoint p1_;

  bool absolute_scale_;

  virtual void updateInternals() = 0;

  void fillGraphMenu();
  virtual void exportData(const QString &) = 0;
};

#endif
