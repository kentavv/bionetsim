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
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qpushbutton.h>

#include "calculate.h"
#include "line_graph_view.h"
#include "matrix.h"
#include "pathway_canvas_items.h"
#include "result.h"
#include "squarewave_cv_dialog.h"

SquareWaveControlVariableDialog::SquareWaveControlVariableDialog(QWidget *parent,
								 const Simulation &simulation,
								 SquareWaveControlVariableItem &cv)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    cv_(cv),
    simulation_(simulation),
    values_(NULL),
    duration_(NULL),
    transition_(NULL),
    phase_shift_(NULL),
    noise_(NULL)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox, *hbox2;

  hbox = new QHBox(this);
  new QLabel("Control Variable Label: ", hbox);
  label_ = new QLineEdit(hbox);

  exported_ = new QCheckBox("Exported", this);
  plotted_ = new QCheckBox("Plotted", this);

  env_cb_ = new QComboBox(false, this);
  QGroupBox *group, *group2;

  group = new QGroupBox(2, Vertical, "Parameters", this);

  hbox2 = new QHBox(group);
  group2 = new QGroupBox(3, Vertical, "State One", hbox2);
  hbox = new QHBox(group2);
  new QLabel("Value: ", hbox);
  t_values_[0] = new QLineEdit(hbox);
  hbox = new QHBox(group2);
  new QLabel("Duration: ", hbox);
  t_duration_[0] = new QLineEdit(hbox);
  hbox = new QHBox(group2);
  new QLabel("Transition Time: ", hbox);
  t_transition_[0] = new QLineEdit(hbox);

  group2 = new QGroupBox(3, Vertical, "State Two", hbox2);
  hbox = new QHBox(group2);
  new QLabel("Value: ", hbox);
  t_values_[1] = new QLineEdit(hbox);
  hbox = new QHBox(group2);
  new QLabel("Duration: ", hbox);
  t_duration_[1] = new QLineEdit(hbox);
  hbox = new QHBox(group2);
  new QLabel("Transition Time: ", hbox);
  t_transition_[1] = new QLineEdit(hbox);

  hbox2 = new QHBox(group);
  hbox2->setSpacing(8);
  hbox = new QHBox(hbox2);
  new QLabel("Phase Shift: ", hbox);
  t_phase_shift_ = new QLineEdit(hbox);

  hbox = new QHBox(hbox2);
  new QLabel("Gaussian Noise (std. dev.): ", hbox);
  t_noise_ = new QLineEdit(hbox);

  lg_ = new LineGraphView(this);
  lg_->setFixedHeight(200);
  
  hbox = new QHBox(hbox2);
  new QLabel("Period: ", hbox);
  period_ = new QLineEdit(hbox);
  period_->setReadOnly(true);

  new QLabel("Notes:", this);
  notes_ = new QMultiLineEdit(this);

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);

  updateDialog(simulation);
  
  connect(label_, SIGNAL(textChanged(const QString &)), SLOT(setChanged()));
  connect(exported_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(plotted_, SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(t_values_[0],     SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_duration_[0],   SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_transition_[0], SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_values_[1],     SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_duration_[1],   SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_transition_[1], SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_phase_shift_,   SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(t_noise_,         SIGNAL(textChanged(const QString &)), SLOT(setTopologyChanged()));
  connect(notes_, SIGNAL(textChanged()), SLOT(setChanged()));

  connect(label_,           SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_values_[0],     SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_duration_[0],   SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_transition_[0], SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_values_[1],     SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_duration_[1],   SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_transition_[1], SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_phase_shift_,   SIGNAL(returnPressed()), SLOT(apply()));
  connect(t_noise_,         SIGNAL(returnPressed()), SLOT(apply()));
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  connect(env_cb_, SIGNAL(activated(int)), SLOT(changeEnv(int)));

  connect(t_values_[0],     SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_duration_[0],   SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_transition_[0], SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_values_[1],     SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_duration_[1],   SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_transition_[1], SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_phase_shift_,   SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));
  connect(t_noise_,         SIGNAL(textChanged(const QString &)), SLOT(updateGraph()));

  updateGraph();
}

SquareWaveControlVariableDialog::~SquareWaveControlVariableDialog() {
  if(values_)      delete[] values_;
  if(duration_)    delete[] duration_;
  if(transition_)  delete[] transition_;
  if(phase_shift_) delete[] phase_shift_;
  if(noise_)       delete[] noise_;
}

void
SquareWaveControlVariableDialog::updateItem() {
  cv_.setLabel(label_->text());
  cv_.setExported(exported_->isChecked());
  cv_.setPlotted(plotted_->isChecked());
  
  for(int i=0; i<2; i++) {
    values_[cur_env_*2 + i]     = t_values_[i]->text().toDouble();
    duration_[cur_env_*2 + i]   = t_duration_[i]->text().toDouble();
    transition_[cur_env_*2 + i] = t_transition_[i]->text().toDouble();
  }
  phase_shift_[cur_env_]        = t_phase_shift_->text().toDouble();
  noise_[cur_env_]              = t_noise_->text().toDouble();

  cv_.setParameters(values_, duration_, transition_, phase_shift_, noise_);
  
  cv_.setNotes(notes_->text());
}

void
SquareWaveControlVariableDialog::updateDialog(const Simulation &simulation) {
  simulation_ = simulation;

  setCaption(cv_.caption());
  label_->setText(cv_.label());
  
  exported_->setChecked(cv_.exported());
  plotted_->setChecked(cv_.plotted());

  int i, ne = simulation.numEnvironment();
  if(values_)      delete[] values_;
  values_        = new double[ne*2];
  if(duration_)    delete[] duration_;
  duration_      = new double[ne*2];
  if(transition_)  delete[] transition_;
  transition_    = new double[ne*2];
  if(phase_shift_) delete[] phase_shift_;
  phase_shift_   = new double[ne];
  if(noise_)       delete[] noise_;
  noise_         = new double[ne];
  cv_.parameters(values_, duration_, transition_, phase_shift_, noise_);
  env_cb_->clear();
  env_cb_->insertStringList(simulation.environmentDescriptions());

  cur_env_ = simulation.curEnvironment();
  env_cb_->setCurrentItem(cur_env_);
  simulation_.setCurEnvironment(0);

  for(i=0; i<2; i++) {
    t_values_[i]->setText(tr("%1").arg(values_[cur_env_*2 + i]));
    t_duration_[i]->setText(tr("%1").arg(duration_[cur_env_*2 + i]));
    t_transition_[i]->setText(tr("%1").arg(transition_[cur_env_*2 + i]));
  }
  t_phase_shift_->setText(tr("%1").arg(phase_shift_[cur_env_]));
  t_noise_->setText(tr("%1").arg(noise_[cur_env_]));

  notes_->setText(cv_.notes());

  updateGraph();
}

void
SquareWaveControlVariableDialog::changeEnv(int env) {
  for(int i=0; i<2; i++) {
    values_[cur_env_*2 + i]     = t_values_[i]->text().toDouble();
    duration_[cur_env_*2 + i]   = t_duration_[i]->text().toDouble();
    transition_[cur_env_*2 + i] = t_transition_[i]->text().toDouble();

    t_values_[i]->setText(tr("%1").arg(values_[env*2 + i]));
    t_duration_[i]->setText(tr("%1").arg(duration_[env*2 + i]));
    t_transition_[i]->setText(tr("%1").arg(transition_[env*2 + i]));
  }
  t_phase_shift_->setText(tr("%1").arg(phase_shift_[env]));
  t_noise_->setText(tr("%1").arg(noise_[env]));
  
  cur_env_ = env;
  updateGraph();
}

void
SquareWaveControlVariableDialog::updateGraph() {
  double values[2]     = { t_values_[0]->text().toDouble(),
			   t_values_[1]->text().toDouble() };
  double duration[2]   = { t_duration_[0]->text().toDouble(),
			   t_duration_[1]->text().toDouble() };
  double transition[2] = { t_transition_[0]->text().toDouble(),
			   t_transition_[1]->text().toDouble() };
  double phase_shift   =   t_phase_shift_->text().toDouble();  
  double noise         =   t_noise_->text().toDouble();
  
  double period = duration[0] + transition[0] + duration[1] + transition[1];

  period_->setText(tr("%1").arg(period));

  SquareWaveControlVariableItem *cv = new SquareWaveControlVariableItem(NULL);
  cv->setValues(values[0], values[1]);
  cv->setDuration(duration[0], duration[1]);
  cv->setTransitionTime(transition[0], transition[1]);
  cv->setPhaseShift(phase_shift);
  cv->setNoise(noise);

  QList<PathwayCanvasItem> items;
  items.setAutoDelete(true);
  items.append(cv);

  Editor editor;
  PathwayMatrix pem;
  pem.setup(editor, simulation_, items);
  pem.simulation.setCurEnvironment(0);
  pem.simulation.setNumEnvironment(1);
  pem.simulation.setSimulationMethod(0);
  
  Result results;
  if(run_simulation(pem, results)) {
    lg_->setData(results);
    lg_->setGraphVisible(0, true);
  }
}

void
SquareWaveControlVariableDialog::apply() {
  if(topology_changed_) {
    cv_.updatePropertiesFromDialog();
    emit topologyChanged();
  } else if(changed_) {
    cv_.updatePropertiesFromDialog();
    emit changed();
  }
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
SquareWaveControlVariableDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
SquareWaveControlVariableDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
