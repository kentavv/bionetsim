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
  return fitness(param_filename);
  
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
Individual::fitness(const QString &ff_name) const {
  if(!current_) {
    if(checkConstraints()) {
      extractKC(g_, gl_, ep.pem.kc);

      const_cast<Individual*>(this)->f_ = -1.0;

      Result result;

      double target[4*3] = { 43474.82, 44461.52, 47795.19, 53051.28,    // d133
			     427.4511, 550.8609, 13733.83, 235.2039,    // d038
			     28855.19, 1058.356, 769.8561, 633.8722  }; // d016

      double ft[4*3] = { -1.0, -1.0, -1.0, -1.0,
			 -1.0, -1.0, -1.0, -1.0,
			 -1.0, -1.0, -1.0, -1.0 };
      int d133_GFP_ind  = ep.pem.moleculeLookup("d133_GFP");
      int d038_GFP_ind  = ep.pem.moleculeLookup("d038_GFP");
      int d016_GFP_ind  = ep.pem.moleculeLookup("d016_GFP");

      if(d133_GFP_ind == -1 || d038_GFP_ind == -1 || d016_GFP_ind == -1) {
	abort();
      }

      {
	for(int i=0; i<4; i++) {
	  ep.pem.setCurEnvironment(i);
	  if(run_simulation(ep.pem, result)) {
	    ft[i+0*4] = result.m(result.m.nrows()-1, d133_GFP_ind);
	    ft[i+1*4] = result.m(result.m.nrows()-1, d038_GFP_ind);
	    ft[i+2*4] = result.m(result.m.nrows()-1, d016_GFP_ind);
	  }
	}
	
	if(ft[0+0*4] != -1 && ft[1+0*4] != -1 && ft[2+0*4] != -1 && ft[3+0*4] != -1 &&
	   ft[0+1*4] != -1 && ft[1+1*4] != -1 && ft[2+1*4] != -1 && ft[3+1*4] != -1 &&
	   ft[0+2*4] != -1 && ft[1+2*4] != -1 && ft[2+2*4] != -1 && ft[3+2*4] != -1) {
	  const_cast<Individual*>(this)->f_ = sqrt(sqr(ft[0+0*4] - target[0+0*4]) +
						   sqr(ft[1+0*4] - target[1+0*4]) +
						   sqr(ft[2+0*4] - target[2+0*4]) +
						   sqr(ft[3+0*4] - target[3+0*4]) +
						   sqr(ft[0+1*4] - target[0+1*4]) +
						   sqr(ft[1+1*4] - target[1+1*4]) +
						   sqr(ft[2+1*4] - target[2+1*4]) +
						   sqr(ft[3+1*4] - target[3+1*4]) +
						   sqr(ft[0+2*4] - target[0+2*4]) +
						   sqr(ft[1+2*4] - target[1+2*4]) +
						   sqr(ft[2+2*4] - target[2+2*4]) +
						   sqr(ft[3+2*4] - target[3+2*4]));
	}
	//printf("%f %f %f %f f:%f\n", ft[0], ft[1], ft[2], ft[3], f_);

	for(int i=0; i<4*3; i++) {
	  const_cast<Individual*>(this)->fl_(i/4,i%4) = ft[i];
	}
      }
      
      if(!ep.maximize && f_ == -1.0) {
	const_cast<Individual*>(this)->f_ = DBL_MAX;
      }
    } else {
      const_cast<Individual*>(this)->f_ = ep.maximize ? -DBL_MAX : DBL_MAX;
    }
    const_cast<Individual*>(this)->current_ = true;
  }
  return f_;
}

#if 0
double
Individual::fitness(const QString &ff_name) const {
  if(!current_) {
    if(checkConstraints()) {
      extractKC(g_, gl_, ep.pem.kc);

      const_cast<Individual*>(this)->f_ = -1.0;

      Result result;

      double target[4];

      setTarget(ff_name, target);

      double ft[4] = { -1.0, -1.0, -1.0, -1.0 };
      int GFP_ind  = ep.pem.moleculeLookup("GFP");

      if(GFP_ind == -1) {
	abort();
      }

      if(ff < 0) {
	ep.pem.setCurEnvironment(ff);
	if(run_simulation(ep.pem, result)) {
	  ft[ff] = result.m(result.m.nrows()-1, GFP_ind);
	}
	
	if(ft[ff] != -1) {
	  const_cast<Individual*>(this)->f_ = sqrt(sqr(ft[ff] - target[ff]));
	}
	//printf("%f %f %f %f f:%f\n", ft[0], ft[1], ft[2], ft[3], f_);
      } else if(1 || ff == 4) {
	for(int i=0; i<4; i++) {
	  ep.pem.setCurEnvironment(i);
	  if(run_simulation(ep.pem, result)) {
	    ft[i] = result.m(result.m.nrows()-1, GFP_ind);
	  }
	}
	
	if(ft[0] != -1 && ft[1] != -1 && ft[2] != -1 && ft[3] != -1) {
	  const_cast<Individual*>(this)->f_ = sqrt(sqr(ft[0] - target[0]) +
						   sqr(ft[1] - target[1]) +
						   sqr(ft[2] - target[2]) +
						   sqr(ft[3] - target[3]));
	}
	//printf("%f %f %f %f f:%f\n", ft[0], ft[1], ft[2], ft[3], f_);

	const_cast<Individual*>(this)->fl_[0] = ft[0];
	const_cast<Individual*>(this)->fl_[1] = ft[1];
	const_cast<Individual*>(this)->fl_[2] = ft[2];
	const_cast<Individual*>(this)->fl_[3] = ft[3];
      }
      
      if(!ep.maximize && f_ == -1.0) {
	const_cast<Individual*>(this)->f_ = DBL_MAX;
      }
    } else {
      const_cast<Individual*>(this)->f_ = ep.maximize ? -DBL_MAX : DBL_MAX;
    }
    const_cast<Individual*>(this)->current_ = true;
  }
  return f_;
}
#endif

