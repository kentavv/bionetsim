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

#ifndef _INTERPOLATED_CONTROL_VARIABLE_DIALOG_H_
#define _INTERPOLATED_CONTROL_VARIABLE_DIALOG_H_

#include <qdialog.h>

#include "pathway_canvas_items.h"
#include "simulation.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMultiLineEdit;
class QTable;

class LineGraphView;

class InterpolatedControlVariableDialog : public QDialog {
  Q_OBJECT
public:
  InterpolatedControlVariableDialog(QWidget *parent,
				  const Simulation &simulation,
				  InterpolatedControlVariableItem &cv);
  ~InterpolatedControlVariableDialog();

  void updateItem();
  void updateDialog(const Simulation &simulation);

  void setValues(const Matrix &);

signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void import();
  void apply();
  void setChanged();
  void setTopologyChanged();

  void updateGraph();

protected:
  InterpolatedControlVariableItem &cv_;
  
  Simulation simulation_;

  QLineEdit *label_;
  QCheckBox *exported_;
  QCheckBox *plotted_;

  //int cur_env_;
  QTable *values_;
  
  QMultiLineEdit *notes_;

  LineGraphView *lg_;

  QPushButton *apply_;
  
  bool changed_;
  bool topology_changed_;
};

#endif
