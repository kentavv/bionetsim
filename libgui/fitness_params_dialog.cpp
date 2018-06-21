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

#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qcheckbox.h>

#include "fitness_params_dialog.h"
#include "fitness.h"
#include "fitness_lang.h"

FitnessParametersDialog::FitnessParametersDialog(QWidget *parent,
						 const QStringList &mol_names,
						 const QStringList &env_names,
						 const Fitness &fitness)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  hbox = new QHBox(this);
  new QLabel("Fitness Function", hbox);
  ff_ = new QComboBox(hbox);
  
  generic_group_ = new QGroupBox(4, Vertical, "Generic Fitness Function Parameters", this);
  generic_target_ = new QTable(generic_group_);
  hbox = new QHBox(generic_group_);
  QPushButton *add = new QPushButton("Add", hbox);
  QPushButton *remove = new QPushButton("Remove", hbox);

	dbg_ = new QCheckBox("Enable script debugging", this);
	dbg_->setChecked(fitnessFunctionDebug() > 0);

  text_ = new QTextEdit(this);
  //text_->setTabStopWidth(2);
  //text_->setFamily("Courier");
  //text_->setCurrentFont(QFont("Courier", 10));
  text_->setFont(QFont("Courier", 8));

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *calc = new QPushButton("Calculate", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);
  
  setValues(mol_names, env_names, fitness);

  connect(ff_, SIGNAL(activated(int)), SLOT(setChanged()));
  connect(ff_, SIGNAL(activated(const QString &)), SLOT(functionChanged(const QString &)));
  connect(add, SIGNAL(clicked()), SLOT(setChanged()));
  connect(remove, SIGNAL(clicked()), SLOT(setChanged()));
  connect(add, SIGNAL(clicked()), SLOT(addTarget()));
  connect(remove, SIGNAL(clicked()), SLOT(removeTarget()));
  connect(generic_target_, SIGNAL(valueChanged(int, int)), SLOT(setChanged()));
  connect(text_, SIGNAL(textChanged()), SLOT(setChanged()));
	  
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(calc, SIGNAL(clicked()), SLOT(apply()));
  connect(calc, SIGNAL(clicked()), SIGNAL(calculate()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	connect(dbg_, SIGNAL(toggled(bool)), SLOT(enableDebug(bool)));
}

void
FitnessParametersDialog::getValues(Fitness &fitness) {
  QStringList names;
  int nr = generic_target_->numRows();
  int nc = generic_target_->numCols()-1;
  Matrix mat(nr, nc);  
  for(int i=0; i<nr; i++) {
    QComboBox *cb = (QComboBox*)generic_target_->cellWidget(i, 0);
    names.push_back(cb->currentText());
    for(int j=0; j<nc; j++) {
      mat(i, j) = generic_target_->text(i, j+1).toDouble();
    }
  }

  fitness.setFunction(ff_->currentText());
  fitness.setGeneric(names, mat);
  fitness.setScript(text_->text());
}

void
FitnessParametersDialog::setValues(const QStringList &mol_names,
				   const QStringList &env_names,
				   const Fitness &fitness) {
  mol_names_ = mol_names;
  
  ff_->clear();
  ff_->insertStringList(fitness.functionNames());
  ff_->setCurrentText(fitness.function());
  text_->setText(fitness.script());

  generic_group_->setEnabled(fitness.function() == "Generic");
  text_->setEnabled(fitness.function() == "Script");

  QStringList labels = env_names;
  labels.push_front("Molecule");
  generic_target_->setNumRows(0);
  generic_target_->setNumCols(labels.count());
  generic_target_->setColumnLabels(labels);
  
  QStringList names;
  Matrix mat;
  fitness.genericTarget(names, mat);
  for(int i=0; i<mat.nrows(); i++) {
    addTarget();
    QComboBox *cb = (QComboBox*)generic_target_->cellWidget(i, 0);
    cb->setCurrentText(names[i]);
    for(int j=0; j<mat.ncolumns(); j++) {
      generic_target_->setText(i, j+1, tr("%1").arg(mat(i, j)));
    }
  }
}

void
FitnessParametersDialog::apply() {
  if(topology_changed_) emit topologyChanged();
  else if(changed_)     emit changed();
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
FitnessParametersDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
FitnessParametersDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}

void
FitnessParametersDialog::addTarget() {
  int ind = generic_target_->numRows();
  generic_target_->insertRows(ind);
  QComboBox *cb = new QComboBox(this);
  cb->insertStringList(mol_names_);
  generic_target_->setCellWidget(ind, 0, cb);
}

void
FitnessParametersDialog::removeTarget() {
  int r = generic_target_->currentRow();
  if(generic_target_->isRowSelected(r)) {
    generic_target_->removeRow(r);
  }
}

void
FitnessParametersDialog::functionChanged(const QString &str) {
  generic_group_->setEnabled(str == "Generic");
  dbg_->setEnabled(str == "Script");
  text_->setEnabled(str == "Script");
}

void
FitnessParametersDialog::enableDebug(bool dbg) {
	setFitnessFunctionDebug(dbg ? 1 : 0);
}