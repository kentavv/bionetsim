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

#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>

#include "qwt_math.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"

#include "histogram_curve.h"
#include "histogram_view.h"
#include "math_utils.h"
#include "version.h"

HistogramView::HistogramView(QWidget *parent)
  : PlotView(parent),
    data_(NULL),
    avg_(NULL),
    time_(NULL),
    min_(NULL),
    max_(NULL),
    nsamples_(0),
    nbins_(NULL),
    g_(-1)
{
  connect(this, SIGNAL(plotMouseReleased(const QMouseEvent &)),
	  SLOT(plotMouseReleased(const QMouseEvent&)));
}

HistogramView::~HistogramView() {
  clear();
}

void
HistogramView::clear() {
  if(data_) {
    for(int i=0; i<nvars_; i++) {
      delete data_[i];
    }
    delete[] data_;
    data_ = NULL;
  }
  if(avg_) {
    for(int i=0; i<nvars_; i++) {
      delete avg_[i];
    }
    delete[] avg_;
    avg_ = NULL;
  }
  if(time_) {
    delete[] time_;
    time_ = NULL;
  }
  if(min_) {
    delete[] min_;
    min_ = NULL;
  }
  if(max_) {
    delete[] max_;
    max_ = NULL;
  }
  if(nbins_) {
    delete[] nbins_;
    nbins_ = NULL;
  }

  QwtPlot::clear();
  if(curves_) delete[] curves_;
  curves_ = NULL;  
  data_ = NULL;
  graph_menu_->clear();
  graph_menu_->setEnabled(false);
  labels_.clear();

  nvars_ = 0;
  nsamples_ = 0;

  g_ = -1;

  zoom_ = false;
  zoomed_ = false;

  tel_.clear();
}

void 
HistogramView::setData(const QPtrList<Matrix> &tel,
		       const QString &label) {
  QStringList tmp;
  tmp.append(label);
  setData(tel, tmp);
}

void 
HistogramView::setData(const QPtrList<Matrix> &tel,
		       const QStringList &labels) {
  if(tel.isEmpty()) return;
  bool reset = (labels_   != labels || 
		nvars_    != tel.getFirst()->ncolumns()-1 ||
		nsamples_ != tel.getFirst()->nrows());

  if(reset) { 
    clear();
    nvars_    = tel.getFirst()->ncolumns()-1;
    nsamples_ = tel.getFirst()->nrows();
    labels_   = labels;
    min_      = new float[nvars_];
    max_      = new float[nvars_];
    data_     = new float*[nvars_];
    avg_      = new double*[nvars_];
    nbins_    = new int[nvars_];
    time_     = new double[nsamples_];
  } else {
    for(int i=0; i<nvars_; i++) {
      delete[] data_[i];
      delete[] avg_[i];
    }
  }

  tel_ = tel;
  tel_.setAutoDelete(true);

  findRange();

  computeMean();

  copyTime();

  if(reset) {
    for(int i=0; i<nvars_; i++) {
			nbins_[i] = GenoDYN::min(100, GenoDYN::roundToInt(max_[i] - min_[i]) + 1);
    }
  } else {
    // Clamp the number of bins in the range [1, M-m+1] to prevent
    // the graph oddities that occur when the number of bins exceeds
    // M-m+1
    for(int i=0; i<nvars_; i++) {
      nbins_[i] = GenoDYN::clamp(nbins_[i], 1, GenoDYN::roundToInt(max_[i] - min_[i]) + 1);
    }
  }

  arrangeDataIntoBins();

  setAxisMaxMinor(yLeft, 0);

  if(nvars_ == 0 || nsamples_ == 0 || !nbins_ || !data_) return;

  graph_menu_->setEnabled(true);
  
  if(reset) {
    if(g_ >= nvars_) {
      g_ = nvars_ - 1;
    }

    curves_ = new long[nvars_*2];
    for(int i=0; i<nvars_; i++) {
			if(i < (int)labels.count()) {
				curves_[i       ] = insertCurve(new HistogramCurve(this, labels[i]));
				curves_[i+nvars_] = insertCurve(labels[i] + " Average");
				graph_menu_->insertItem(labels[i], i);
			} else {
				QString str;
				str.sprintf("Graph %d", i+1);
				curves_[i       ] = insertCurve(new HistogramCurve(this, str));
				curves_[i+nvars_] = insertCurve(str + " Average");
				graph_menu_->insertItem(str, i);
			}
			curve(curves_[i       ])->setPen(black);
			curve(curves_[i+nvars_])->setPen(black);
      curve(curves_[i+nvars_])->setStyle(QwtCurve::Steps);
    }
  }

  assignDataToCurves();

  if(reset) {
    graph_menu_->insertSeparator();
    graph_menu_->insertItem("Plot histogram",       nvars_);
    graph_menu_->insertItem("Plot average",         nvars_+1);
    graph_menu_->setItemChecked(nvars_,   true);
    graph_menu_->setItemChecked(nvars_+1, true);

    graph_menu_->insertSeparator();
    graph_menu_->insertItem("Set number of bins", nvars_+2);
    graph_menu_->insertSeparator();

    PlotView::fillGraphMenu();
  }
  
  updateInternals();
}

