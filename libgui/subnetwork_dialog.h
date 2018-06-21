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

#ifndef _SUBNETWORK_DIALOG_H_
#define _SUBNETWORK_DIALOG_H_

#include <qdialog.h>

#include "pathway_canvas_items.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QMultiLineEdit;
class QSpinBox;
class NetworkEditorApp;
class NetworkEditor;

class SubnetworkDialog : public QDialog {
  Q_OBJECT
public:
  SubnetworkDialog(QWidget *parent,
		   SubnetworkItem &m);

  void updateItem();
  void updateDialog();

  bool hasChanged() const;
  
signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void apply();
  void ok();
  void setChanged();
  void setTopologyChanged();
  void closeEvent(QCloseEvent*);

protected:
  SubnetworkItem &m_;
  
  QLineEdit      *label_;
  QMultiLineEdit *notes_;
  QPushButton    *apply_;
  
  bool changed_;
  bool topology_changed_;

  NetworkEditorApp *mw_;
};

#endif
