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
#include <qlist.h>
#include <qmessagebox.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qvgroupbox.h>

#include "subnetwork_dialog.h"
#include "pathway_canvas_items.h"
#include "network_editor_mw.h"
#include "network_editor.h"

SubnetworkDialog::SubnetworkDialog(QWidget *parent,
				   SubnetworkItem &m)
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
    new QLabel("Subnetwork Label: ", hbox);
    label_ = new QLineEdit(hbox);
    
    new QLabel("Notes:", vbox);
    notes_ = new QMultiLineEdit(vbox);

    mw_ = new NetworkEditorApp(vbox, NULL, 0, true);
    
    tab->addTab(vbox, "Options");
  }

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);

  updateDialog();

  connect(label_, SIGNAL(textChanged(const QString &)), SLOT(setChanged()));
  connect(notes_, SIGNAL(textChanged()), SLOT(setChanged()));
  
  connect(mw_, SIGNAL(changed()), SLOT(setChanged()));
  connect(mw_, SIGNAL(topologyChanged()), SLOT(setTopologyChanged()));

  connect(label_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(ok()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

void
SubnetworkDialog::updateItem() {
  m_.setLabel(label_->text());
  m_.setNotes(notes_->text());

  QList<PathwayCanvasItem> items;
  NetworkEditor *ne = mw_->activeNetworkEditor();
  if(ne) {
    ne->copyCanvas(items);
    m_.setItems(items);
  }
}

void
SubnetworkDialog::updateDialog() {
  setCaption(m_.caption());
  label_->setText(m_.label());
  notes_->setText(m_.notes());

  QList<PathwayCanvasItem> items = m_.items();
  NetworkEditor *ne = mw_->activeNetworkEditor();
  if(!ne) {
    ne = mw_->newDoc();
  }
  ne->replaceCanvas(items);
}

void
SubnetworkDialog::apply() {
  if(mw_->numNetworks() > 1) {
    QMessageBox::warning(this, "Subnetwork error", "At most one network can be defined as a subnetwork");
    return;
  }
  if(mw_->numNetworks() == 0) {
    QMessageBox::warning(this, "Subnetwork error", "At least one network must be defined as a subnetwork");
    return;
  }
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
SubnetworkDialog::ok() {
  apply();
  if(mw_->numNetworks() == 1) accept();
}

void
SubnetworkDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
SubnetworkDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}

bool
SubnetworkDialog::hasChanged() const {
  return changed_ || topology_changed_;
}

void
SubnetworkDialog::closeEvent(QCloseEvent *e) {
  if(hasChanged()) {
    apply();
  }

  if(mw_->numNetworks() == 1) {
    e->accept();
    close();
  } else {
    e->ignore();
  }
}
