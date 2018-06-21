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

#ifndef _REACTION_VIEW_H_
#define _REACTION_VIEW_H_

#include <qlabel.h>
#include <qlist.h>
#include <qscrollview.h>

#include "pathway_canvas_items.h"

class QLabel;
class QLineEdit;
class QMouseEvent;
class QPushButton;
class QTimer;

class CoeffLabel : public QLabel {
  Q_OBJECT
public:
  CoeffLabel(QWidget *parent);

  int value() const;
  void setValue(int);

signals:
  void valueChanged();

protected slots:
  void inc();
 
protected:
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);

  int v_;
  bool down_;
  QTimer *t_;
};

class ReactionView : public QScrollView {
  Q_OBJECT
public:
  ReactionView(QWidget *parent, const MassActionReactionItem *r = NULL);
  ~ReactionView();
  
  void getValues(MassActionReactionItem &r);
  void setValues(const MassActionReactionItem &r);

signals:
  void changed();
  void topologyChanged();

protected slots:
  void flipRXNDirection();
  
protected:
  QLineEdit *label_;
  QLineEdit *kf_;
  QLineEdit *kr_;
  QPushButton *rxn_arrow_;
  QList<CoeffLabel> reactant_coeffs_;
  QList<QLabel> reactants_;
  QList<QLabel> reactant_sums_;
  QList<CoeffLabel> product_coeffs_;
  QList<QLabel> products_;
  QList<QLabel> product_sums_;
  bool rxn_re_;
};

#endif
