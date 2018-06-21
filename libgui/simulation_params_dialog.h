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

#ifndef _SIMULATION_PARAMS_DIALOG_H_
#define _SIMULATION_PARAMS_DIALOG_H_

#include <qdialog.h>

#include "simulation.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QRadioButton;

class SimulationParametersDialog : public QDialog {
  Q_OBJECT
public:
  SimulationParametersDialog(QWidget *parent,
			     const Simulation &simulation);

  void getValues(Simulation &simulation);
  void setValues(const Simulation &simulation);

public slots:
  void methodChanged(const QString&);
  void trajectoryChecked(int);
  void calculateChecked(int);
 
signals:
  void changed();
  void topologyChanged();
  
protected slots:
  void apply();
  void setChanged();
  void setTopologyChanged();
  
protected:
  QComboBox *sim_method_;
  QLineEdit *t0_, *tend_, *tstep_;

  QGroupBox *cont_options_;
  QCheckBox *sim_to_ss_;
  QLineEdit *ss_tol_;
  QLineEdit *atol_, *rtol_;
  QLineEdit *max_step_, *min_step_;

  QGroupBox *disc_options_;
  QRadioButton *single_traj_;
  QRadioButton *multiple_traj_;
  QLineEdit *nreps_;
  QLineEdit *hg_update_rate_;

  QRadioButton *calculate_local_;
  QRadioButton *calculate_remote_;
  QLineEdit *remote_cpu_;

  QPushButton *apply_;
  
  bool changed_;
  bool topology_changed_;
};

#endif
