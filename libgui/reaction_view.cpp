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

#include <qlabel.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qtimer.h>

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "reaction_view.h"

#include "ir.xpm"
#include "re.xpm"

CoeffLabel::CoeffLabel(QWidget *parent)
  : QLabel(parent),
    v_(1)
{
  setNum(v_);
  setFrameStyle(LineEditPanel);
  setFrameShadow(Raised);
  setAlignment(AlignRight);
  t_ = new QTimer(this);
  connect(t_, SIGNAL(timeout()), SLOT(inc()));

  adjustSize();
  setFixedWidth(fontMetrics().width("99") + frameWidth()*2 + 2);
}

int
CoeffLabel::value() const {
  return v_;
}

void
CoeffLabel::setValue(int v) {
  v_ = v;
  if(v_ < 1) {
    v_ = 1;
  } else if(v_ > 99) {
    v_ = 99;
  }
  setNum(v_);
}
  
void
CoeffLabel::mousePressEvent(QMouseEvent *e) {
  if(e->button() == RightButton) {
    v_--;
    down_ = true;
    t_->start(100);
    if(v_ < 1) {
      v_ = 1;
    } else {
      setNum(v_);
      emit valueChanged();
    }
  } else if(e->button() == LeftButton) {
    v_++;
    down_ = false;
    t_->start(100);
    if(v_ > 99) {
      v_ = 99;
    } else {
      setNum(v_);
      emit valueChanged();
    }
  }
}

void
CoeffLabel::mouseReleaseEvent(QMouseEvent *) {
  t_->stop();
}   

void
CoeffLabel::inc() {
  if(down_) {
    v_--;
    if(v_ < 1) {
      v_ = 1;
      t_->stop();
    } else {
      setNum(v_);
      emit valueChanged();
    }
  } else {
    v_++;
    if(v_ > 99) {
      v_ = 99;
      t_->stop();
    } else {
      setNum(v_);
      emit valueChanged();
    }
  }  
}

ReactionView::ReactionView(QWidget *parent, const MassActionReactionItem *r)
  : QScrollView(parent),
    label_(NULL),
    kf_(NULL),
    kr_(NULL),
    rxn_arrow_(NULL)
{
  viewport()->setBackgroundColor(eraseColor());
  setHScrollBarMode(AlwaysOn);
  
  reactants_.setAutoDelete(true);
  reactant_coeffs_.setAutoDelete(true);
  reactant_sums_.setAutoDelete(true);
  products_.setAutoDelete(true);
  product_coeffs_.setAutoDelete(true);
  product_sums_.setAutoDelete(true);

  if(r) setValues(*r);
}

ReactionView::~ReactionView() {
}

void
ReactionView::getValues(MassActionReactionItem &r) {
  r.setLabel(label_->text());
  r.setKf(kf_->text().toDouble());
  r.setKr(kr_->text().toDouble());

  r.setReversible(rxn_re_);

  QList<EdgeBaseItem> edges = r.edges();
  EdgeBaseItem *e;
  int i, j;
  for(i=0, j=0, e = edges.first(); e; e = edges.next()) {
    if(!isReactionEdge(e)) continue;
    ReactionEdgeItem *edge = (ReactionEdgeItem*)e;
    
    if(edge->moleculeIsReactant()) {
      edge->setCoefficient(reactant_coeffs_.at(i)->value());
      i++;
    } else if(edge->moleculeIsProduct()) {
      edge->setCoefficient(product_coeffs_.at(j)->value());
      j++;
    }
  }  
}

