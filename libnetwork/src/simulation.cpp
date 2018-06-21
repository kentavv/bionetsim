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

#include <qobject.h>

#include "simulation.h"
#include "log.h"
#include "fitness_functions.h"
#include "simulation.h"

Simulation::Simulation()
  : sim_method_(0),
    t0_(0.0),
    tend_(160.0),
    tstep_(0.1),
    sim_to_ss_(false),
    ss_tol_(1e-6),
    atol_(1e-8),
    rtol_(1e-4),
    max_step_(1.0),
    min_step_(0.0),
    single_traj_(false),
    nreps_(20),
    hg_update_rate_(-1),
    cur_env_(0),
    use_remote_cpu_(false),
    remote_cpu_("127.0.0.1")
{
  // Setup an initial environment, required if a new network is being built
  setNumEnvironment(1);
}

Simulation::Simulation(const Simulation &s)
  : sim_method_(s.sim_method_),
    t0_(s.t0_),
    tend_(s.tend_),
    tstep_(s.tstep_),
    sim_to_ss_(s.sim_to_ss_),
    ss_tol_(s.ss_tol_),
    atol_(s.atol_),
    rtol_(s.rtol_),
    max_step_(s.max_step_),
    min_step_(s.min_step_),
    single_traj_(s.single_traj_),
    nreps_(s.nreps_),
    hg_update_rate_(s.hg_update_rate_),
    env_descriptions_(s.env_descriptions_),
    cur_env_(s.cur_env_),
    use_remote_cpu_(s.use_remote_cpu_),
    remote_cpu_(s.remote_cpu_)
{
}

Simulation::~Simulation() {
}

Simulation &
Simulation::operator=(const Simulation &s) {
  sim_method_ = s.sim_method_;
  t0_ = s.t0_;
  tend_ = s.tend_;
  tstep_ = s.tstep_;
  sim_to_ss_ = s.sim_to_ss_;
  ss_tol_ = s.ss_tol_;
  atol_ = s.atol_;
  rtol_ = s.rtol_;
  max_step_ = s.max_step_;
  min_step_ = s.min_step_;
  single_traj_ = s.single_traj_;
  nreps_ = s.nreps_;
  hg_update_rate_ = s.hg_update_rate_;
  env_descriptions_ = s.env_descriptions_;
  cur_env_ = s.cur_env_;
  use_remote_cpu_ = s.use_remote_cpu_;
  remote_cpu_ = s.remote_cpu_;

  return *this;
}

void
Simulation::clear() {
  env_descriptions_.clear();
}

double
Simulation::timeBegin() const {
  return t0_;
}

void
Simulation::setTimeBegin(double v) {
  t0_ = v;
}

double
Simulation::timeEnd() const {
  return tend_;
}

void
Simulation::setTimeEnd(double v) {
  tend_ = v;
}

double
Simulation::timeStepSize() const {
  return tstep_;
}

void
Simulation::setTimeStepSize(double v) {
  tstep_ = v;
}

bool
Simulation::simulateToSteadyState() const {
  return sim_to_ss_;
}

void
Simulation::setSimulateToSteadyState(bool v) {
  sim_to_ss_ = v;
}

double
Simulation::steadyStateTolerance() const {
  return ss_tol_;
}

void
Simulation::setSteadyStateTolerance(double v) {
  ss_tol_= v;
}

double
Simulation::absoluteTolerance() const {
  return atol_;
}

void
Simulation::setAbsoluteTolerance(double v) {
  atol_ = v;
}

double
Simulation::relativeTolerance() const {
  return rtol_;
}

void
Simulation::setRelativeTolerance(double v) {
  rtol_ = v;
}

double
Simulation::maxStep() const {
  return max_step_;
}

void
Simulation::setMaxStep(double v) {
  max_step_ = v;
}

double
Simulation::minStep() const {
  return min_step_;
}
  
void
Simulation::setMinStep(double v) {
  min_step_ = v;
}

bool
Simulation::singleTrajectory() const {
  return single_traj_;
}

void
Simulation::setSingleTrajectory(bool v) {
  single_traj_ = v;
}

int
Simulation::numReplicates() const {
  return nreps_;
}
  
void
Simulation::setNumReplicates(int v) {
  nreps_ = v;
}

int
Simulation::histogramUpdateRate() const {
  return hg_update_rate_;
}

void
Simulation::setHistogramUpdateRate(int v) {
  hg_update_rate_ = v;
}

QStringList
Simulation::environmentDescriptions() const {
  return env_descriptions_;
}

void
Simulation::setEnvironmentDescriptions(QStringList &s) {
  env_descriptions_ = s;
  setCurEnvironment(curEnvironment());
}

QString 
Simulation::environmentLabel(int i) const {
  if(i < 0 || i >= env_descriptions_.count()) return "None Existent";
  return *env_descriptions_.at(i);
}

int
Simulation::curEnvironment() const {
  return cur_env_;
}
  
void
Simulation::setCurEnvironment(int v) {
  if(v < 0) v = 0;
  if(v > numEnvironment()) v = numEnvironment() - 1;
  cur_env_ = v;
}
  
