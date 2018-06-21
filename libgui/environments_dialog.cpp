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
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qtable.h>

#include "environments_dialog.h"
#include "simulation.h"
#include "version.h"

EnvironmentsDialog::EnvironmentsDialog(QWidget *parent,
				       const Simulation &simulation)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  envs_ = new QTable(this);
  envs_->setSorting(false);
  envs_->setColumnWidth(0, 300);
  envs_->verticalHeader()->setResizeEnabled(false);
  
  hbox = new QHBox(this);
  QPushButton *add = new QPushButton("Add", hbox);
  QPushButton *remove = new QPushButton("Remove", hbox);

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);
  
  setValues(simulation);
  envs_->setColumnWidth(0, 300);
  resize(width(), 250);  

  connect(add, SIGNAL(clicked()), SLOT(setChanged()));
  connect(remove, SIGNAL(clicked()), SLOT(setChanged()));
  connect(add, SIGNAL(clicked()), SLOT(addEnvironment()));
  connect(remove, SIGNAL(clicked()), SLOT(removeEnvironment()));
  connect(envs_, SIGNAL(valueChanged(int, int)), SLOT(setChanged()));
  connect(envs_, SIGNAL(valueChanged(int, int)), SLOT(fixupColumnWidth()));
	  
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
EnvironmentsDialog::getValues(Simulation &simulation) {
  int n = envs_->numRows();
  QStringList desc;
  for(int i=0; i<n; i++) {
    desc.push_back(envs_->text(i, 0));
    printf("%s\n", (const char*)envs_->text(i, 0));
  }
  simulation.setNumEnvironment(n);
  simulation.setEnvironmentDescriptions(desc);
}

void
EnvironmentsDialog::setValues(const Simulation &simulation) {
  envs_->setNumRows(0);
  envs_->setNumCols(1);
  envs_->horizontalHeader()->setLabel(0, "Description");
  
  QStringList desc = simulation.environmentDescriptions();
  envs_->setNumRows(desc.count());
  QStringList::iterator i;
  int j;
  for(i = desc.begin(), j=0; i != desc.end(); i++, j++) {
    envs_->setText(j, 0, *i);
  }
  fixupColumnWidth();
}

void
EnvironmentsDialog::accept() {
  if(envs_->numRows() > 0) {
    QDialog::accept();
  }
}

void
EnvironmentsDialog::apply() {
  if(envs_->numRows() == 0) {
    QMessageBox::warning(this, appname, "At least one environment must be specified", QMessageBox::Ok, QMessageBox::NoButton);
  } else {
    if(topology_changed_) emit topologyChanged();
    else if(changed_)     emit changed();
    changed_ = false;
    topology_changed_ = false;
    apply_->setEnabled(false);
  }
}

void
EnvironmentsDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
EnvironmentsDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}

void
EnvironmentsDialog::addEnvironment() {
  envs_->insertRows(envs_->numRows());
  envs_->setText(envs_->numRows()-1, 0,
		 tr("New Environment %1").arg(envs_->numRows()));
}

void
EnvironmentsDialog::removeEnvironment() {
  if(envs_->isRowSelected(envs_->currentRow())) {
    envs_->removeRow(envs_->currentRow());
  }
}

void
EnvironmentsDialog::fixupColumnWidth() {
  envs_->adjustColumn(0);
}