void
HistogramView::findRange() {
  QPtrListIterator<Matrix> it(tel_);
  const Matrix *pm;
  int i, j;

  for(i=0; i<nvars_; i++) {
    min_[i] = FLT_MAX;
    max_[i] = -FLT_MAX;
  }

  for(pm = it.toFirst(); pm; pm = ++it) {
		const Matrix &m = *pm;
		for(i=0; i<nvars_; i++) {
			for(j=0; j<nsamples_; j++) {
				float v = m(j, i+1);
				if(v < min_[i]) min_[i] = v;
				if(v > max_[i]) max_[i] = v;
			}
		}
  }
}

void
HistogramView::computeMean() {
  QPtrListIterator<Matrix> it(tel_);
  const Matrix *pm;
  int i, j;
  int nreps = tel_.count();

  for(i=0; i<nvars_; i++) {
    avg_[i] = new double[nsamples_];
    for(j=0; j<nsamples_; j++) {
      avg_[i][j] = 0.0;
    }
  }

  for(pm = it.toFirst(); pm; pm = ++it) {
    const Matrix &m = *pm;
    for(i=0; i<nvars_; i++) {
			for(j=0; j<nsamples_; j++) {
				avg_[i][j] += m(j, i+1);
			}
		}
  }
  for(i=0; i<nvars_; i++) {
    for(j=0; j<nsamples_; j++) {
      avg_[i][j] /= nreps;
    }
  }
}

void
HistogramView::arrangeDataIntoBins() {
  QPtrListIterator<Matrix> it(tel_);
  const Matrix *pm;
  int i, j;

  for(i=0; i<nvars_; i++) {
    const int ncells = nbins_[i] * nsamples_;
    data_[i] = new float[ncells];
    for(j=0; j<ncells; j++) {
      data_[i][j] = 0.0;
    }
  }

	for(pm = it.toFirst(); pm; pm = ++it) {
		const Matrix &m = *pm;
		for(i=0; i<nvars_; i++) {
			for(j=0; j<nsamples_; j++) {
				float v = m(j, i+1);
				int k = 0;
				if(max_[i] - min_[i] > 0) {
					k = GenoDYN::roundToInt((v - min_[i]) / (max_[i] - min_[i]) * (nbins_[i]-1));
				}

				data_[i][k*nsamples_+j]++;
			}
		}
  }
}

void
HistogramView::copyTime() {
  QPtrListIterator<Matrix> it(tel_);
  int i;

  const Matrix &m = *it.toFirst();
  for(i=0; i<nsamples_; i++) {
    time_[i] = m(i, 0);
  }
}

void
HistogramView::setGraphVisible(int i) {
  for(int j=0; j<nvars_*2; j++) {
    curve(curves_[j])->setEnabled(false);
    g_ = -1;
  }
  if(0 <= i && i < nvars_) {
    graph_menu_->setItemChecked(i, true);
    curve(curves_[i])->setEnabled(graph_menu_->isItemChecked(nvars_));
    curve(curves_[i+nvars_])->setEnabled(graph_menu_->isItemChecked(nvars_+1));
    g_ = i;
  }
  updateInternals();
}

void
HistogramView::assignDataToCurves() {
  for(int i=0; i<nvars_; i++) {
    ((HistogramCurve*)curve(curves_[i]))->setRawMat(data_[i], nsamples_, nbins_[i], time_, min_[i], max_[i]);
    curve(curves_[i+nvars_])->setRawData(time_, avg_[i], nsamples_);
  }
}

void
HistogramView::updateInternals() {
  if(!zoomed_) {
    if(nsamples_ > 0 && nvars_ > 0) setAxisScale(xBottom, time_[0], time_[nsamples_-1]);

    if(g_ == -1) {
      setAxisScale(yLeft, 0.0, 1.0);
    } else {
      int m = 0;
      for(int i=0; i<nvars_; i++) {
	int tm = GenoDYN::roundToInt(max_[i] - min_[i]) + 1;
	if(tm > m) m = tm;
      }
      m = GenoDYN::min(m, nbins_[g_]);
      m = GenoDYN::min(m, 10);

      float min_y = floor(min_[g_]);
      if(absoluteScale() && min_y > 0.0) min_y = 0.0;
      setAxisScale(yLeft, min_y, ceil(max_[g_]));

      //setAxisScale(yLeft, 0.0, ceil(max_[g_]));
      setAxisMaxMajor(yLeft, m);
      setAxisMaxMinor(yLeft, 0);
      //setAxisScale(yLeft, floor(min_[g_]), ceil(max_[g_] + 0.9));
      //setAxisMaxMajor(yLeft, ceil(max_[g_] + 0.9) - floor(min_[g_]));
      //setAxisMaxMajor(yLeft, qwtMin(nbins_[g_], 5));
      //setAxisScale(yLeft, min_[g_], max_[g_]);
      //setAxisScale(yRight, 1, nbins_[g_]);
      //setAxisMaxMajor(yRight, nbins_[g_]);
      //setAxisMaxMinor(yRight, 0);
      //enableAxis(yRight);
    }
  }
  replot();
}

