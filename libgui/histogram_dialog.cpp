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

#include <qlistview.h>
#include <qptrlist.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qvaluevector.h>

#include "histogram_dialog.h"
#include "histogram_view.h"
#include "line_graph_view.h"
#include "log.h"
#include "math_utils.h"
#include "matrix.h"
#include "plot_dialog.h"

HistogramDialog::HistogramDialog(QWidget *parent)
  : PlotDialog(parent, NULL, false, Qt::WStyle_MinMax),
    nvars_(0)
{
  sv_ = new QScrollView(split_);
  sv_->setMinimumSize(mgw, mgh);
  wl_.setAutoDelete(true);
}

HistogramDialog::~HistogramDialog() {
  clear();
}

void
HistogramDialog::clear() {
  wl_.clear();
  sv_->resizeContents(1, 1);
  fullscreen_ = NULL;
  nvars_ = 0;
}

void
HistogramDialog::setData(const Result &result) {
  const QPtrList<Matrix> &tel    = result.tel;
  const QStringList &labels      = result.column_labels;
  const QValueVector<bool> &cont = result.cont;

  if(tel.isEmpty()) {
    pneDebug("clear");
    clear();
    return;
  }

	// XXX
  bool reset = true || (labels_ != labels || 
		nvars_ != tel.getFirst()->ncolumns() ||
		!(cont_ == cont));

  if(reset) {
    clear();
    labels_ = labels;
    nvars_ = tel.getFirst()->ncolumns();
    cont_ = cont;
  }

	int i, j;
	int nsamples = tel.getFirst()->nrows();

	for(i=1; i<nvars_; i++) {
		PlotView *gv;
		if(cont[i-1]) {
			gv = reset ? new LineGraphView(sv_) : wl_.at(i-1);
			Matrix nm(nsamples, 2);
			{
				Matrix *m = tel.getFirst();
				for(j=0; j<nsamples; j++) {
					nm(j, 0) = (*m)(j, 0);
					nm(j, 1) = (*m)(j, i);
				}
			}
			((LineGraphView*)gv)->setData(nm, labels_[i-1]);
			if(reset) {
				((LineGraphView*)gv)->setAllVisible(false);
				((LineGraphView*)gv)->setGraphVisible(0, true);
			}
		} else {
			gv = reset ? new HistogramView(sv_) : wl_.at(i-1);
			QPtrList<Matrix> tel2;
			{
				QPtrListIterator<Matrix> it(tel);
				for(const Matrix *pm = it.toFirst(); pm; pm = ++it) {
					const Matrix &m = *pm;
					Matrix *nm = new Matrix(nsamples, 2);
					for(j=0; j<nsamples; j++) {
						(*nm)(j, 0) = m(j, 0);
						(*nm)(j, 1) = m(j, i);
					}
					tel2.append(nm);
					//nm->print();
				}
			}
			((HistogramView*)gv)->setData(tel2, labels_[i-1]);
			if(reset) {
				((HistogramView*)gv)->setGraphVisible(0);
			}
		}
		if(reset) {
			gv->resize(mgw, mgh);
			gv->setTitle(labels_[i-1]);
			sv_->addChild(gv);
			gv->show();
			wl_.append(gv);
			connect(gv, SIGNAL(fullScreenRequested(PlotView*)), SLOT(fullScreen(PlotView*)));
		}
	}

	if(reset) updateGraphLayout();
}
