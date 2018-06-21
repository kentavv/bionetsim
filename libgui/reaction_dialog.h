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

#ifndef _REACTION_DIALOG_H_
#define _REACTION_DIALOG_H_

#include <qdialog.h>
// Change to QPtrList in Qt3
#include <qlist.h>

#include "pathway_canvas_items.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QMultiLineEdit;
class QTable;
class ReactionView;

class ReactionDialog : public QDialog {
  Q_OBJECT
public:
  ReactionDialog(QWidget *parent,
		 MassActionReactionItem &r);

  void updateItem();
  void updateDialog();

signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void apply();
  void setChanged();
  void setTopologyChanged();
  
protected:
  MassActionReactionItem &r_;
  
  ReactionView *rxn_view_;
  QLineEdit *label_;
  QLineEdit *kinetic_param_;
  QLineEdit *reverse_kinetic_param_;
  QCheckBox *reversible_;
  QMultiLineEdit *notes_;
  QTable *reactants_, *products_;
  QLineEdit *equation_;
  QPushButton *apply_;
  
  bool changed_;
  bool topology_changed_;
};

#endif
