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

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qcombobox.h>
#include <qslider.h>

#include "dynamic_dialog.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "vcrcontrols.h"

DynamicDialog::DynamicDialog(QWidget *parent)
: QDialog(parent, NULL),
pitems_(NULL),
color_mode_(0),
norm_mode_(1),
w_(NULL)
{
	QVBoxLayout *vbox = new QVBoxLayout(this,8);
	vbox->setAutoAdd(true);

	vcr_ = new VCRControls(this);

	slider_ = new QSlider(Qt::Horizontal, this);

	QHBox *hbox;

	hbox = new QHBox(this);
	QComboBox *cb = new QComboBox(hbox);
	cb->insertItem("Normalize Each Node Independently");
	cb->insertItem("Normalize Over All Nodes");
	cb->insertItem("Normalize Over All Nodes Each Time Step");
	cb->setCurrentItem(norm_mode_);

	QComboBox *cb2 = new QComboBox(hbox);
	cb2->insertItem("HSV Color Scheme");
	cb2->insertItem("Red Color Scheme");
	cb2->setCurrentItem(color_mode_);

	hbox = new QHBox(this);
	QPushButton *ok = new QPushButton("OK", hbox);
	ok->setFixedWidth(ok->sizeHint().width());
	ok->setDefault(true);

	connect(vcr_, SIGNAL(changedFrame(int)), SLOT(changedTime(int)));
	connect(cb2, SIGNAL(activated(int)), SLOT(colorModeChanged(int)));
	connect(cb, SIGNAL(activated(int)), SLOT(normalizeModeChanged(int)));
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	connect(slider_, SIGNAL(sliderMoved(int)), SLOT(sliderMoved(int)));
	connect(slider_, SIGNAL(sliderMoved(int)), SLOT(changedTime(int)));
}

DynamicDialog::~DynamicDialog() {
}

void
DynamicDialog::setData(const QValueList<Result> &results,
											 QList<PathwayCanvasItem> *pitems,
											 QWidget *w) {
												 if(results.size() > 0) {
													 m_ = results[0].m;
													 pitems_ = pitems;
													 w_ = w;
													 vcr_->setFrameLimit(m_.nrows());
													 slider_->setRange(1, m_.nrows());
												 }
}

void
DynamicDialog::colorModeChanged(int in) {
	color_mode_ = in;
	changedTime(vcr_->frame());
}

void
DynamicDialog::normalizeModeChanged(int in) {
	norm_mode_ = in;
	changedTime(vcr_->frame());
}

void
DynamicDialog::changedTime(int ts) {
	int i, t, nm=m_.ncolumns(), nt=m_.nrows();
	Matrix max_r(nm, 1, -1e6), min_r(nm, 1, 1e6);
	double max_v(-1e6), min_v(1e6);

	if(ts < 1 || nt < ts || nm < 2) return;
	
	slider_->setValue(ts);

	int a, b;
	if(norm_mode_ == 2) {
		a = ts-1;
		b = a+1;
	} else {
		a = 0;
		b = nt;
	}
	for(t=a; t<b; t++) {
		for(i=1; i<nm; i++) {
			if(m_(t, i) > max_r(i, 0)) max_r(i, 0) = m_(t, i);
			if(m_(t, i) < min_r(i, 0)) min_r(i, 0) = m_(t, i);

			if(m_(t, i) > max_v) max_v = m_(t, i);
			if(m_(t, i) < min_v) min_v = m_(t, i);
		}
	}

	t = ts - 1;
	if(pitems_) {
		PathwayCanvasItem *item;
		i = 1;
		for(int j=0; j<2; j++) {
			for(item=pitems_->first(); item; item=pitems_->next()) {
				if(isMolecule(item) &&
					((j == 0 && isControlVariable(item)) ||
					(j == 1 && !isControlVariable(item)))) {
						if(((MoleculeItem*)item)->plotted()) {
						double s;
						if(norm_mode_ == 0 && max_r(i, 0) != min_r(i, 0)) {
							s = (m_(t, i) - min_r(i, 0)) / (max_r(i, 0) - min_r(i, 0));
						} else {
							s = (m_(t, i) - min_v) / (max_v - min_v);
						}
						if(color_mode_ == 0) {
							((MoleculeItem*)item)->setColor(QColor((int)((1.0-s)*240+0.5), 255, 255, QColor::Hsv));
							((MoleculeItem*)item)->setConcentration(m_(t, i));
						} else {
							int g = (int)((1.0-s)*255+0.5);
							((MoleculeItem*)item)->setColor(QColor(255, g, g));
							((MoleculeItem*)item)->setConcentration(m_(t, i));
						}
						i++;
						}
				}
			}
		}
		w_->update();
	}
}

void
DynamicDialog::stop() {
	vcr_->stop();
	if(pitems_) {
		PathwayCanvasItem *item;
		for(item=pitems_->first(); item; item=pitems_->next()) {
			if(isMolecule(item)) {
				((MolRxnBaseItem*)item)->defaultColor();
			}
		}
		w_->update();
	}
}

void
DynamicDialog::done(int r) {
	stop();
	QDialog::done(r);
}

void
DynamicDialog::hide() {
	stop();
	QDialog::hide();
}

void
DynamicDialog::show() {
	changedTime(vcr_->frame());
	QDialog::show();
}

void
DynamicDialog::sliderMoved(int ts) {
	vcr_->setFrame(ts);
}
