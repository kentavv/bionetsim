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

#include <stdio.h>

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qtable.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qvgroupbox.h>

#include "reaction_dialog.h"
#include "reaction_view.h"

ReactionDialog::ReactionDialog(QWidget *parent,
			       MassActionReactionItem &r)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    r_(r),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  QTabWidget *tab = new QTabWidget(this);
  
  {
    QVBox *vbox = new QVBox(this);
    vbox->setSpacing(8);

    rxn_view_ = new ReactionView(vbox);
    
    new QLabel("Notes:", vbox);
    notes_ = new QMultiLineEdit(vbox);

    tab->addTab(vbox, "Options");
  }

#if 0
  {
    QVBox *vbox = new QVBox(this);
    vbox->setSpacing(8);
    
    QGroupBox *con_rates = new QVGroupBox("Kinetic Rates", vbox);
    hbox = new QHBox(con_rates);
    new QLabel("Kf", hbox);
    QComboBox *cb = new QComboBox(hbox);
    cb->insertItem("None");
    cb->insertItem("Equal to");
    new QComboBox(hbox);
    
    hbox = new QHBox(con_rates);
    new QLabel("Kr", hbox);
    cb = new QComboBox(hbox);
    cb->insertItem("None");
    cb->insertItem("Equal to");
    new QComboBox(hbox);
    
    /*QGroupBox *con_rcoeffs =*/ new QVGroupBox("Reactant Coefficients", vbox);
    /*QGroupBox *con_pcoeffs =*/ new QVGroupBox("Product Coefficients", vbox);

    tab->addTab(vbox, "Constraints");
  }

  {
    QVBox *vbox = new QVBox(this);
    vbox->setSpacing(8);

    QGroupBox *opt_rates = new QVGroupBox("Kinetic Rates", vbox);
    hbox = new QHBox(opt_rates);
    new QLabel("Kf", hbox);
    new QLineEdit(hbox);
    new QLineEdit(hbox);
    new QCheckBox("Optimize", hbox);
    
    hbox = new QHBox(opt_rates);
    new QLabel("Kr", hbox);
    new QLineEdit(hbox);
    new QLineEdit(hbox);
    new QCheckBox("Optimize", hbox);
    
    /*QGroupBox *opt_rcoeffs =*/ new QVGroupBox("Reactant Coefficients", vbox);
    /*QGroupBox *opt_pcoeffs =*/ new QVGroupBox("Product Coefficients", vbox);

    tab->addTab(vbox, "Optimization");
  }
#endif
  
  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);
    
  updateDialog();
    
  connect(rxn_view_, SIGNAL(changed()), SLOT(setChanged()));
  connect(rxn_view_, SIGNAL(topologyChanged()), SLOT(setTopologyChanged()));
  connect(notes_, SIGNAL(textChanged()), SLOT(setChanged()));
  
#if 0
  connect(label_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(kinetic_param_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(reverse_kinetic_param_, SIGNAL(returnPressed()), SLOT(apply()));
#endif
  
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
ReactionDialog::updateItem() {
  rxn_view_->getValues(r_);
  r_.setNotes(notes_->text());
}

void
ReactionDialog::updateDialog() {
  rxn_view_->setValues(r_);
  setCaption(r_.caption());
  notes_->setText(r_.notes());
}

void
ReactionDialog::apply() {
  if(topology_changed_) {
    r_.updatePropertiesFromDialog();
    emit topologyChanged();
  } else if(changed_) {
    r_.updatePropertiesFromDialog();
    emit changed();
  }
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
ReactionDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
ReactionDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
