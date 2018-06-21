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
#include <qfiledialog.h>
#include <qhbox.h>
#include <qimage.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qsplitter.h>

#include "plot_dialog.h"
#include "plot_view.h"

// Height and width of the mini-graphs used in the collage
const int PlotDialog::mgw = 200;
const int PlotDialog::mgh = 200;

PlotDialog::PlotDialog(QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f),
    fullscreen_(NULL)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  split_ = new QSplitter(this);
  split_->setOrientation(QSplitter::Horizontal);

  lv_ = new ListViewHelper(split_);
  lv_->addColumn("Name");

  QHBox *hbox;
  hbox = new QHBox(this);
  QPushButton *print = new QPushButton("Print", hbox);
  QPushButton *ss = new QPushButton("Save Snapshot", hbox);
  QPushButton *cc = new QPushButton("Copy to Clipboard", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  hbox->setFixedHeight(hbox->sizeHint().height());

  connect(print, SIGNAL(clicked()), SLOT(print()));
  connect(ss, SIGNAL(clicked()), SLOT(saveSnapshot()));
  connect(cc, SIGNAL(clicked()), SLOT(copyToClipboard()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(lv_, SIGNAL(selectionChanged(QListViewItem*)), SLOT(graphSelected(QListViewItem*)));
  connect(lv_, SIGNAL(resized()), SLOT(updateGraphLayout()));
}

PlotDialog::~PlotDialog() {
}

void
PlotDialog::clear() {
}

void
PlotDialog::print() {
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setPageSize(QPrinter::Letter);
  if(printer.setup(this)) {
    if(!fullscreen_) {
      QPaintDeviceMetrics m(&printer);
      QPainter pp(&printer);
      int n = wl_.count(), nh = 4, nv, s;
      if(m.width() < m.height()) {
	s = m.width() / nh;
	nv = m.height() / s;
      } else {
	s = m.height() / nh;
	nv = m.width() / s;
      }
      for(int i=0, j=0; i<n; i++) {
	PlotView *pv = ((PlotView*)wl_.at(i));
	pv->print(&pp, QRect(s * (j % nh), s * (j / nh), s, s));
	if(++j == nh*nv && i != n-1) {
	  printer.newPage();
	  j = 0;
	}
      }
    } else {
      fullscreen_->print(printer);
    }
  }
}

void
PlotDialog::genPixmap(QPixmap &pix) {
  if(!fullscreen_) {
    pix.resize(sv_->contentsWidth(), sv_->contentsHeight());
    pix.fill(white);
    QPainter p(&pix);

    int n = wl_.count();
    for(int i=0; i<n; i++) {
      PlotView *pv = ((PlotView*)wl_.at(i));
      pv->print(&p, pv->geometry());
    }
  } else {
    pix.resize(fullscreen_->size());
    pix.fill(white);
    QPainter p(&pix);
    
    fullscreen_->print(&p, QRect(0, 0, fullscreen_->width(), fullscreen_->height()));
  }
}

void
PlotDialog::saveSnapshot() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "PNG Files (*.png);;All Files (*)", this);
  if(fn.isEmpty()) return;

  QPixmap pix;
  genPixmap(pix);

  QImage img = pix.convertToImage();
  img.save(fn, "PNG", 0);
}

void
PlotDialog::copyToClipboard() {
  QClipboard *cb = QApplication::clipboard();
  if(!cb) return;

  QPixmap pix;
  genPixmap(pix);

  cb->setPixmap(pix, QClipboard::Clipboard);
}

void
PlotDialog::updateGraphLayout() {
  int n = wl_.count();
  if(fullscreen_) {
    for(int i=0; i<n; i++) {
      wl_.at(i)->hide();
    }
    fullscreen_->resize(sv_->contentsRect().size());
    sv_->resizeContents(sv_->viewport()->width(), sv_->viewport()->height());
    sv_->moveChild(fullscreen_, 0, 0);
    fullscreen_->show();
  } else {
    //printf("%d %d %d\n", sv_->width(), sv_->visibleWidth(), sv_->verticalScrollBar()->width());
    int nw = sv_->visibleWidth() / mgw;
    if(nw == 0) nw = 1;
    sv_->resizeContents(nw * mgw, (n / nw + (n % nw == 0 ? 0 : 1)) * mgh);
    for(int i=0; i<n; i++) {
      wl_.at(i)->show();
      sv_->moveChild(wl_.at(i), (i%nw)*mgw, (i/nw)*mgh);
    }
  }
}

void
PlotDialog::resizeEvent(QResizeEvent *e) {
  QDialog::resizeEvent(e);
  updateGraphLayout();
}

void
PlotDialog::show() {
  QDialog::show();
  updateGraphLayout();
}

void
PlotDialog::clearResults() {
	results_.clear();
}

void
PlotDialog::setResults(const QValueList<Result> &results) {
  lv_->clear();

  QValueList<Result>::const_iterator i;
  for(i = results.begin(); i != results.end(); ++i) {
    new QListViewItem(lv_, tr((*i).env));
  }

  results_ = results;

  setGraph(0);

  if(results.empty()) clear();
}

void
PlotDialog::fullScreen(PlotView *v) {
  if(fullscreen_) {
    fullscreen_ = NULL;
    v->resize(mgw, mgh);
    v->setAxisTitle(PlotView::xBottom, "");
    v->setAxisTitle(PlotView::yLeft, "");
  } else {
    fullscreen_ = v;
    v->setAxisTitle(PlotView::xBottom, "Time");
    v->setAxisTitle(PlotView::yLeft, "Concentration");
  }
  updateGraphLayout();
}

void
PlotDialog::graphSelected(QListViewItem *item) {
  QString env = item->text(0);
  int j=0;

  QValueList<Result>::const_iterator i;
  for(i = results_.begin(); i != results_.end(); ++i) {
    if((*i).env == env) {
      setData(results_[j]);
      break;
    }
    j++;
  }
}

void
PlotDialog::setGraph(int j) {
  QListViewItemIterator it(lv_);
  while(it.current() && j >= 0) {
    if(j==0) lv_->setSelected(it.current(), true);
    j--;
    ++it;
  }
}
