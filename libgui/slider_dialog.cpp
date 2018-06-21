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

#include <qcheckbox.h>
#include <qcursor.h>
#include <qdialog.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qvbox.h>

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "slider_dialog.h"

SliderItem::SliderItem(QWidget *parent, MolRxnBaseItem *item, const QString &comp)
  : QWidget(parent),
    item_(item),
    comp_(comp)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  hbox = new QHBox(this);
  name_ = new QLabel(hbox);
  rxn_ = new QLabel(hbox);

  hbox = new QHBox(this);
  min_ = new QLineEdit(hbox);
  cur_ = new QLineEdit(hbox);
  max_ = new QLineEdit(hbox);

  slider_ = new QSlider(0, 1000, 10, 5, Horizontal, this);

  min_->setText("0.0");
  max_->setText("2000.0");
  
  updateFields();

  connect(slider_, SIGNAL(valueChanged(int)), SLOT(sliderChanged(int)));
  connect(max_, SIGNAL(textChanged(const QString&)), SLOT(sliderLimitsChanged()));
  connect(min_, SIGNAL(valueChanged(const QString&)), SLOT(sliderLimitsChanged()));
  connect(cur_, SIGNAL(valueChanged(const QString&)), SLOT(curChanged()));

  resize(300, 90);
}

SliderItem::~SliderItem() {
}

void
SliderItem::updateFields() {
  if(isReaction(item_)) {
    name_->setText(item_->label());
    QString str = ((ReactionItem*)item_)->equation();
    if(comp_ == "Kf") {
      str.replace(" <-> ", " --> ");
      rxn_->setText(str);
    } else if(comp_ == "Kr") {
      str.replace(" <-> ", " <-- ");
      rxn_->setText(str);
    }
  } else if(isMolecule(item_)) {
    if(comp_ == "Decay Rate") {
      name_->setText(item_->label() + " -> 0");
    } else if(comp_ == "Initial Concentration") {
      name_->setText(item_->label());
    }
  }

  double cur = -1.0;
  if(isReaction(item_)) {
    if(comp_ == "Kf") {
      cur = ((MassActionReactionItem*)item_)->Kf();
    } else if(comp_ == "Kr") {
      cur = ((MassActionReactionItem*)item_)->Kr();
    }
  } else if(isMolecule(item_)) {
    if(comp_ == "Decay Rate") {
      cur = ((MoleculeItem*)item_)->decayRate();
    } else if(comp_ == "Initial Concentration") {
      cur = ((MoleculeItem*)item_)->initialConcentration();
    }
  }

  cur_->setText(tr("%1").arg(cur));

  double min = min_->text().toDouble();
  double max = max_->text().toDouble();
  int v = (int)((cur+min)*1000/(max-min) + 0.5);
  slider_->setValue(v);
}

void
SliderItem::sliderChanged(int) {
  double min = min_->text().toDouble();
  double max = max_->text().toDouble();
  double cur = slider_->value() * (max-min)/1000 + min;

  if(isReaction(item_)) {
    if(comp_ == "Kf") {
      ((MassActionReactionItem*)item_)->setKf(cur);
    } else if(comp_ == "Kr") {
      ((MassActionReactionItem*)item_)->setKr(cur);
    }
  } else if(isMolecule(item_)) {
    if(comp_ == "Decay Rate") {
      ((MoleculeItem*)item_)->setDecayRate(cur);
    } else if(comp_ == "Initial Concentration") {
      ((MoleculeItem*)item_)->setInitialConcentration(cur);
    }
  }

  updateFields();

  emit parametersChanged();
}

void
SliderItem::sliderLimitsChanged() {
  double min = min_->text().toDouble();
  double max = max_->text().toDouble();
  double cur = cur_->text().toDouble();

  if(min < max) {
    if(cur < min) cur = min;
    if(cur > max) cur = max;

    int v = (int)((cur+min)*1000/(max-min) + 0.5);
    slider_->setValue(v);
  }
}