void
Individual::setTarget(const QString &ff_name, double target[4]) const {
  if(ff_name == "d012.param") {
    target[0] = 9799.553; target[1] = 135.4443; target[2] = 384.7536; target[3] = 207.8155;
  } else if(ff_name == "d016.param") {
    target[0] = 28855.19; target[1] = 1058.356; target[2] = 769.8561; target[3] = 633.8722;
  } else if(ff_name == "d018.param") {
    target[0] = 5746.876; target[1] = 4954.329; target[2] = 6977.721; target[3] = 6682.387;
  } else if(ff_name == "d019.param") {
    target[0] = 331.4371; target[1] = 529.6587; target[2] = 10787.62; target[3] = 168.6702;
  } else if(ff_name == "d028.param") {
    target[0] = 571.3948; target[1] = 658.9516; target[2] = 284.0915; target[3] = 255.054;
  } else if(ff_name == "d032.param") {
    target[0] = 48101.94; target[1] = 1047.139; target[2] = 48895.48; target[3] = 737.5005;
  } else if(ff_name == "d038.param") {
    target[0] = 427.4511; target[1] = 550.8609; target[2] = 13733.83; target[3] = 235.2039;
  } else if(ff_name == "d052.param") {
    target[0] = 13557.13; target[1] = 467.7863; target[2] = 435.0814; target[3] = 686.2104;
  } else if(ff_name == "d066.param") {
    target[0] = 7853.719; target[1] = 1621.622; target[2] = 410.1682; target[3] = 435.9176;
  } else if(ff_name == "d078.param") {
    target[0] = 102.9218; target[1] = 146.2082; target[2] = 48589.24; target[3] = 407.9522;
  } else if(ff_name == "d090.param") {
    target[0] = 203.558; target[1] = 162.6739; target[2] = 92.84085; target[3] = 125.2698;
  } else if(ff_name == "d101.param") {
    target[0] = 38861.47; target[1] = 591.6231; target[2] = 4654.478; target[3] = 885.175;
  } else if(ff_name == "d104.param") {
    target[0] = 215.8781; target[1] = 210.1672; target[2] = 234.766; target[3] = 343.5589;
  } else if(ff_name == "d113.param") {
    target[0] = 35163.88; target[1] = 1836.717; target[2] = 38297.85; target[3] = 1125.72;
  } else if(ff_name == "d114.param") {
    target[0] = 218.8726; target[1] = 276.0236; target[2] = 446.396; target[3] = 416.1289;
  } else if(ff_name == "d117.param") {
    target[0] = 45398.87; target[1] = 46253.63; target[2] = 51533.72; target[3] = 52351.76;
  } else if(ff_name == "d123.param") {
    target[0] = 653.3381; target[1] = 866.5852; target[2] = 402.461; target[3] = 282.9939;
  } else if(ff_name == "d133.param") {
    target[0] = 43474.82; target[1] = 44461.52; target[2] = 47795.19; target[3] = 53051.28;
  } else if(ff_name == "d135.param") {
    target[0] = 6088.122; target[1] = 5654.586; target[2] = 5820.305; target[3] = 5765.542;
  } else if(ff_name == "d143.param") {
    target[0] = 14218.99; target[1] = 14124.03; target[2] = 568.3894; target[3] = 451.3978;
  } else if(ff_name == "d180.param") {
    target[0] = 47496.43; target[1] = 48890.79; target[2] = 791.2396; target[3] = 885.5921;
  } else if(ff_name == "d250.param") {
    target[0] = 795.7318; target[1] = 296.989; target[2] = 354.3591; target[3] = 182.9879;
  } else if(ff_name == "d253.param") {
    target[0] = 885.4587; target[1] = 799.1516; target[2] = 325.6498; target[3] = 204.3227;
  } else if(ff_name == "c024.param") {
    target[0] = 13922.28; target[1] = 439.0846; target[2] = 538.8381; target[3] = 345.9892;
  } else if(ff_name == "c101.param") {
    target[0] = 176.7035; target[1] = 9121.306; target[2] = 297.036; target[3] = 226.886;
  } else if(ff_name == "c103.param") {
    target[0] = 11305.01; target[1] = 2440.715; target[2] = 818.0481; target[3] = 653.8954;
  } else if(ff_name == "c113.param") {
    target[0] = 13454.04; target[1] = 1798.284; target[2] = 447.3287; target[3] = 530.8328;
  } else if(ff_name == "c144.param") {
    target[0] = 31623.44; target[1] = 4857.925; target[2] = 534.1963; target[3] = 656.0131;
  } else if(ff_name == "c195.param") {
    target[0] = 46271.42; target[1] = 18231.93; target[2] = 68665.37; target[3] = 56802.93;
  } else if(ff_name == "c242.param") {
    target[0] = 17403.9; target[1] = 715.8412; target[2] = 567.6562; target[3] = 442.334;
  } else {
    abort();
  }
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