void
HistogramView::updateGraphMenu() {
  PlotView::updateGraphMenu();
  if(g_ != -1) {
    graph_menu_->setItemChecked(g_, true);
  }
}

void
HistogramView::graphMenu(int id) {
  if(id == nvars_) {
    bool s = curve(curves_[g_])->enabled();
    curve(curves_[g_])->setEnabled(!s);
    graph_menu_->setItemChecked(nvars_, !s);
  } else if(id == nvars_+1) {
    bool s = curve(curves_[g_+nvars_])->enabled();
    curve(curves_[g_+nvars_])->setEnabled(!s);
    graph_menu_->setItemChecked(nvars_+1, !s);
  } else if(id == nvars_+2) {
    bool ok;
    int mb = GenoDYN::roundToInt(max_[g_] - min_[g_]) + 1;
    int n = QInputDialog::getInteger(appname, tr("Enter number of bins [1, %1]:").arg(mb), nbins_[g_], 1, mb, 1, &ok, this);
    if(ok) {
      nbins_[g_] = n;
      arrangeDataIntoBins();
      assignDataToCurves();
    }
  } else if(id >= nvars_+100) {
    PlotView::graphMenu(id);
  } else {
    setGraphVisible(id);
  }
  updateInternals();
}

void
HistogramView::keyPressEvent(QKeyEvent *e) {
  if(g_ > 0 && e->text() == "n") {
    setGraphVisible((g_ + 1) % nvars_);
  } else if(g_ > 0 && e->text() == "p") {
    setGraphVisible((g_ == 0) ? nvars_ - 1 : g_ - 1);
  } else {
    e->ignore();
  }
}

void
HistogramView::plotMouseReleased(const QMouseEvent &e) {
  if(e.button() == Qt::LeftButton && zoom_) {
    zoom_ = false;
    zoomed_ = true;
    enableOutline(false);

    if(p1_ == e.pos()) return;

    // The following is based off of Qwt's bode example:
    
    // some shortcuts
    int axl= QwtPlot::yLeft, axr = QwtPlot::yRight, axb= QwtPlot::xBottom;
    // Don't invert any scales which aren't inverted
    int x1 = qwtMin(p1_.x(), e.pos().x());
    int x2 = qwtMax(p1_.x(), e.pos().x());
    int y1 = qwtMin(p1_.y(), e.pos().y());
    int y2 = qwtMax(p1_.y(), e.pos().y());
    
    // limit selected area to a minimum of 11x11 points
    int lim = 5 - (y2 - y1) / 2;
    if(lim > 0) {
      y1 -= lim;
      y2 += lim;
    }
    lim = 5 - (x2 - x1 + 1) / 2;
    if(lim > 0) {
      x1 -= lim;
      x2 += lim;
    }
    
    // Set fixed scales
    if(g_ != -1) {
      setAxisScale(axl, 
		   qwtLim((float)invTransform(axl,y1), min_[g_], max_[g_]),
		   qwtLim((float)invTransform(axl,y2), min_[g_], max_[g_]));
      setAxisScale(axr, 
		   qwtLim((float)invTransform(axr,y1), 1.f, (float)nbins_[g_]),
		   qwtLim((float)invTransform(axr,y2), 1.f, (float)nbins_[g_]));
    } else {
      setAxisScale(axl, 
		   invTransform(axl,y1),
		   invTransform(axl,y2));
      setAxisScale(axr, 
		   invTransform(axr,y1),
		   invTransform(axr,y2));
    }

    if(time_) {
      setAxisScale(axb, 
		   qwtLim((float)invTransform(axb, x1), (float)time_[0], (float)time_[nsamples_-1]),
		   qwtLim((float)invTransform(axb, x2), (float)time_[0], (float)time_[nsamples_-1]));
    } else {
      setAxisScale(axb, 
		   invTransform(axb, x1), invTransform(axb, x2));
    }

    replot(); 
  }
}

void
HistogramView::exportData(const QString &filename) {
  FILE *file = fopen((const char*)filename, "w");
  if(!file) {
    QMessageBox::critical(NULL, NULL, QString("Unable to export to ") + filename);
    return;
  }
#if 0
  int r, c;
  if(nc_ > 0) fprintf(file, "Time");
  for(c=1; c<nc_; c++) {
    if(curve(curves_[c-1])->enabled()) {
      if((c-1) < (int)labels_.count()) {
	fprintf(file, "\t%s", (const char*)labels_[c-1]);
      } else {
	fprintf(file, "\tGraph %d", c);
      }
    }
  }
  fputc('\n', file);
  for(r=0; r<nr_; r++) {
    for(c=0; c<nc_; c++) {
      if(c == 0) {
	fprintf(file, "%f", data_[c][r]);
      } else if(curve(curves_[c-1])->enabled()) {
	fprintf(file, "\t%f", data_[c][r]);
      }
    }
    fputc('\n', file);
  }
#endif
  fclose(file);
}