void
SliderItem::curChanged() {
  double min = min_->text().toDouble();
  double max = max_->text().toDouble();
  double cur = cur_->text().toDouble();

  if(min <= cur && cur <= max) {
    slider_->setValue(cur);
    sliderChanged(0);
  }
}

SliderScrollView::SliderScrollView(QWidget *parent)
  : QScrollView(parent)
{
  slider_menu_ = new QPopupMenu(this);
  slider_menu_->setCheckable(true);
  slider_menu_->setEnabled(false);

  viewport()->setMouseTracking(TRUE);
  
  setHScrollBarMode(AlwaysOn);
  setVScrollBarMode(AlwaysOn);
  resizeContents(100, 100);
  enableClipper(true);
  
  connect(slider_menu_, SIGNAL(activated(int)), SLOT(sliderMenu(int)));
  connect(slider_menu_, SIGNAL(aboutToShow()), SLOT(updateSliderMenu()));
}

SliderScrollView::~SliderScrollView() {
}

void
SliderScrollView::setItems(const QList<PathwayCanvasItem> &items) {
  items_ = items;
  updateSliderMenu();
}

void
SliderScrollView::mousePressEvent(QMouseEvent *e) {
  QScrollView::mousePressEvent(e);
  if(e->button() == Qt::RightButton) {
    updateSliderMenu();
    slider_menu_->popup(QCursor::pos());
  }
}

void
SliderScrollView::updateSliderMenu() {
  slider_menu_->clear();
  slider_menu_->setEnabled(true);
  sliders_.clear();
  
  PathwayCanvasItem *i;
  int j=0;
  QString str;
  for(i=items_.first(); i; i=items_.next()) {
    if(isMolecule(i)) {
      str = ((MoleculeItem*)i)->label() + " (IC)";
      slider_menu_->insertItem(str, j++);
      SliderItem *s = new SliderItem(this->viewport(), (MoleculeItem*)i, "Initial Concentration");
      sliders_.append(s);
      addChild(s, 0, (j-1)*90);
      s->show();
      connect(s, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
      
      if(((MoleculeItem*)i)->decays()) {
	str = ((MoleculeItem*)i)->label() + " --> 0";
	slider_menu_->insertItem(str, j++);
	SliderItem *s = new SliderItem(this->viewport(), (MoleculeItem*)i, "Decay Rate");
	sliders_.append(s);
	addChild(s, 0, (j-1)*90);
	s->show();
	connect(s, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
      }
    }
  }

  slider_menu_->insertSeparator();
  
  for(i=items_.first(); i; i=items_.next()) {
    if(isReaction(i)) {
      str = ((ReactionItem*)i)->label() + ": " + ((ReactionItem*)i)->equation();
      str.replace(" <-> ", " --> ");
      slider_menu_->insertItem(str, j++);
      SliderItem *s = new SliderItem(this->viewport(), (ReactionItem*)i, "Kf");
      sliders_.append(s);
      addChild(s, 0, (j-1)*90);
      s->show();
      connect(s, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
      
      if(((ReactionItem*)i)->reversible()) {
	str.replace(" --> ", " <-- ");
	slider_menu_->insertItem(str, j++);
	SliderItem *s = new SliderItem(this->viewport(), (ReactionItem*)i, "Kr");
	sliders_.append(s);
	addChild(s, 0, (j-1)*90);
	s->show();
	connect(s, SIGNAL(parametersChanged()), SIGNAL(parametersChanged()));
      }
    }
  }
  resizeContents(300, j*90);
}

void
SliderScrollView::sliderMenu(int) {
}

SliderDialog::SliderDialog(QWidget *parent)
  : QDialog(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  sv_ = new SliderScrollView(this);

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);

  connect(sv_, SIGNAL(parametersChanged()), SLOT(setParametersChanged()));

  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

SliderDialog::~SliderDialog() {
}

void
SliderDialog::setItems(const QList<PathwayCanvasItem> &items) {
  sv_->setItems(items);
}

void
SliderDialog::apply() {
  emit parametersChanged();
  apply_->setEnabled(false);
}

void
SliderDialog::setParametersChanged() {
  apply_->setEnabled(true);
  emit parametersChanged();
}

