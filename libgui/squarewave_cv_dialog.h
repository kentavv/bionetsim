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

#ifndef _SQUAREWAVE_CONTROL_VARIABLE_DIALOG_H_
#define _SQUAREWAVE_CONTROL_VARIABLE_DIALOG_H_

#include <qdialog.h>

#include "pathway_canvas_items.h"
#include "simulation.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMultiLineEdit;

class LineGraphView;

class SquareWaveControlVariableDialog : public QDialog {
  Q_OBJECT
public:
  SquareWaveControlVariableDialog(QWidget *parent,
				  const Simulation &simulation,
				  SquareWaveControlVariableItem &cv);
  ~SquareWaveControlVariableDialog();

  void updateItem();
  void updateDialog(const Simulation &simulation);

signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void apply();
  void setChanged();
  void setTopologyChanged();

  void changeEnv(int);
  void updateGraph();

protected:
  SquareWaveControlVariableItem &cv_;
  
  Simulation simulation_;

  QLineEdit *label_;
  QCheckBox *exported_;
  QCheckBox *plotted_;

  QComboBox *env_cb_;
  
  int cur_env_;
  double *values_;
  double *duration_;
  double *transition_;
  double *phase_shift_;
  double *noise_;
  
  QLineEdit *t_values_[2];
  QLineEdit *t_duration_[2];
  QLineEdit *t_transition_[2];
  QLineEdit *t_phase_shift_;
  QLineEdit *t_noise_;
  
  QLineEdit *period_;

  QMultiLineEdit *notes_;

  LineGraphView *lg_;

  QPushButton *apply_;
  
  bool changed_;
  bool topology_changed_;
};

#endif
