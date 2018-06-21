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
#include <qdialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qpushbutton.h>
#include <qspinbox.h>

#include "annotation_dialog.h"
#include "pathway_canvas_items.h"

AnnotationDialog::AnnotationDialog(QWidget *parent,
				   AnnotationItem &m)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    m_(m),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  new QLabel("Annotation", this);

  new QLabel("Notes:", this);
  notes_ = new QMultiLineEdit(this);
  
  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);

  updateDialog();
  
  connect(notes_, SIGNAL(textChanged()), SLOT(setChanged()));

  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
AnnotationDialog::updateItem() {
  m_.setLabel(notes_->text());
}

void
AnnotationDialog::updateDialog() {
  notes_->setText(m_.label());
}

void
AnnotationDialog::apply() {
  if(topology_changed_) {
    m_.updatePropertiesFromDialog();
    emit topologyChanged();
  } else if(changed_) {
    m_.updatePropertiesFromDialog();
    emit changed();
  }
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
AnnotationDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
AnnotationDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
