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

#include <cfloat>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include <qdatetime.h>
#include <qstringlist.h>

#include <gsl/gsl_vector.h>

#include "calculate.h"
#include "evolve_params.h"
#include "individual.h"
#include "math_utils.h"
#include "net_utils.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "pathway_save.h"
#include "random.h"
#include "simplex.h"
#include "simulated_annealing.h"
#include "version.h"

extern EvolveParams ep;
extern Random rng;
extern QString param_filename;

int ff = 0;

Individual::Individual(int gl)
  : g_(NULL),
    gl_(0),
    f_(0.0),
    current_(false),
    local_opt_converged_(false)
{
  setGeneLength(gl);
}

Individual::Individual(const double *v, int gl)
  : g_(NULL),
    gl_(0),
    f_(0.0),
    current_(false),
    local_opt_converged_(false)
{
  load(v, gl);
}

Individual::Individual(const Individual &in)
  : g_(NULL),
    gl_(in.gl_),
    f_(in.f_),
    fl_(in.fl_),
    current_(in.current_),
    local_opt_converged_(in.local_opt_converged_)
{
  if(gl_ > 0) {
    g_ = new double[gl_];
    for(int i=0; i<gl_; i++) {
      g_[i] = in.g_[i];
    }
  }
}

Individual::~Individual() {
  if(g_) delete[] g_;
}

Individual &
Individual::operator = (const Individual &in) {
  if(g_) delete[] g_;
  g_ = NULL;

  gl_ = in.gl_;
  if(gl_ > 0) {
    g_ = new double[gl_];
    for(int i=0; i<gl_; i++) {
      g_[i] = in.g_[i];
    }
  }

  f_ = in.f_;
  fl_ = in.fl_;

  current_ = in.current_;
  local_opt_converged_ = in.local_opt_converged_;

  return *this;
}

int
Individual::geneLength() const {
  return gl_;
}

void
Individual::setGeneLength(int gl) {
  if(gl_ != gl) {
    if(g_) delete[] g_;
    g_ = NULL;
    
    gl_ = gl;
    if(gl_ > 0) {
      g_ = new double[gl_];
    }
    setChanged();
  }
}

double
Individual::operator()(int pos) const {
  assert(0 <= pos < gl_);
  return g_[pos];
}

double &
Individual::ref(int pos) {
  assert(0 <= pos < gl_);
  setChanged();
  return g_[pos];
}

void
Individual::load(const double *v, int gl) {
  setGeneLength(gl);
  for(int i=0; i<gl_; i++) {
    g_[i] = v[i];
  }
  setChanged();
}

void
Individual::random() {
  for(int i=0; i<gl_; i++) {
    g_[i] = my_random(i);
  }
  enforceConstraints();
  setChanged();
}

double
Individual::fitness() const {
  if(!current_) {
    if(checkConstraints()) {
      extractKC(g_, gl_, ep.pem.kc);

      if(!ep.fitness.calculate(ep.pem,
			       const_cast<Individual*>(this)->f_,
			       const_cast<Individual*>(this)->fl_)) {
	if(ep.maximize) {
	  const_cast<Individual*>(this)->f_ = -DBL_MAX;
	} else {
	  const_cast<Individual*>(this)->f_ = DBL_MAX;
	}
      }
    } else {
      const_cast<Individual*>(this)->f_ = ep.maximize ? -DBL_MAX : DBL_MAX;
    }
    const_cast<Individual*>(this)->current_ = true;
  }
  return f_;
}

double
Individual::localAreaOptimization() {
  return simplex();
}

double
Individual::simplex() {
  double diff = 0.0;
  if(!local_opt_converged_) {
    double score;
    local_opt_converged_ = do_simplex(g_, gl_, score);
    double pf = f_;
    enforceConstraints();  // Required especially if using integer parameters since simplex
                           // does not provide a method to restrict parameters to integers
    setChanged();
    fitness();
    diff = f_ - pf;
    current_ = true;
  }
  return diff;
}

double
Individual::simulatedAnnealing() {
  double diff = 0.0;
  if(!local_opt_converged_) {
    double score;
    do_sa(g_, gl_, score);
    local_opt_converged_ = false; // Never actually converges
    double pf = f_;
    enforceConstraints();  // Required especially if using integer parameters since simplex
                           // does not provide a method to restrict parameters to integers
    setChanged();
    fitness();
    diff = f_ - pf;
    current_ = true;
  }
  return diff;
}

bool
Individual::operator == (const Individual &in) const {
  if(gl_ != in.gl_) return false;
  for(int i=0; i<gl_; i++) {
    if(g_[i] != in.g_[i]) return false;
  }
  return true;
}

bool
Individual::operator != (const Individual &in) const {
  return !(*this == in);
}

bool
Individual::operator < (const Individual &in) const {
  return ((ep.maximize  && fitness() > in.fitness()) ||
	  (!ep.maximize && fitness() < in.fitness()));
}

bool
Individual::operator > (const Individual &in) const {
  return ((ep.maximize  && fitness() < in.fitness()) ||
	  (!ep.maximize && fitness() > in.fitness()));
}

void
Individual::mutate() {
  int i = rng.integer(0, gl_-1);
  g_[i] = my_random(i);
  enforceConstraints();
  setChanged();
}

