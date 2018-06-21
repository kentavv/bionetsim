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

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpopupmenu.h>

//#define USE_ZOOMER

#ifdef USE_ZOOMER
#include "qwt/examples/realtime_plot/scrollzoomer.h"
#endif

#include "plot_view.h"

PlotView::PlotView(QWidget *parent)
  : QwtPlot(parent),
    nvars_(0),
    curves_(NULL),
    zoom_(false),
    zoomed_(false),
    absolute_scale_(true)
{
  graph_menu_ = new QPopupMenu(this);
  graph_menu_->setCheckable(true);
  graph_menu_->setEnabled(false);
  
  setCanvasBackground(white);

#ifdef USE_ZOOMER
  ScrollZoomer *zoomer = new ScrollZoomer(canvas());
  zoomer->setRubberBandPen(QPen(Qt::black, 0, Qt::DotLine));
  zoomer->setCursorLabelPen(QPen(Qt::black));
#else
  setOutlinePen(black);
  enableOutline(false);
  setOutlineStyle(Qwt::Rect);
#endif
  
  connect(graph_menu_, SIGNAL(activated(int)), SLOT(graphMenu(int)));
  connect(graph_menu_, SIGNAL(aboutToShow()), SLOT(updateGraphMenu()));
  connect(this, SIGNAL(plotMousePressed(const QMouseEvent&)),
	  SLOT(plotMousePressed(const QMouseEvent&)));
}

PlotView::~PlotView() {
}

void
PlotView::draw(QPainter *p) {
  print(p, QRect(0, 0, width(), height()));
}

void
PlotView::plotMousePressed(const QMouseEvent &e) {
  if(e.button() == Qt::RightButton && !zoom_) {
    graph_menu_->popup(QCursor::pos());
  } else if(e.button() == Qt::LeftButton) {
#ifndef USE_ZOOMER
    enableOutline(true);
    zoom_ = true;
    p1_ = e.pos();
#endif
  }
}

void
PlotView::graphMenu(int id) {
  if(id == nvars_+100) {
    QString fn = QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", this);
    if(fn.isEmpty()) return;
    exportData(fn);
  } else if(id == nvars_+101) {
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setPageSize(QPrinter::Letter);
    if(printer.setup(this)) {
      print(printer);
    }
  } else if(id == nvars_+102) {
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
  } else if(id == nvars_+103) {
    QClipboard *cb = QApplication::clipboard();
    if(!cb) return;

    QPixmap pix(size());
    pix.fill(white);
    {
      QPainter p(&pix);
      print(&p, QRect(0, 0, width(), height()));
    }

    cb->setPixmap(pix, QClipboard::Clipboard);
  } else if(id == nvars_+104) {
    zoom_ = false;
    zoomed_ = false;
  } else if(id == nvars_+105) {
    emit fullScreenRequested(this);
    graph_menu_->setItemChecked(nvars_+105, !graph_menu_->isItemChecked(nvars_+105));
  } else if(id == nvars_+106) {
    bool v = !absoluteScale();
    graph_menu_->setItemChecked(nvars_+106, v);
    setAbsoluteScale(v);
  }
}

void
PlotView::fillGraphMenu() {
  graph_menu_->insertItem("Export data", nvars_+100);
  graph_menu_->insertItem("Print", nvars_+101);
  graph_menu_->insertItem("Save snapshot", nvars_+102);
  graph_menu_->insertItem("Copy to clipboard", nvars_+103);
  graph_menu_->insertItem("Zoom out", nvars_+104);
  graph_menu_->insertItem("Full screen", nvars_+105);
  graph_menu_->insertItem("Absolute Scale", nvars_+106);
  graph_menu_->setItemChecked(nvars_+106, absolute_scale_);
}

void
PlotView::updateGraphMenu() {
  for(int i=0; i<nvars_; i++) {
    graph_menu_->setItemChecked(i, curve(curves_[i])->enabled());
  }
}

bool
PlotView::absoluteScale() {
  return absolute_scale_;
}

void
PlotView::setAbsoluteScale(bool v) {
  absolute_scale_ = v;
  updateInternals();
}

