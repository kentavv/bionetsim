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

#ifndef _LINE_GRAPH_VIEW_
#define _LINE_GRAPH_VIEW_

#include <qstringlist.h>
#include <qtooltip.h>

#include "matrix.h"
#include "plot_view.h"
#include "result.h"

class QPopupMenu;
class LGVConfigDialog;
class TextViewer;

class LGVTip : public QToolTip {
public:
  LGVTip(QWidget *parent);
  
protected:
  void maybeTip(const QPoint &);
};

class LineGraphView : public PlotView {
  Q_OBJECT
public:
  LineGraphView(QWidget *parent);
  ~LineGraphView();
  
  void clear();
  void setData(const Matrix &data);
  void setData(const Matrix &data, const QString &label);
  void setData(const Matrix &data, const QStringList &labels);
  void setData(const Result &result);

  void setRawData(const double * const *data, int nr, int nc);
  void setRawData(const double * const *data, int nr, int nc, const QStringList &labels);

  bool isGraphVisible(int i);
  void setGraphVisible(int, bool);
  void setAllVisible(bool);

  void tip(QPoint p, QRect &r, QString &str) const;
	    
protected slots:
  void graphMenu(int);

  void plotMouseMoved(const QMouseEvent&);
  void plotMouseReleased(const QMouseEvent&);

protected:
  double **cdata_;
  const double * const *data_;
  int nr_, nc_;
  
  LGVConfigDialog *config_;

  LGVTip *tip_;

  TextViewer *tv_;
  
  static QColor colors[13];

  void updateInternals();
  void exportData(const QString&);

  int nGraphsVisible() const;
  void clearCopiedData();
};

#endif