void
Individual::shiftMutate() {
  int i = rng.integer(0, gl_-1);
  g_[i] *= 1.0 + rng.real(-1.0, 1.0) * ep.shift_mutation_range;
  allele_range_check(i, g_[i]);
  enforceConstraints();
  setChanged();
}

void
Individual::helpfulMutate() {
  Individual c = *this;
  c.mutate();
  if((ep.maximize  && c.fitness() >= fitness()) ||
     (!ep.maximize && c.fitness() <= fitness())) {
    *this = c;
  }
}

void
Individual::helpfulShiftMutate() {
  Individual c = *this;
  c.shiftMutate();
  if((ep.maximize  && c.fitness() >= fitness()) ||
     (!ep.maximize && c.fitness() <= fitness())) {
    *this = c;
  }
}

void
Individual::print() const {
  printf("%f\t", fitness());
  fl_.printVector();
  for(int i=0; i<gl_; i++) {
    printf("\t%.6f", g_[i]);
  }
  printf("\tkc:");
  extractKC(g_, gl_, ep.pem.kc);
  for(int i=0; i<ep.pem.kc.nrows(); i++) {
    printf("\t%.6f", ep.pem.kc(i, 0));
  }
  putchar('\n');
}

bool
Individual::save(const char *filename) const {
  int env = 0;

  extractKC(g_, gl_, ep.pem.kc);

  ep.pem.applyKCToItems();
  ep.pem.setCurEnvironment(env);
  ep.pem.simulation.setCurEnvironment(env);

  return pathwaySave(filename, appname, appversion,
		     ep.pem.editor, ep.fitness,
		     ep.pem.simulation, ep.pem.items);
}

bool
Individual::saveResults(const char *filename) const {
  extractKC(g_, gl_, ep.pem.kc);      
  Result result;
  run_simulation(ep.pem, result);
  FILE *file = fopen(filename, "w");
  if(!file) return false;
  result.m.print(file);
  fclose(file);
  return false;
}

void
Individual::setChanged() {
  current_ = false;
  local_opt_converged_ = false;
}

void
Individual::allele_range_check(int pos, double &allele) {
  //puts("-====================");
  //ep.param_ranges.print();
  if(     allele < ep.param_ranges(pos, 0)) allele = ep.param_ranges(pos, 0);
  else if(allele > ep.param_ranges(pos, 1)) allele = ep.param_ranges(pos, 1);
  if(ep.integer_parameters) {
    allele = roundToInt(allele);
  }
}

double
Individual::my_random(int pos) {
#if 0
  int ind = rng.integer(0, 10);
  double rand_v[11] = { 1e-5,
			1e-4,
			1e-3,
			1e-2,
			1e-1,
			1e+0,
			1e+1,
			1e+2,
			1e+3,
			1e+4,
			1e+5 };
					       
  double ran = rand_v[ind];
#else
  //double ran = rng.real(ep.param_ranges(pos, 0), ep.param_ranges(pos, 1));
  double ran = pow(10.0, rng.real(-5, 4));
#endif
  allele_range_check(pos, ran);
  return ran;
}

#define CLAMP(x, y) if((x) < (y)) { (y) = 0.999 * (x); c = true; }

void
Individual::extractKC(const double *v, int /*n*/, Matrix &kc) {
  //kc.resize(n, 1);
  int n = ep.pem.kc.nrows();
  for(int i=0; i<n; i++) {
    int ind = ep.kc2param_map(i, 0); 
    if(ind != -1) {
      kc(i, 0) = v[ind];
    }
  }
}

bool
Individual::checkConstraints() const {
  for(int i=0; i<gl_; i++) {
    if(     g_[i] < ep.param_ranges(i, 0)) return false;
    else if(g_[i] > ep.param_ranges(i, 1)) return false;
  }
  return true;
}

void
Individual::enforceConstraints() {
  bool c = false;
  for(int i=0; i<gl_; i++) {
    if(     g_[i] < ep.param_ranges(i, 0)) {
      g_[i] = ep.param_ranges(i, 0);
      c = true;
    } else if(g_[i] > ep.param_ranges(i, 1)) {
      g_[i] = ep.param_ranges(i, 1);
      c = true;
    }
    if(ep.integer_parameters) {
      if(fabs(g_[i] - (int)g_[i]) > 1e-6) {
        g_[i] = roundToInt(g_[i]);
        c = true;
      }
    }
  }
  if(c) setChanged();
}

bool
Individual::send(int fd) const {
  return (::send(fd, (void*)&gl_, sizeof(gl_)) &&
	  ::send(fd, (void*)&f_, sizeof(f_)) &&
	  ::send(fd, fl_) &&
	  ::send(fd, (void*)&current_, sizeof(current_)) &&
	  ::send(fd, (void*)&local_opt_converged_, sizeof(local_opt_converged_)) &&
	  ::send(fd, (void*)g_, sizeof(double)*gl_));
}

bool
Individual::receive(int fd) {
  int gl;
  if(!::receive(fd, (void*)&gl, sizeof(gl))) return false;
  setGeneLength(gl);
  return (::receive(fd, (void*)&f_, sizeof(f_)) &&
	  ::receive(fd, fl_) &&
	  ::receive(fd, (void*)&current_, sizeof(current_)) &&
	  ::receive(fd, (void*)&local_opt_converged_, sizeof(local_opt_converged_)) &&
	  ::receive(fd, (void*)g_, sizeof(double)*gl_));
}

