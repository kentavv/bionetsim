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

#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>

extern const int n_simulationMethodNames;
extern const char * const simulationMethodNames[];

class Simulation {
public:
  Simulation();
  Simulation(const Simulation &);
  ~Simulation();

  Simulation &operator=(const Simulation &);
  void clear();

  void save(QDomDocument &doc, QDomElement &n) const;

  int simulationMethod() const;
  void setSimulationMethod(int);
  void setSimulationMethod(const QString &);
  
  double timeBegin() const;
  void setTimeBegin(double);

  double timeEnd() const;
  void setTimeEnd(double);

  double timeStepSize() const;
  void setTimeStepSize(double);

  // Continuous options ---------
  bool simulateToSteadyState() const;
  void setSimulateToSteadyState(bool);

  double steadyStateTolerance() const;
  void setSteadyStateTolerance(double);
  
  double absoluteTolerance() const;
  void setAbsoluteTolerance(double);

  double relativeTolerance() const;
  void setRelativeTolerance(double);

  double maxStep() const;
  void setMaxStep(double);
  // ---------------------------
  
  // Stochastic options --------
  bool singleTrajectory() const;
  void setSingleTrajectory(bool);

  int numReplicates() const;
  void setNumReplicates(int);

  int histogramUpdateRate() const;
  void setHistogramUpdateRate(int);
  // ---------------------------
  
  double minStep() const;
  void setMinStep(double);

  QStringList environmentDescriptions() const;
  void setEnvironmentDescriptions(QStringList &);
  QString environmentLabel(int) const;

  int curEnvironment() const;
  void setCurEnvironment(int);
  void setCurEnvironment(const QString &);

  int numEnvironment() const;
  void setNumEnvironment(int);

  bool useRemoteCPU() const;
  void setUseRemoteCPU(bool);

  QString remoteCPU() const;
  void setRemoteCPU(const QString &);

  QString genReport() const;
  
private:
  int sim_method_;
  double t0_;
  double tend_;
  double tstep_;

  bool sim_to_ss_;
  double ss_tol_;
  
  double atol_;
  double rtol_;
  double max_step_;
  double min_step_;

  bool single_traj_;
  int nreps_;
  int hg_update_rate_;

  QStringList env_descriptions_;
  int cur_env_;

  bool use_remote_cpu_;
  QString remote_cpu_;
};

#endif
