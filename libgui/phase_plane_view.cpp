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

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qprinter.h>

#include "qwt_plot.h"
#include "qwt_math.h"

#include "lgv_config_dialog.h"
#include "phase_plane_view.h"

//----------------------------------------------------------------------

PhasePlaneView::PhasePlaneView(QWidget *parent)
  : QwtPlot(parent),
    cdata_(NULL),
    data_(NULL),
    nr_(0), nc_(0),
    x_(-1), y_(-1),
    curve_(-1),
    config_(NULL),
    zoom_(false),
    zoomed_(false)
{
  markers_[0] = -1;
  markers_[1] = -1;

  x_menu_ = new QPopupMenu(this);
  x_menu_->setCheckable(true);

  y_menu_ = new QPopupMenu(this);
  y_menu_->setCheckable(true);

  graph_menu_ = new QPopupMenu(this);
  graph_menu_->setCheckable(true);
  graph_menu_->setEnabled(false);
  graph_menu_->insertItem("X Axis", x_menu_);
  graph_menu_->insertItem("Y Axis", y_menu_);
  
  setCanvasBackground(white);

  setOutlinePen(black);
  
  setOutlinePen(black);
  enableOutline(false);

  connect(x_menu_, SIGNAL(activated(int)), SLOT(xMenu(int)));
  connect(y_menu_, SIGNAL(activated(int)), SLOT(yMenu(int)));
  connect(graph_menu_, SIGNAL(activated(int)), SLOT(graphMenu(int)));
  connect(x_menu_, SIGNAL(aboutToShow()), SLOT(updateXMenu()));
  connect(y_menu_, SIGNAL(aboutToShow()), SLOT(updateYMenu()));
  connect(this, SIGNAL(plotMouseMoved(const QMouseEvent&)),
	  SLOT(plotMouseMoved(const QMouseEvent&)));
  connect(this, SIGNAL(plotMousePressed(const QMouseEvent &)),
	  SLOT(plotMousePressed(const QMouseEvent&)));
  connect(this, SIGNAL(plotMouseReleased(const QMouseEvent &)),
	  SLOT(plotMouseReleased(const QMouseEvent&)));

  setFocusPolicy(QWidget::StrongFocus);
  setFocus();
}

PhasePlaneView::~PhasePlaneView() {
  clear();
}

void
PhasePlaneView::clear() {
  QwtPlot::clear();
  curve_ = -1;
  markers_[0] = -1;
  markers_[1] = -1;
  if(cdata_) {
    for(int i=0; i<nc_; i++) {
      delete[] cdata_[i];
    }
    delete[] cdata_;
    cdata_ = NULL;
  }
  data_ = NULL;
  x_ = -1;
  y_ = -1;
  x_menu_->clear();
  y_menu_->clear();
  graph_menu_->setEnabled(false);
  labels_.clear();
}

void
PhasePlaneView::setData(const Matrix &data) {
  QStringList tmp;
  setData(data, tmp);
}

void
PhasePlaneView::setData(const Matrix &data, const QStringList &labels) {
  int nr = data.nrows();
  int nc = data.ncolumns();
  double **pcdata = NULL;
  if(nc > 0) {
    pcdata = new double*[nc];
    for(int i=0; i<nc; i++) {
      pcdata[i] = new double[nr];
      for(int j=0; j<nr; j++) {
	pcdata[i][j] = data(j, i);
      }
    }
  }
  setRawData(pcdata, nr, nc, labels);
}

void
PhasePlaneView::setRawData(const double * const *data, int nr, int nc) {
  QStringList tmp;
  setRawData(data, nr, nc, tmp);
}

