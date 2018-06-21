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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include "simulation_params_dialog.h"
#include "fitness_functions.h"

SimulationParametersDialog::SimulationParametersDialog(QWidget *parent,
						       const Simulation &simulation)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    changed_(false),
    topology_changed_(false)
{
  QVBoxLayout *vbox = new QVBoxLayout(this,9);
  vbox->setAutoAdd(true);

  QHBox *hbox;
  QLabel *label;
  QGroupBox *group;
  
  hbox = new QHBox(this);
  label = new QLabel("Simulation Method: ", hbox);
  label->setFixedSize(label->sizeHint());
  sim_method_ = new QComboBox(hbox);

  {
    group = new QGroupBox(4, Horizontal, "Time", this);
    label = new QLabel("Start: ", group);
    label->setFixedSize(label->sizeHint());
    t0_ = new QLineEdit(group);
    label = new QLabel("  End: ", group);
    label->setFixedSize(label->sizeHint());
    tend_ = new QLineEdit(group);
    label = new QLabel("Sampling Rate: ", group);
    label->setFixedSize(label->sizeHint());
    tstep_ = new QLineEdit(group);
  }

  {
    QGroupBox *calculate_options = new QGroupBox(1, Horizontal, "Calculation Engine", this);

    calculate_local_ = new QRadioButton("Use local CPU", calculate_options);
    calculate_remote_ = new QRadioButton("Use remote CPU", calculate_options);

    QButtonGroup *rg = new QButtonGroup(calculate_options);
    rg->insert(calculate_local_, 0);
    rg->insert(calculate_remote_, 1);
    connect(rg, SIGNAL(clicked(int)), SLOT(calculateChecked(int)));
    rg->hide();
    
    hbox = new QHBox(calculate_options);
    label = new QLabel("      Address of remote CPU: ", hbox);
    label->setFixedSize(label->sizeHint());
    remote_cpu_ = new QLineEdit(hbox);
  }
  
  {
    cont_options_ = new QGroupBox(4, Vertical, "Continuous Options", this);

    sim_to_ss_ = new QCheckBox("Simulate to Steady State", cont_options_);
    connect(sim_to_ss_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
    
    hbox = new QHBox(cont_options_);
    label = new QLabel("      Steady state tolerance: ", hbox);
    label->setFixedSize(label->sizeHint());
    ss_tol_ = new QLineEdit(hbox);
    
    group = new QGroupBox(4, Horizontal, "Tolerances", cont_options_);
    label = new QLabel("Absolute: ", group);
    label->setFixedSize(label->sizeHint());
    atol_ = new QLineEdit(group);
    label = new QLabel("  Relative: ", group);
    label->setFixedSize(label->sizeHint());
    rtol_ = new QLineEdit(group);
    
    group = new QGroupBox(4, Horizontal, "Step Sizes", cont_options_);
    label = new QLabel("Minimum: ", group);
    label->setFixedSize(label->sizeHint());
    min_step_ = new QLineEdit(group);
    label = new QLabel("  Maximum: ", group);
    label->setFixedSize(label->sizeHint());
    max_step_ = new QLineEdit(group);
  }
  
  {
    disc_options_ = new QGroupBox(1, Horizontal, "Discrete Options", this);

    single_traj_ = new QRadioButton("Single trajectory", disc_options_);
    multiple_traj_ = new QRadioButton("Multiple trajectories", disc_options_);

    QButtonGroup *rg = new QButtonGroup(disc_options_);
    rg->insert(single_traj_, 0);
    rg->insert(multiple_traj_, 1);
    connect(rg, SIGNAL(clicked(int)), SLOT(trajectoryChecked(int)));
    rg->hide();
    
    hbox = new QHBox(disc_options_);
    label = new QLabel("      Number of replicates: ", hbox);
    label->setFixedSize(label->sizeHint());
    nreps_ = new QLineEdit(hbox);

    hbox = new QHBox(disc_options_);
    label = new QLabel("      Histogram update rate: ", hbox);
    label->setFixedSize(label->sizeHint());
    hg_update_rate_ = new QLineEdit(hbox);
  }
  
  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);
  
  setValues(simulation);

  connect(sim_method_, SIGNAL(activated(int)), SLOT(setTopologyChanged()));
  connect(t0_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(tend_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(tstep_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(ss_tol_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(atol_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(rtol_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(max_step_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(min_step_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(single_traj_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(multiple_traj_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(nreps_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(hg_update_rate_, SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(calculate_local_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(calculate_remote_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(remote_cpu_, SIGNAL(textChanged(const QString &)), SLOT(setChanged()));

  connect(t0_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(tend_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(tstep_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(ss_tol_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(atol_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(rtol_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(max_step_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(min_step_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(nreps_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(hg_update_rate_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(remote_cpu_, SIGNAL(returnPressed()), SLOT(apply()));
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  connect(sim_method_, SIGNAL(activated(const QString&)), SLOT(methodChanged(const QString&)));

  methodChanged(sim_method_->currentText());
}

void
SimulationParametersDialog::getValues(Simulation &simulation) {
  simulation.setSimulationMethod(sim_method_->currentItem());
  simulation.setTimeBegin(t0_->text().toDouble());
  simulation.setTimeEnd(tend_->text().toDouble());
  simulation.setTimeStepSize(tstep_->text().toDouble());

  simulation.setSimulateToSteadyState(sim_to_ss_->isChecked());
  simulation.setSteadyStateTolerance(ss_tol_->text().toDouble());
  simulation.setAbsoluteTolerance(atol_->text().toDouble());
  simulation.setRelativeTolerance(rtol_->text().toDouble());
  simulation.setMaxStep(max_step_->text().toDouble());
  simulation.setMinStep(min_step_->text().toDouble());

  simulation.setSingleTrajectory(single_traj_->isChecked());
  simulation.setNumReplicates(nreps_->text().toUInt());
  simulation.setHistogramUpdateRate(hg_update_rate_->text().toInt());  

  simulation.setUseRemoteCPU(calculate_remote_->isChecked());
  simulation.setRemoteCPU(remote_cpu_->text());
}

void
SimulationParametersDialog::setValues(const Simulation &simulation) {
  int i;
  
  sim_method_->clear();
  for(i=0; i<n_simulationMethodNames; i++) {
    sim_method_->insertItem(simulationMethodNames[i], i);
  }
  sim_method_->setCurrentItem(simulation.simulationMethod());
  t0_->setText(tr("%1").arg(simulation.timeBegin()));
  tend_->setText(tr("%1").arg(simulation.timeEnd()));
  tstep_->setText(tr("%1").arg(simulation.timeStepSize()));

  sim_to_ss_->setChecked(simulation.simulateToSteadyState());
  ss_tol_->setText(tr("%1").arg(simulation.steadyStateTolerance()));
  atol_->setText(tr("%1").arg(simulation.absoluteTolerance()));
  rtol_->setText(tr("%1").arg(simulation.relativeTolerance()));
  min_step_->setText(tr("%1").arg(simulation.minStep()));
  max_step_->setText(tr("%1").arg(simulation.maxStep()));

  single_traj_->setChecked(simulation.singleTrajectory());
  multiple_traj_->setChecked(!simulation.singleTrajectory());

  nreps_->setText(tr("%1").arg(simulation.numReplicates()));
  hg_update_rate_->setText(tr("%1").arg(simulation.histogramUpdateRate()));

  nreps_->setEnabled(multiple_traj_->isChecked());
  hg_update_rate_->setEnabled(multiple_traj_->isChecked());

  calculate_local_->setChecked(!simulation.useRemoteCPU());
  calculate_remote_->setChecked(simulation.useRemoteCPU());

  remote_cpu_->setText(tr("%1").arg(simulation.remoteCPU()));

  remote_cpu_->setEnabled(calculate_remote_->isChecked());
}

void
SimulationParametersDialog::methodChanged(const QString &method) {
  if(method == "ODE") {
    cont_options_->setEnabled(true);
    disc_options_->setEnabled(false);
  } else {
    cont_options_->setEnabled(false);
    disc_options_->setEnabled(true);
  }
}

void
SimulationParametersDialog::trajectoryChecked(int id) {
  nreps_->setEnabled(id == 1);
  hg_update_rate_->setEnabled(id == 1);
}

void
SimulationParametersDialog::calculateChecked(int id) {
  remote_cpu_->setEnabled(id == 1);
}

void
SimulationParametersDialog::apply() {
  if(topology_changed_) emit topologyChanged();
  else if(changed_)     emit changed();
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
SimulationParametersDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
SimulationParametersDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