void
ReactionView::setValues(const MassActionReactionItem &r) {
  QFont f = font();
  f.setItalic(true);
  
  reactants_.clear();
  reactant_coeffs_.clear();
  reactant_sums_.clear();
  products_.clear();
  product_coeffs_.clear();
  product_sums_.clear();

  if(!label_) {
    label_ = new QLineEdit(viewport());
    label_->setAlignment(AlignHCenter);
    connect(label_, SIGNAL(textChanged(const QString &)), SIGNAL(changed()));
  }
  if(!kf_) {
    kf_ = new QLineEdit(viewport());
    kf_->setAlignment(AlignHCenter);
    connect(kf_, SIGNAL(textChanged(const QString &)), SIGNAL(topologyChanged()));
  }
  if(!kr_) {
    kr_ = new QLineEdit(viewport());
    kr_->setAlignment(AlignHCenter);
    connect(kr_, SIGNAL(textChanged(const QString &)), SIGNAL(topologyChanged()));
  }
  if(!rxn_arrow_) {
    rxn_arrow_ = new QPushButton(viewport());
    connect(rxn_arrow_, SIGNAL(clicked()), SLOT(flipRXNDirection()));
    connect(rxn_arrow_, SIGNAL(clicked()), SIGNAL(topologyChanged()));
  }
  
  label_->setText(r.label());
  kf_->setText(tr("%1").arg(r.Kf()));
  kr_->setText(tr("%1").arg(r.Kr()));

  label_->adjustSize();
  label_->setMinimumWidth(200);
  
  if(r.reversible()) {
    rxn_arrow_->setPixmap(QPixmap(re_xpm));
    rxn_re_ = true;
    kr_->setEnabled(true);
  } else {
    rxn_arrow_->setPixmap(QPixmap(ir_xpm));
    rxn_re_ = false;
    kr_->setEnabled(false);
  }
  rxn_arrow_->adjustSize();

  kf_->adjustSize();
  kr_->adjustSize();
  kf_->setFixedWidth(rxn_arrow_->width()-4);
  kr_->setFixedWidth(rxn_arrow_->width()-4);
  
  addChild(label_);
  addChild(kf_);
  addChild(rxn_arrow_);
  addChild(kr_);
  
  label_->show();
  
  QList<EdgeBaseItem> edges = r.edges();
  EdgeBaseItem *e;
  int i, j;
  for(i=0, j=0, e = edges.first(); e; e = edges.next()) {
    if(!isReactionEdge(e)) continue;
    ReactionEdgeItem *edge = (ReactionEdgeItem*)e;
    
    CoeffLabel *cl = new CoeffLabel(viewport());
    cl->setValue(edge->coefficient());
    connect(cl, SIGNAL(valueChanged()), SIGNAL(topologyChanged()));
    
    QLabel *lb = new QLabel(edge->molecule()->label(), viewport());
    lb->setFont(f);
    lb->adjustSize();
    addChild(lb);
    
    if(edge->moleculeIsReactant()) {
      if(!reactants_.isEmpty()) {
	QLabel *pl = new QLabel("+", viewport());
	pl->adjustSize();
	addChild(pl);
	reactant_sums_.append(pl);
      }
      reactant_coeffs_.append(cl);
      reactants_.append(lb);
      i++;
    } else if(edge->moleculeIsProduct()) {
      if(!products_.isEmpty()) {
	QLabel *pl = new QLabel("+", viewport());
	pl->adjustSize();
	addChild(pl);
	product_sums_.append(pl);
      }
      product_coeffs_.append(cl);
      products_.append(lb);
      j++;
    }
  }

  {
    QWidget *w;
    moveChild(label_, 5, 5);
    int x=5, y=label_->height() * 3;
    int np = products_.count(), nr = reactants_.count();
    int i;
    
    for(i=0; i<nr; i++) {
      if(i > 0) {
	w = reactant_sums_.at(i-1);
	moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
	w->show();
	x += w->width() + 5;
      }

      w = reactant_coeffs_.at(i);
      moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
      w->show();
      x += w->width();

      w = reactants_.at(i);
      moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
      w->show();
      x += w->width() + 5;
    }

    moveChild(kf_, x+2, y-kf_->height());
    moveChild(rxn_arrow_, x, y);
    moveChild(kr_, x+2, y+rxn_arrow_->height());
    kf_->show();
    kr_->show();
    rxn_arrow_->show();
    x += rxn_arrow_->width()+5;
    
    for(i=0; i<np; i++) {
      if(i > 0) {
	w = product_sums_.at(i-1);
	moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
	w->show();
	x += w->width() + 5;
      }

      w = product_coeffs_.at(i);
      moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
      w->show();
      x += w->width();

      w = products_.at(i);
      moveChild(w, x, y + (rxn_arrow_->height()-w->height())/2);
      w->show();
      x += w->width() + 5;
    }
    {
      int w = x+5 > 200+5 ? x+5 : 200+5;
      int h = kr_->y()+kr_->height();
      label_->setFixedWidth(w);
      resizeContents(w, h);
      setFixedHeight(h+25);
    }
  }
}

void
ReactionView::flipRXNDirection() {
  if(!rxn_re_) {
    rxn_arrow_->setPixmap(QPixmap(re_xpm));
    rxn_re_ = true;
    kr_->setEnabled(true);
  } else {
    rxn_arrow_->setPixmap(QPixmap(ir_xpm));
    rxn_re_ = false;
    kr_->setEnabled(false);
  }
}
