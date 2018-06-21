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

#include <cfloat>

#include <math.h>

#include <qpainter.h>

#include <qwt_plot_classes.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include "histogram_curve.h"
#include "math_utils.h"

HistogramCurve::HistogramCurve(QwtPlot *parent, const char *name)
  : QwtPlotCurve(parent, name),
    mat_(NULL),
    time_(NULL),
    w_(0),
    h_(0),
    min_(-1),
    max_(-1),
    ymin_(0),
    ymax_(1)
{
}

HistogramCurve::~HistogramCurve() {
}

void
HistogramCurve::setRawMat(const float *mat, int w, int h, const double *time, float ymin, float ymax) {
  mat_ = mat;
  time_ = time;
  w_ = w;
  h_ = h;
  ymin_ = ymin;
  ymax_ = ymax;

  findMinMax();
}

void
HistogramCurve::findMinMax() {
  min_ = FLT_MAX;
  max_ = -FLT_MAX;
  int n = w_ * h_;
  for(int i=0; i<n; i++) {
    float v = mat_[i];
    if(v < min_) min_ = v;
    if(v > max_) max_ = v;
  }
}

void
HistogramCurve::draw(QPainter *painter,
		     const QwtDiMap &xMap, const QwtDiMap &yMap,
		     int from, int to) {
  if(mat_) drawHisto(painter, xMap, yMap, from, to);
  QwtPlotCurve::draw(painter, xMap, yMap, from, to);
}

void
HistogramCurve::drawHisto(QPainter *p,
			  const QwtDiMap &xMap, const QwtDiMap &yMap,
			  int /*from*/, int /*to*/) {
  if(w_ < 2) return;
  int bw = GenoDYN::roundToInt(ceil(xMap.xTransform(time_[1]) - xMap.xTransform(time_[0])));
	int bh = GenoDYN::roundToInt(ceil(yMap.xTransform(ymin_) - yMap.xTransform((ymax_ - ymin_)/h_ + ymin_)));
	for(int y=0; y<h_; y++) {
		int yb = yMap.transform(y/(float)h_*(ymax_ - ymin_) + ymin_);
		for(int x=0; x<w_;) {
			// Average together otherwise overlapping bins
			// Only done along the time dimension as this is the most likely place for 
			// the situation to occur such with fast sampling rates
			int n = 0;
			float s = 0;
			while(x+n < w_ && xMap.transform(time_[x]) == xMap.transform(time_[x+n])) {
				s += mat_[y*w_ + x + n];
				n++;
			}

			s /= n;

			if(s > 0) {
				//float v = (s - min_) / (max_ - min_);
				float v = s / max_;
				QRect r(xMap.transform(time_[x]), yb, bw, -bh);
				p->fillRect(r, QColor((int)((1.0-v)*240+0.5), 255, 255, QColor::Hsv));
			}

			x += n;
		}
	}
}
