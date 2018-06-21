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

#ifndef _PHASE_PLANE_VIEW_
#define _PHASE_PLANE_VIEW_

#include <qstringlist.h>
#include <qvaluevector.h>

#include "qwt/include/qwt_plot.h"

#include "matrix.h"

class QPopupMenu;
class LGVConfigDialog;

class PhasePlaneView : public QwtPlot {
  Q_OBJECT
public:
  PhasePlaneView(QWidget *parent);
  ~PhasePlaneView();
  
  void clear();
  void setData(const Matrix &data);
  void setData(const Matrix &data, const QStringList &labels);

  void setRawData(const double * const *data, int nr, int nc);
  void setRawData(const double * const *data, int nr, int nc, const QStringList &labels);

  void setGraphs(int, int);

  void draw(QPainter*);
	    
protected slots:
  void xMenu(int);
  void yMenu(int);
  void graphMenu(int);
  void updateXMenu();
  void updateYMenu();
  void displayGraphMenu(const QMouseEvent&);
   
  void plotMouseMoved(const QMouseEvent&);
  void plotMousePressed(const QMouseEvent&);
  void plotMouseReleased(const QMouseEvent&);

 protected:
  double **cdata_;
  const double * const *data_;
  int nr_, nc_;
  int x_, y_;
  long curve_;
  long markers_[2];
  QStringList labels_;
  
  LGVConfigDialog *config_;
  QPopupMenu *x_menu_;
  QPopupMenu *y_menu_;
  QPopupMenu *graph_menu_;

  bool zoom_;
  bool zoomed_;
  QPoint p1_;

  void updateInternals();
  int nGraphsVisible() const;
  QString label(int) const;

  void keyPressEvent(QKeyEvent*);
};

#endif