void
PhasePlaneView::setRawData(const double * const *data, int nr, int nc, const QStringList &labels) {
  bool reset = (labels_ != labels || nc_ != nc);

  if(reset) {
    clear();
    labels_ = labels;
  }
  
  nr_ = nr;
  nc_ = nc;
  data_ = data;
  
  if(nc_ == 0 || nr_ == 0 || !data_) return;

  graph_menu_->setEnabled(true);
  
  if(reset) {
    curve_ = insertCurve("Phase-Plane Plot");
    markers_[0] = insertMarker("t0");
    markers_[1] = insertMarker("te");
    QwtSymbol ssym(QwtSymbol::Diamond, blue, blue, QSize(11, 11));
    QwtSymbol esym(QwtSymbol::Diamond, red, red, QSize(11, 11));
    marker(markers_[0])->setSymbol(ssym);
    marker(markers_[1])->setSymbol(esym);
    marker(markers_[0])->setLabelAlignment(AlignTop);
    marker(markers_[1])->setLabelAlignment(AlignTop);
    //curve(curve_)->setStyle(QwtCurve::Spline, QwtCurve::Parametric);
    // 0 has a pixel width of 1, 1 has a pixel width of 1 but is more accurate
    int line_width = 2;
    curve(curve_)->setPen(QPen(black, line_width));
    x_menu_->insertItem("Time", 0);
    y_menu_->insertItem("Time", 0);
  }
  for(int i=0; i<nc_-1; i++) {
    if(reset) {
      if(i < (int)labels.count()) {
	x_menu_->insertItem(labels[i], i+1);
	y_menu_->insertItem(labels[i], i+1);
      } else {
	QString str;
	str.sprintf("Graph %d", i+1);
	x_menu_->insertItem(str, i+1);
	y_menu_->insertItem(str, i+1);
      }
    }
  }

  if(reset) {
    graph_menu_->insertSeparator();
    //graph_menu_->insertItem("Configure", nc_+2);
    graph_menu_->insertItem("Export data", nc_+3);
    graph_menu_->insertItem("Print", nc_+4);
    graph_menu_->insertItem("Save snapshot", nc_+5);
    graph_menu_->insertItem("Copy to clipboard", nc_+6);
    graph_menu_->insertItem("Zoom out", nc_+7);
  }

  if(x_ < 0) x_ = 0;
  if(y_ < 0) y_ = 0;
  if(x_ >= nc_) x_ = nc_-1;
  if(y_ >= nc_) y_ = nc_-1;
  
  setGraphs(x_, y_);
}

void
PhasePlaneView::setGraphs(int x, int y) {
  if(x < 0 || nc_-1 < x ||
     y < 0 || nc_-1 < y) return;
  x_ = x;
  y_ = y;
  setAxisTitle(xBottom, label(x_));
  setAxisTitle(yLeft, label(y_));
  updateInternals();
}

void
PhasePlaneView::updateInternals() {
  if(!zoomed_) {
    if(0 <= x_ && x_ < nc_ &&
       0 <= y_ && y_ < nc_) {
      setCurveRawData(curve_, (double*)data_[x_], (double*)data_[y_], nr_);
      marker(markers_[0])->setXValue(data_[x_][0]);
      marker(markers_[0])->setYValue(data_[y_][0]);
      marker(markers_[1])->setXValue(data_[x_][nr_-1]);
      marker(markers_[1])->setYValue(data_[y_][nr_-1]);
      
      double min_x = DBL_MAX, max_x = -DBL_MAX;
      double min_y = DBL_MAX, max_y = -DBL_MAX;
      for(int i=0; i<nr_; i++) {
	if(data_[x_][i] < min_x) min_x = data_[x_][i];
	if(data_[x_][i] > max_x) max_x = data_[x_][i];
	if(data_[y_][i] < min_y) min_y = data_[y_][i];
	if(data_[y_][i] > max_y) max_y = data_[y_][i];
      }
      setAxisScale(xBottom, min_x, max_x);
      setAxisScale(yLeft, min_y, max_y);
    } else {
      setAxisScale(yLeft, 0.0, 1.0);
    }
  }
  replot();
}

void
PhasePlaneView::draw(QPainter *p) {
  print(p, QRect(0, 0, width(), height()));
}

void
PhasePlaneView::displayGraphMenu(const QMouseEvent &e) {
  if(e.button() == Qt::RightButton) {
    graph_menu_->popup(QCursor::pos());
  }
}

void
PhasePlaneView::xMenu(int id) {
  x_ = id;
  if(y_ == -1) y_ = x_;
  setGraphs(x_, y_);
}

void
PhasePlaneView::yMenu(int id) {
  y_ = id;
  if(x_ == -1) x_ = y_;
  setGraphs(x_, y_);
}

