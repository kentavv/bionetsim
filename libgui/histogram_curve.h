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

#ifndef _HISTOGRAM_CURVE_H_
#define _HISTOGRAM_CURVE_H_

#include <qwt/include/qwt_plot_classes.h>

class QwtPlot;

class HistogramCurve : public QwtPlotCurve {
public:
  HistogramCurve(QwtPlot *parent, const char *name = NULL);
  ~HistogramCurve();

  void setRawMat(const float *mat, int w, int h, const double *time, float ymin, float ymax);

  void draw(QPainter *p, const QwtDiMap &xMap, const QwtDiMap &yMap,
	    int from = 0, int to = -1);

protected:
  const float *mat_;
  const double *time_;
  int w_, h_;
  float min_, max_;
  float ymin_, ymax_;
  
  void findMinMax();
  
  void drawHisto(QPainter *p,
		 const QwtDiMap &xMap, const QwtDiMap &yMap,
		 int from, int to);
};

#endif
