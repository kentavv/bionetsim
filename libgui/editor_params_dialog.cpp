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
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qstringlist.h>

#include "editor_params_dialog.h"

EditorParametersDialog::EditorParametersDialog(QWidget *parent,
					       const Editor &editor)
  : QDialog(parent, NULL, true, Qt::WStyle_MinMax),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,8);
  vbox->setAutoAdd(true);

  QHBox *hbox;
  QLabel *label;
  QGroupBox *group;
  
  group = new QGroupBox(4, Vertical, "Grid", this);
  hbox = new QHBox(group);
  use_grid_ = new QCheckBox("Use Grid", hbox);
  display_grid_ = new QCheckBox("Display Grid", hbox);
  hbox = new QHBox(group);
  label = new QLabel("X Step Size: ", hbox);
  label->setFixedSize(label->sizeHint());
  gsx_ = new QSpinBox(3, 4096, 1, hbox);
  label = new QLabel("  Y Step Size: ", hbox);
  label->setFixedSize(label->sizeHint());
  gsy_ = new QSpinBox(3, 4096, 1, hbox);

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);
  
  setValues(editor);

  connect(gsx_, SIGNAL(valueChanged(int)), SLOT(setChanged()));
  connect(gsy_, SIGNAL(valueChanged(int)), SLOT(setChanged()));
  connect(use_grid_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(display_grid_, SIGNAL(clicked()), SLOT(setChanged()));
	  
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
EditorParametersDialog::getValues(Editor &editor) {
  editor.gsx_            = gsx_->value();
  editor.gsy_            = gsy_->value();
  editor.use_grid_       = use_grid_->isChecked();
  editor.display_grid_   = display_grid_->isChecked();
}

void
EditorParametersDialog::setValues(const Editor &editor) {
  use_grid_->setChecked(editor.use_grid_);
  display_grid_->setChecked(editor.display_grid_);
  gsx_->setValue(editor.gsx_);
  gsy_->setValue(editor.gsy_);
}

 void
EditorParametersDialog::apply() {
  if(topology_changed_) emit topologyChanged();
  else if(changed_)     emit changed();
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
EditorParametersDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
EditorParametersDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
