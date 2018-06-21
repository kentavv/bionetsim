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

#include <qdatetime.h>
#include <qstringlist.h>

#include <gsl/gsl_vector.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "evolve_params.h"
#include "individual.h"
#include "math_utils.h"
#include "pathway_loader.h"
#include "random.h"
#include "simplex.h"
#include "version.h"

extern EvolveParams ep;
extern Random rng;

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

#if 0
double
func(double x, double y) {
  //printf("%f %f\n", x, y);
  return 2 * (3.75 - sqrt(x*x + y*y)) + sin(1.55 * 3.14159 * (3.75 - sqrt(x*x + y*y)) * (6.0 - sqrt(x*x + y*y)) / 3.0);
}
#endif

#if 0
double
func(double x, double y) {
  return -x * sin(sqrt(fabs(x))) /* *cos(sqrt(fabs(x)))*/ - y * sin(sqrt(fabs(y)))/* *cos(sqrt(fabs(y)))*/;
}
#endif

#if 1
double
func(double x, double y) {
  return -((x*x - cos(2 * 3.14159 * x)) + (y*y - cos(2 * 3.14159 * y)));
}
#endif

double
Individual::fitness() const {
  if(!current_) {
    if(checkConstraints()) {
      const_cast<Individual*>(this)->f_ = -func(g_[0], g_[1]);
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

double
Individual::localAreaOptimization() {
  double diff = 0.0;
  if(!local_opt_converged_) {
    double score;
    local_opt_converged_ = do_simplex(g_, gl_, score);
    diff = f_ - score;
    f_ = score;
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
  return ((ep.maximize  && const_cast<Individual*>(this)->fitness() > const_cast<Individual&>(in).fitness()) ||
	  (!ep.maximize && const_cast<Individual*>(this)->fitness() < const_cast<Individual&>(in).fitness()));
}

bool
Individual::operator > (const Individual &in) const {
  return ((ep.maximize  && const_cast<Individual*>(this)->fitness() < const_cast<Individual&>(in).fitness()) ||
	  (!ep.maximize && const_cast<Individual*>(this)->fitness() > const_cast<Individual&>(in).fitness()));
}

void
Individual::mutate() {
  int i = rng.integer(0, gl_-1);
  //printf("C %f\n", g_[i]);
  g_[i] = my_random(i);
  //printf("D %f\n", g_[i]);
  enforceConstraints();
  //printf("E %f\n", g_[i]);
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
  //puts("hi");
  Individual c = *this;
  c.mutate();
  //c.print();
  //this->print();
  //printf("%f %f\n", c.fitness(), fitness());
  if(c.fitness() <= fitness()) {
    *this = c;
  }
}

void
Individual::helpfulShiftMutate() {
  Individual c = *this;
  c.shiftMutate();
  if(c.fitness() <= fitness()) {
    *this = c;
  }
}

void
Individual::print() const {
  printf("%f", fitness());
  for(int i=0; i<gl_; i++) {
    printf("\t%.2f", g_[i]);
  }
  putchar('\n');
}

bool
Individual::save(const char *filename) const {
  int i;
  int env = 0;

  extractKC(g_, gl_, ep.pem.kc);

  PathwayCanvasItem *p;
  for(i=0, p=ep.pem.items.first(); p; p=ep.pem.items.next()) {
    if(p->rtti() == MoleculeItem::RTTI) {
      MoleculeItem *q = (MoleculeItem*)p;
      if(q->decays()) {
	q->setDecayRate(ep.pem.kc(i,0));
	i++;
      }
    } else if(p->rtti() == MassActionReactionItem::RTTI) {
      MassActionReactionItem *q = (MassActionReactionItem*)p;
      q->setKf(ep.pem.kc(i,0));
      i++;
      if(q->reversible()) {      
	q->setKr(ep.pem.kc(i,0));
	i++;
      }
    }
  }
  ep.pem.setCurEnvironment(env);
  ep.pem.simulation.setCurEnvironment(env);

  if(!pathwaySave(filename, appname, appversion,
		  ep.pem.editor, ep.pem.simulation, ep.pem.items)) {
    return false;
  }

  return true;
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
  double ran = rng.real(ep.param_ranges(pos, 0), ep.param_ranges(pos, 1));
  //printf("A %f\n", ran);
  allele_range_check(pos, ran);
  //printf("B %f\n", ran);
  return ran;
}

void
Individual::extractKC(const double *v, int n, Matrix &kc) {
  //extractKC_Generic(v, n, kc);
  //extractKC_d038e(v, n, kc);
}

bool
Individual::checkConstraints() const {
  return checkConstraints_Generic();
  //return checkConstraints_d038e();
  //return checkConstraints_ft4();
}

void
Individual::enforceConstraints() {
  enforceConstraints_Generic();
  //enforceConstraints_d038e();
  //enforceConstraints_ft4();
}









void
Individual::extractKC_Generic(const double *v, int n, Matrix &kc) {
  kc.resize(n, 1);
  for(int i=0; i<n; i++) {
    kc(i, 0) = v[i];
  }
}

bool
Individual::checkConstraints_Generic() const {
  for(int i=0; i<gl_; i++) {
    if(     g_[i] < ep.param_ranges(i, 0)) return false;
    else if(g_[i] > ep.param_ranges(i, 1)) return false;
  }
  return true;
}

void
Individual::enforceConstraints_Generic() {
  bool c = false;
  for(int i=0; i<gl_; i++) {
    if(     g_[i] < ep.param_ranges(i, 0)) {
      g_[i] = ep.param_ranges(i, 0);
      c = true;
    } else if(g_[i] > ep.param_ranges(i, 1)) {
      g_[i] = ep.param_ranges(i, 1);
      c = true;
    }
  }
  if(c) setChanged();
}





#define CLAMP(x, y) if((x) < (y)) { (y) = 0.999 * (x); c = true; }

void
Individual::extractKC_d038e(const double *v, int, Matrix &kc) {
  kc( 0, 0) = v[ 0];
  kc( 1, 0) = v[ 1];
  kc( 2, 0) = v[ 2];
  kc( 3, 0) = v[ 3];
  kc( 4, 0) = v[ 4];
  kc( 5, 0) = v[ 5];
  kc( 6, 0) = v[ 6];
  kc( 7, 0) = v[ 7];
  kc( 8, 0) = v[ 8];
  kc( 9, 0) = v[ 9];
  kc(10, 0) = v[10];
  kc(11, 0) = v[11];
  kc(12, 0) = v[12];
  kc(13, 0) = v[13];
  kc(14, 0) = v[12];
  kc(15, 0) = v[13];
  kc(16, 0) = v[14];
  kc(17, 0) = v[15];
  kc(18, 0) = v[16];
  kc(19, 0) = v[17];  
}

bool
Individual::checkConstraints_d038e() const {
  if(!checkConstraints_Generic()) return false;

  // Degradations must be less than productions
  if(g_[ 4] < g_[ 0]) return false;
  if(g_[ 5] < g_[ 1]) return false;
  if(g_[ 8] < g_[ 2]) return false;
  if(g_[11] < g_[ 3]) return false;

  // Kr must be less than Kf
  if(g_[ 6] < g_[ 7]) return false;
  if(g_[ 9] < g_[10]) return false;
  if(g_[12] < g_[13]) return false;
  if(g_[14] < g_[15]) return false;
  if(g_[16] < g_[17]) return false;

  return true;
}

void
Individual::enforceConstraints_d038e() {
  bool c = false;
  enforceConstraints_Generic();
  
  CLAMP(g_[ 4], g_[ 0]);
  CLAMP(g_[ 5], g_[ 1]);
  CLAMP(g_[ 8], g_[ 2]);
  CLAMP(g_[11], g_[ 3]);
  
  CLAMP(g_[ 6], g_[ 7]);
  CLAMP(g_[ 9], g_[10]);
  CLAMP(g_[12], g_[13]);
  CLAMP(g_[14], g_[15]);
  CLAMP(g_[16], g_[17]);
  
  if(c) setChanged();
}




bool
Individual::checkConstraints_ft4() const {
  if(!checkConstraints_Generic()) return false;
  if(g_[10] < g_[ 0]) return false;
  if(g_[13] < g_[ 1]) return false;
  if(g_[16] < g_[ 2]) return false;
  if(g_[17] < g_[ 3]) return false;
  if(g_[20] < g_[ 4]) return false;
  if(g_[25] < g_[ 5]) return false;
  if(g_[28] < g_[ 6]) return false;
  if(g_[33] < g_[ 7]) return false;
  
  if(g_[ 8] < g_[ 9]) return false;
  if(g_[11] < g_[12]) return false;
  if(g_[14] < g_[15]) return false;
  if(g_[18] < g_[19]) return false;
  if(g_[21] < g_[22]) return false;
  if(g_[23] < g_[24]) return false;
  if(g_[26] < g_[27]) return false;
  if(g_[29] < g_[30]) return false;
  if(g_[31] < g_[32]) return false;
  if(g_[34] < g_[35]) return false;
  if(g_[36] < g_[37]) return false;
  if(g_[38] < g_[39]) return false;
  if(g_[40] < g_[41]) return false;
  if(g_[42] < g_[43]) return false;
  if(g_[44] < g_[45]) return false;

  return true;
}

void
Individual::enforceConstraints_ft4() {
  bool c = false;
  enforceConstraints_Generic();
  CLAMP(g_[10], g_[ 0]);
  CLAMP(g_[13], g_[ 1]);
  CLAMP(g_[16], g_[ 2]);
  CLAMP(g_[17], g_[ 3]);
  CLAMP(g_[20], g_[ 4]);
  CLAMP(g_[25], g_[ 5]);
  CLAMP(g_[28], g_[ 6]);
  CLAMP(g_[33], g_[ 7]);
  
  CLAMP(g_[ 8], g_[ 9]);
  CLAMP(g_[11], g_[12]);
  CLAMP(g_[14], g_[15]);
  CLAMP(g_[18], g_[19]);
  CLAMP(g_[21], g_[22]);
  CLAMP(g_[23], g_[24]);
  CLAMP(g_[26], g_[27]);
  CLAMP(g_[29], g_[30]);
  CLAMP(g_[31], g_[32]);
  CLAMP(g_[34], g_[35]);
  CLAMP(g_[36], g_[37]);
  CLAMP(g_[38], g_[39]);
  CLAMP(g_[40], g_[41]);
  CLAMP(g_[42], g_[43]);
  CLAMP(g_[44], g_[45]);

  if(c) setChanged();
}