void
PhasePlaneView::graphMenu(int id) {
  //printf("%d\n", id);
  if(id == nc_+2) {
    //if(!config_) config_ = new LGVConfigDialog(this, this);
    //config_->show();
  } else if(id == nc_+3) {
    if(x_ == -1 || y_ == -1) return;
    QString fn = QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", this);
    if(fn.isEmpty()) return;
    
    FILE *file = fopen((const char*)fn, "w");
    if(!file) {
      QMessageBox::critical(NULL, NULL, QString("Unable to export to ") + fn);
      return;
    }
    int r;
    fprintf(file, "%s", (const char*)label(x_));
    fprintf(file, "%s", (const char*)label(y_));
    fputc('\n', file);
    for(r=0; r<nr_; r++) {
      fprintf(file, "%f\t%f\n", data_[x_][r], data_[y_][r]);
    }
    fclose(file);
  } else if(id == nc_+4) {
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setPageSize(QPrinter::Letter);
    if(printer.setup(this)) {
      print(printer);
    }
  } else if(id == nc_+5) {
    QString fn = QFileDialog::getSaveFileName(QString::null, "PNG Files (*.png);;All Files (*)", this);
    if(fn.isEmpty()) return;

    QPixmap pix(size());
    pix.fill(white);
    {
      QPainter p(&pix);
      print(&p, QRect(0, 0, width(), height()));
    }
    QImage img = pix.convertToImage();
    if(!img.save(fn, "PNG", 0)) {
      QMessageBox::critical(NULL, NULL, QString("Unable to save to ") + fn);
    }
  } else if(id == nc_+6) {
    QClipboard *cb = QApplication::clipboard();
    if(!cb) return;

    QPixmap pix(size());
    pix.fill(white);
    {
      QPainter p(&pix);
      print(&p, QRect(0, 0, width(), height()));
    }

    cb->setPixmap(pix, QClipboard::Clipboard);
  } else if(id == nc_+7) {
    zoom_ = false;
    zoomed_ = false;
  }
  updateInternals();
}

void
PhasePlaneView::updateXMenu() {
  for(int i=0; i<nc_; i++) {
    x_menu_->setItemChecked(i, false);
  }
  if(x_ != -1) {
    x_menu_->setItemChecked(x_, true);
  }
}

void
PhasePlaneView::updateYMenu() {
  for(int i=0; i<nc_; i++) {
    y_menu_->setItemChecked(i, false);
  }
  if(y_ != -1) {
    y_menu_->setItemChecked(y_, true);
  }
}

QString
PhasePlaneView::label(int id) const {
  QString str;
  if(id == 0) {
    str = "Time";
  } else if(id <= (int)labels_.count()) {
    str = labels_[id-1];
  } else {
    str.sprintf("Graph %d", id);
  }
  return str;
}

void
PhasePlaneView::keyPressEvent(QKeyEvent *e) {
  if(x_ != -1 && y_ != -1) {
    if(e->text() == "x") {
      x_ = (x_ + 1) % nc_;
      setGraphs(x_, y_);
    } else if(e->text() == "X") {
      x_--;
      if(x_ < 0) x_ = nc_ - 1;
      setGraphs(x_, y_);
    } else if(e->text() == "y") {
      y_ = (y_ + 1) % nc_;
      setGraphs(x_, y_);
    } else if(e->text() == "Y") {
      y_--;
      if(y_ < 0) y_ = nc_ - 1;
      setGraphs(x_, y_);
    } else {
      e->ignore();
    }
  } else {
    e->ignore();
  }
}

void
PhasePlaneView::plotMouseMoved(const QMouseEvent &) {
}

void
PhasePlaneView::plotMousePressed(const QMouseEvent &e) {
  if(e.button() == Qt::RightButton && !zoom_) {
    graph_menu_->popup(QCursor::pos());
  } else if(e.button() == Qt::LeftButton) {
    enableOutline(true);
    zoom_ = true;
    setOutlineStyle(Qwt::Rect);
    p1_ = e.pos();
  }
}

void
PhasePlaneView::plotMouseReleased(const QMouseEvent &e) {
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
    
    //showInfo(cursorInfo);
    setOutlineStyle(Qwt::Triangle);

    zoom_ = false;
    zoomed_ = true;
    enableOutline(false);
  }
}
