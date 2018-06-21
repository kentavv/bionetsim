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

#ifndef _ENVIRONMENTS_DIALOG_H_
#define _ENVIRONMENTS_DIALOG_H_

#include <qdialog.h>

#include "simulation.h"

class QPushButton;
class QTable;

class EnvironmentsDialog : public QDialog {
  Q_OBJECT
public:
  EnvironmentsDialog(QWidget *parent,
		     const Simulation &simulation);
  
  void getValues(Simulation &simulation);
  void setValues(const Simulation &simulation);
  
signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void accept();
  void apply();
  void setChanged();
  void setTopologyChanged();

  void addEnvironment();
  void removeEnvironment();
  void fixupColumnWidth();
  
protected:
  QTable *envs_;
  QPushButton *apply_;

  bool changed_;
  bool topology_changed_;
};

#endif