void
Simulation::setCurEnvironment(const QString &str) {
  int ind = env_descriptions_.findIndex(str);
  if(ind != -1) {
    setCurEnvironment(ind);
  }
}
  
int
Simulation::numEnvironment() const {
  return env_descriptions_.count();
}
  
void
Simulation::setNumEnvironment(int v) {
  int n = env_descriptions_.count();
  if(v < n) {
    for(int i=n-1; i>=v; i--) {
      env_descriptions_.pop_back();
    }
  } else if(v > n) {
    for(int i=n; i<v; i++) {
      env_descriptions_.push_back(QObject::tr("Environment %1").arg(i+1));
    }
  }
}
  
int
Simulation::simulationMethod() const {
  return sim_method_;
}

void
Simulation::setSimulationMethod(int v) {
  sim_method_ = v;
}

void
Simulation::setSimulationMethod(const QString &str) {
  int i;
  for(i=0; i<n_simulationMethodNames; i++) {
    if(str == simulationMethodNames[i]) break;
  }
  if(i < n_simulationMethodNames) {
    setSimulationMethod(i);
  } else {
    pneDebug("Unknown simulation method: " + str);
  }
}

bool
Simulation::useRemoteCPU() const {
  return use_remote_cpu_;
}

void
Simulation::setUseRemoteCPU(bool v) {
  use_remote_cpu_ = v;
}

QString
Simulation::remoteCPU() const {
  return remote_cpu_;
}

void
Simulation::setRemoteCPU(const QString &v) {
  remote_cpu_ = v;
}

void
Simulation::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2;
  QDomText t;

  QDomElement sim = doc.createElement("Simulation");
  n.appendChild(sim);

  e = doc.createElement("SimulationMethod");
  t = doc.createTextNode(simulationMethodNames[sim_method_]);
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("TimeBegin");
  t = doc.createTextNode(QObject::tr("%1").arg(t0_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("TimeEnd");
  t = doc.createTextNode(QObject::tr("%1").arg(tend_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("TimeStepSize");
  t = doc.createTextNode(QObject::tr("%1").arg(tstep_));
  sim.appendChild(e); e.appendChild(t);

  if(sim_to_ss_) {
    e = doc.createElement("SimulateToSteadyState");
    sim.appendChild(e);
  }

  e = doc.createElement("SteadyStateTolerance");
  t = doc.createTextNode(QObject::tr("%1").arg(ss_tol_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("AbsoluteTolerance");
  t = doc.createTextNode(QObject::tr("%1").arg(atol_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("RelativeTolerance");
  t = doc.createTextNode(QObject::tr("%1").arg(rtol_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("MaxStep");
  t = doc.createTextNode(QObject::tr("%1").arg(max_step_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("MinStep");
  t = doc.createTextNode(QObject::tr("%1").arg(min_step_));
  sim.appendChild(e); e.appendChild(t);

  if(single_traj_) {
    e = doc.createElement("SingleTrajectory");
    sim.appendChild(e);
  }

  if(use_remote_cpu_) {
    e = doc.createElement("UseRemoteCPU");
    sim.appendChild(e);
  }

  e = doc.createElement("RemoteCPU");
  t = doc.createTextNode(remote_cpu_);
  sim.appendChild(e); e.appendChild(t);

  {
    e = doc.createElement("Environments");
    sim.appendChild(e);
    
    int ne = env_descriptions_.count();
    for(int i=0; i<ne; i++) {
      e2 = doc.createElement("Environment");
      if(i == cur_env_) e2.setAttribute("current", "true");
      t = doc.createTextNode(env_descriptions_[i]);
      e.appendChild(e2); e2.appendChild(t);
    }
  }

  e = doc.createElement("NumReplicates");
  t = doc.createTextNode(QObject::tr("%1").arg(nreps_));
  sim.appendChild(e); e.appendChild(t);

  e = doc.createElement("HistogramUpdateRate");
  t = doc.createTextNode(QObject::tr("%1").arg(hg_update_rate_));
  sim.appendChild(e); e.appendChild(t);
}

QString
Simulation::genReport() const {
  QString text;

  text.sprintf("Simulation:\n"
	       "  Simulation Method: %s (%d)\n"
	       "  Time Interval: [%g,%g]\n"
	       "  Sampling Rate: %g\n"
	       "  Absolute Tolerance: %g\n"
	       "  Relative Tolerance: %g\n"
	       "  Max ODE Step Size: %g\n"
	       "  Min ODE Step Size: %g\n"
	       "  Single Trajectory: %s\n"
	       "  Number of Replicates: %d\n"
	       "  Histogram Update Rate: %d\n"
	       "  Use remote CPU: %s\n"
	       "  Remote CPU address: %s\n"
	       "  Current Environment: %s [%d (of %d total)]\n",
	       simulationMethodNames[sim_method_], sim_method_,
	       t0_, tend_, tstep_,
	       atol_, rtol_,
	       max_step_, min_step_,
	       single_traj_ ? "true" : "false",
	       nreps_, hg_update_rate_,
	       use_remote_cpu_ ? "true" : "false",
	       (const char*)remote_cpu_,
	       (const char *)env_descriptions_[cur_env_],
	       cur_env_+1, numEnvironment());
  
  return text;
}
