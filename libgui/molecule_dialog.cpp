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
#include <qcombobox.h>
#include <qdialog.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qvgroupbox.h>

#include "molecule_dialog.h"
#include "pathway_canvas_items.h"

MoleculeDialog::MoleculeDialog(QWidget *parent,
			       MoleculeItem &m)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    m_(m),
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
    
    hbox = new QHBox(vbox);
    new QLabel("Molecule Label: ", hbox);
    label_ = new QLineEdit(hbox);
    
    hbox = new QHBox(vbox);
    new QLabel("Initial Concentration: ", hbox);
    initial_con_ = new QLineEdit(hbox);

    hbox = new QHBox(vbox);
    decays_ = new QCheckBox("Decay rate: ", hbox);
    decay_rate_ = new QLineEdit(hbox);
    
    exported_ = new QCheckBox("Exported", vbox);
    plotted_ = new QCheckBox("Plotted", vbox);

    new QLabel("Notes:", vbox);
    notes_ = new QMultiLineEdit(vbox);

    tab->addTab(vbox, "Options");
  }

#if 0
  {
    QVBox *vbox = new QVBox(this);
    vbox->setSpacing(8);
    
    hbox = new QHBox(vbox);
    new QLabel("Initial Concentration", hbox);
    QComboBox *cb = new QComboBox(hbox);
    cb->insertItem("None");
    cb->insertItem("Equal to");
    new QComboBox(hbox);
    
    hbox = new QHBox(vbox);
    new QLabel("Decay Rate", hbox);
    cb = new QComboBox(hbox);
    cb->insertItem("None");
    cb->insertItem("Equal to");
    new QComboBox(hbox);

    tab->addTab(vbox, "Constraints");
  }

  {
    QVBox *vbox = new QVBox(this);
    vbox->setSpacing(8);
    
    hbox = new QHBox(vbox);
    new QLabel("Initial Concentration", hbox);
    new QLineEdit(hbox);
    new QLineEdit(hbox);
    new QCheckBox("Optimize", hbox);
    
    hbox = new QHBox(vbox);
    new QLabel("Decay Rate", hbox);
    new QLineEdit(hbox);
    new QLineEdit(hbox);
    new QCheckBox("Optimize", hbox);

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

  connect(label_, SIGNAL(textChanged(const QString &)), SLOT(setChanged()));
  connect(initial_con_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(decays_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(decay_rate_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(exported_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(plotted_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(notes_, SIGNAL(textChanged()), SLOT(setChanged()));
  
  connect(label_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(initial_con_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(decay_rate_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
MoleculeDialog::updateItem() {
  m_.setLabel(label_->text());
  m_.setInitialConcentration(initial_con_->text().toDouble());
  m_.setDecayRate(decay_rate_->text().toDouble());
  m_.setDecays(decays_->isChecked());
  m_.setExported(exported_->isChecked());
  m_.setPlotted(plotted_->isChecked());
  m_.setNotes(notes_->text());
}

void
MoleculeDialog::updateDialog() {
  setCaption(m_.caption());
  label_->setText(m_.label());
  initial_con_->setText(tr("%1").arg(m_.initialConcentration()));
  decays_->setChecked(m_.decays());
  decay_rate_->setText(tr("%1").arg(m_.decayRate()));
  exported_->setChecked(m_.exported());
  plotted_->setChecked(m_.plotted());
  notes_->setText(m_.notes());
}

void
MoleculeDialog::apply() {
  if(topology_changed_) {
    m_.updatePropertiesFromDialog();
    emit topologyChanged();
  } else if(changed_)     {
    m_.updatePropertiesFromDialog();
    emit changed();
  }
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
MoleculeDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
MoleculeDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
