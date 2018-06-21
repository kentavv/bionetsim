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

#include <stdio.h>
#include <math.h>

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#include "qwt_plot.h"
#include "qwt_math.h"
#include "qwt_plot_canvas.h"

#include "lgv_config_dialog.h"
#include "line_graph_view.h"
#include "textviewer.h"

QColor LineGraphView::colors[13] = {
  QColor(230,  10,  10),
  QColor( 20,  90, 255),
  QColor(150/*50*/,  50, 170),
  QColor(0/*235*/, 235, 235),
  QColor(0/*200*/, 100/*200*/, 200),
  QColor(130, 130, 210),
  QColor(230, 230,   0),
  QColor(250, 150,   0),
  QColor(  0, 220, 220),
  QColor( 15, 130,  15),
  QColor(180,  90, 180),
  QColor(220, 150, 130),
  QColor(190, 160, 110)
};

LGVTip::LGVTip(QWidget *parent)
  : QToolTip(parent)
{}

void
LGVTip::maybeTip(const QPoint &p) {
  if(!parentWidget()->inherits("LineGraphView"))
    return;

  QRect r;
  QString str;
  
  ((LineGraphView*)parentWidget())->tip(p, r, str);
  if(!r.isValid()) return;
  
  tip(r, str);
}

//----------------------------------------------------------------------

LineGraphView::LineGraphView(QWidget *parent)
  : PlotView(parent),
    cdata_(NULL),
    data_(NULL),
    nr_(0), nc_(0),
    config_(NULL),
    tv_(NULL)
{
  canvas()->setMouseTracking(true);
  
  connect(this, SIGNAL(plotMouseMoved(const QMouseEvent&)),
	  SLOT(plotMouseMoved(const QMouseEvent&)));
  connect(this, SIGNAL(plotMouseReleased(const QMouseEvent &)),
	  SLOT(plotMouseReleased(const QMouseEvent&)));

  tip_ = new LGVTip(this);
}

LineGraphView::~LineGraphView() {
  clear();
  clearCopiedData();
  delete tip_;
}

void
LineGraphView::clear() {
  QwtPlot::clear();
  clearCopiedData();
  if(curves_) delete[] curves_;
  curves_ = NULL;  
  data_ = NULL;
  graph_menu_->clear();
  graph_menu_->setEnabled(false);
  labels_.clear();
  if(tv_) tv_->clear();
}

void
LineGraphView::clearCopiedData() {
  if(cdata_) {
    for(int i=0; i<nc_; i++) {
      delete[] cdata_[i];
    }
    delete[] cdata_;
    cdata_ = NULL;
  }
}

void
LineGraphView::setData(const Matrix &data) {
  QStringList tmp;
  setData(data, tmp);
}

void
LineGraphView::setData(const Matrix &data, const QString &label) {
  QStringList tmp;
  tmp.append(label);
  setData(data, tmp);
}

void
LineGraphView::setData(const Matrix &data, const QStringList &labels) {
  int nr = data.nrows();
  int nc = data.ncolumns();
  double **cdata = NULL;
  if(nc > 0) {
    cdata = new double*[nc];
    for(int i=0; i<nc; i++) {
      cdata[i] = new double[nr];
      for(int j=0; j<nr; j++) {
	cdata[i][j] = data(j, i);
      }
    }
  }
  setRawData(cdata, nr, nc, labels);
  cdata_ = cdata;
}

void
LineGraphView::setData(const Result &result) {
  setData(result.m, result.column_labels);
}

void
LineGraphView::setRawData(const double * const *data, int nr, int nc) {
  QStringList tmp;
  setRawData(data, nr, nc, tmp);
}

void
LineGraphView::setRawData(const double * const *data, int nr, int nc, const QStringList &labels) {
  bool reset = (labels_ != labels || nc_ != nc);

  if(tv_) tv_->hide();
  
  if(reset) {
    clear();
    labels_ = labels;
  }
  if(cdata_) {
    clearCopiedData();
  }
  
  nr_ = nr;
  nc_ = nc;
  data_ = data;
  nvars_ = nc_-1;
  
  if(nc_ == 0 || nr_ == 0 || !data_) return;

  graph_menu_->setEnabled(true);
  
  if(reset) {
    curves_ = new long[nvars_];
  }
  for(int i=0; i<nvars_; i++) {
    if(reset) {
      if(i < (int)labels.count()) {
	curves_[i] = insertCurve(labels[i]);
	graph_menu_->insertItem(labels[i], i);
      } else {
	QString str;
	str.sprintf("Graph %d", i+1);
	graph_menu_->insertItem(str, i);
	curves_[i] = insertCurve(str);
      }
      // 0 has a pixel width of 1, 1 has a pixel width of 1 but is more accurate
      int line_width = 2;
      curve(curves_[i])->setPen(QPen(colors[i % 13], line_width));
    }
    setCurveRawData(curves_[i], (double*)data_[0], (double*)data_[i+1], nr_);
  }

  if(reset) {
    if(nvars_ >= 2) {
      graph_menu_->insertSeparator();
      graph_menu_->insertItem("All off",        nvars_);
      graph_menu_->insertItem("All on",         nvars_+1);
      graph_menu_->insertItem("All complement", nvars_+2);
    }

    graph_menu_->insertSeparator();
    //graph_menu_->insertItem("Configure", nvars_+3);
    graph_menu_->insertItem("View data", nvars_+4);
    graph_menu_->insertSeparator();

    fillGraphMenu();
  }
  
  updateInternals();
}

bool
LineGraphView::isGraphVisible(int i) {
  if(0 <= i && i < nvars_) {
    return curve(curves_[i])->enabled();
  }
  return false;
}

