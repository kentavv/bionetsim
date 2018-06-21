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

#include <qlistview.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qtabwidget.h>

#include "concentration_dialog.h"
#include "line_graph_view.h"
#include "phase_plane_view.h"
#include "plot_dialog.h"

ConcentrationDialog::ConcentrationDialog(QWidget *parent)
  : PlotDialog(parent, NULL, false, Qt::WStyle_MinMax),
    data_(NULL),
    nr_(0), nc_(0)
{
  tab_ = new QTabWidget(split_);

  sv_ = new QScrollView(tab_);
  sv_->setMinimumSize(mgw, mgh);
  wl_.setAutoDelete(true);

  phase_ = new PhasePlaneView(tab_);

  tab_->addTab(sv_, "Concentration v. Time");
  tab_->addTab(phase_, "Concentration v. Concentration");
}

ConcentrationDialog::~ConcentrationDialog() {
  if(data_) {
    for(int i=0; i<nc_; i++) {
      delete data_[i];
    }
    delete[] data_;
  }
}

void
ConcentrationDialog::setData(const Result &r) {
  int i, j;
  if(data_) {
    for(i=0; i<nc_; i++) {
      delete[] data_[i];
    }
    delete[] data_;
    data_ = NULL;
  }

  bool reset = (labels_ != r.column_labels || nc_ != r.m.ncolumns());

  nr_ = r.m.nrows();
  nc_ = r.m.ncolumns();
  data_ = new double*[nc_];
  for(i=0; i<nc_; i++) {
    data_[i] = new double[nr_];
    for(j=0; j<nr_; j++) {
      data_[i][j] = r.m(j, i);
    }
  }

  if(reset) {
    labels_ = r.column_labels;
    wl_.clear();
    sv_->resizeContents(1, 1);
    fullscreen_ = NULL;
    for(i=1; i<nc_; i++) {
      LineGraphView *gv = new LineGraphView(sv_);
      gv->resize(mgw, mgh);
      gv->setRawData(data_, nr_, nc_, labels_);
      gv->setAllVisible(false);
      gv->setGraphVisible(i-1, true);
      gv->setTitle(labels_[i-1]);
      sv_->addChild(gv);
      gv->show();
      wl_.append(gv);
      connect(gv, SIGNAL(fullScreenRequested(PlotView*)), SLOT(fullScreen(PlotView*)));
    }

    phase_->clear();
    phase_->setRawData(data_, nr_, nc_, labels_);
  } else {
    for(i=1; i<nc_; i++) {
      LineGraphView *gv = (LineGraphView*)wl_.at(i-1);
      gv->setRawData(data_, nr_, nc_, labels_);
    }

    phase_->setRawData(data_, nr_, nc_, labels_);
  }
}

void
ConcentrationDialog::print() {
  if(tab_->currentPage() == sv_) {
    PlotDialog::print();
  } else if(tab_->currentPage() == phase_) {
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setPageSize(QPrinter::Letter);
    if(printer.setup(this)) {
      phase_->print(printer);
    }
  }
}

void
ConcentrationDialog::genPixmap(QPixmap &pix) {
  if(tab_->currentPage() == sv_) {
    PlotDialog::genPixmap(pix);
  } else if(tab_->currentPage() == phase_) {
    pix.resize(phase_->size());
    pix.fill(white);
    QPainter p(&pix);
    
    phase_->print(&p, QRect(0, 0, phase_->width(), phase_->height()));
  }
}
