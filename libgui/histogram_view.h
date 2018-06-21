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

#ifndef _HISTOGRAM_VIEW_H_
#define _HISTOGRAM_VIEW_H_

#include <qstringlist.h>

#include "matrix.h"
#include "plot_view.h"

class QPopupMenu;

class HistogramView : public PlotView {
  Q_OBJECT
public:
  HistogramView(QWidget *parent = NULL);
  ~HistogramView();

  void clear();
  void setData(const QPtrList<Matrix> &tel,
	       const QString &label);
  void setData(const QPtrList<Matrix> &tel,
	       const QStringList &labels);

  void setGraphVisible(int);

protected slots:
  void graphMenu(int);
  void updateGraphMenu();
  
  void plotMouseReleased(const QMouseEvent&);
  
protected:
  QPtrList<Matrix> tel_;
  float **data_;
  double **avg_;
  double *time_;
  float *min_, *max_;
  int nsamples_, *nbins_;
  int g_;
  
  void updateInternals();
  void exportData(const QString&);

  void keyPressEvent(QKeyEvent*);

  void findRange();
  void computeMean();
  void arrangeDataIntoBins();
  void copyTime();
  void assignDataToCurves();
};

#endif