void
LineGraphView::setGraphVisible(int i, bool v) {
  if(0 <= i && i < nvars_) {
    curve(curves_[i])->setEnabled(v);
  }
  updateInternals();
}

void
LineGraphView::setAllVisible(bool v) {
  for(int i=0; i<nvars_; i++) {
    curve(curves_[i])->setEnabled(v);
  }
  updateInternals();
}

void
LineGraphView::updateInternals() {
  if(!zoomed_) {
    if(nc_ > 0 && nr_ > 0) setAxisScale(xBottom, data_[0][0], data_[0][nr_-1]);

    if(nGraphsVisible() == 0) {
      setAxisScale(yLeft, 0.0, 1.0);
    } else {
      double min_y = DBL_MAX, max_y = -DBL_MAX;
      for(int i=0; i<nvars_; i++) {
	if(absoluteScale() || curve(curves_[i])->enabled()) {
	  double min = curve(curves_[i])->minYValue();
	  double max = curve(curves_[i])->maxYValue();
	  if(min < min_y) min_y = min;
	  if(max > max_y) max_y = max;
	}
      }
      if(absoluteScale() && min_y > 0.0) min_y = 0.0;
      setAxisScale(yLeft, min_y, max_y);
    }
  }
  replot();
}

int
LineGraphView::nGraphsVisible() const {
  int i, n;
  for(n=0, i=0; i<nvars_; i++) {
    if(curve(curves_[i])->enabled()) n++;
  }
  return n;
}

void
LineGraphView::graphMenu(int id) {
  if(id == nvars_) {
    for(int i=0; i<nvars_; i++) {
      curve(curves_[i])->setEnabled(false);
    }
  } else if(id == nvars_+1) {
    for(int i=0; i<nvars_; i++) {
      curve(curves_[i])->setEnabled(true);
    }
  } else if(id == nvars_+2) {
    for(int i=0; i<nvars_; i++) {
      curve(curves_[i])->setEnabled(!curve(curves_[i])->enabled());
    }
  } else if(id == nvars_+3) {
    if(!config_) config_ = new LGVConfigDialog(this, this);
    config_->show();
  } else if(id == nvars_+4) {
    QString txt, str;
    int r, c;
    if(nc_ > 0) txt += "Time            ";
    for(c=1; c<nc_; c++) {
      if(curve(curves_[c-1])->enabled()) {
	if((c-1) < (int)labels_.count()) {
	  str.sprintf("\t%-15s", (const char*)labels_[c-1]);
	} else {
	  str.sprintf("\tGraph %d", c);
	}
	txt += str;
      }
    }
    txt += '\n';
    for(r=0; r<nr_; r++) {
      for(c=0; c<nc_; c++) {
	if(c == 0) {
	  str.sprintf("%f", data_[c][r]);
	  txt += str;
	} else if(curve(curves_[c-1])->enabled()) {
	  str.sprintf("\t%f", data_[c][r]);
	  txt += str;
	}
      }
      txt += '\n';
    }
    if(!tv_) {
      tv_ = new TextViewer(this);
    }
    tv_->setText(txt);
    tv_->show();
  } else if(id >= nvars_+100) {
    PlotView::graphMenu(id);
  } else {
    curve(curves_[id])->setEnabled(!curve(curves_[id])->enabled());
  }

  int n = nGraphsVisible();

  if(n <= 0 || n > 1) {
    setTitle(" ");
  } else {
    for(int i=0; i<nvars_; i++) {
      if(curve(curves_[i])->enabled()) {
	setTitle(labels_[i]);
      }
    }
  }

  updateInternals();
}

void
LineGraphView::plotMouseMoved(const QMouseEvent &) {
}

void
LineGraphView::plotMouseReleased(const QMouseEvent &e) {
  if(e.button() == Qt::LeftButton && zoom_) {
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
    setAxisScale(axl, 
		 invTransform(axl,y1), invTransform(axl,y2));
    setAxisScale(axr, 
		 invTransform(axr,y1), invTransform(axr,y2));
    if(nc_ > 0 && nr_ > 0) {
      setAxisScale(axb, 
		   qwtMax(data_[0][0], invTransform(axb,x1)),
		   qwtMin(data_[0][nr_-1], invTransform(axb,x2)));
    } else {
      setAxisScale(axb, 
		   invTransform(axb,x1), invTransform(axb,x2));
    }
    replot();
    
    zoom_ = false;
    zoomed_ = true;
    enableOutline(false);
  }
}

void
LineGraphView::tip(QPoint p, QRect &r, QString &tip_str) const {
  QString str;

  double px = invTransform(xBottom, p.x());
  double py = invTransform(yLeft, p.y());
  str.sprintf("X=%g, Y=%g", px, py);
  tip_str += str;
  
  for(int i=0; i<nc_-1; i++) {
    double min_ss = DBL_MAX;
    int min_j = 0;
    if(curve(curves_[i])->enabled()) {
      for(int j=0; j<nr_; j++) {
	double dx = data_[0][j] - px;
	double dy = data_[i+1][j] - py;
	double ss = dx*dx + dy*dy;
	if(min_ss > ss) {
	  min_ss = ss;
	  min_j = j;
	}
      }
      str.sprintf("\n%s X=%g Y=%g ss=%g",
		  (const char*)curve(curves_[i])->title(),
		  data_[0][min_j], data_[i+1][min_j], min_ss);
      tip_str += str;
    }
  }
  
  r = QRect(QPoint(p.x(), p.y()), QPoint(p.x()+1, p.y()+1));
}

void
LineGraphView::exportData(const QString &filename) {
  FILE *file = fopen((const char*)filename, "w");
  if(!file) {
    QMessageBox::critical(NULL, NULL, QString("Unable to export to ") + filename);
    return;
  }
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
  fclose(file);
}
