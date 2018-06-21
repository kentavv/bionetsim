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

#include <stdlib.h>

#include "evolve_params.h"
#include "individual.h"
#include "math_utils.h"
#include "population.h"
#include "random.h"

extern EvolveParams ep;
extern Random rng;

Population::Population(int n, int gl)
  : p_(NULL),
    p2_(NULL),
    n_(0),
    gl_(0)
{
  resize(n, gl);
}

Population::Population(const Population &p)
  : p_(NULL),
    p2_(NULL),
    n_(0),
    gl_(0)
{
  resize(p.n_, p.gl_);
  for(int i=0; i<n_; i++) {
    p_[i] = p.p_[i];
  }
}

Population::~Population() {
  if(p_) delete[] p_;
  if(p2_) delete[] p2_;
}

int
Population::size() const {
  return n_;
}

void
Population::resize(int n, int gl) {
  if(n_ != n || gl_ != gl) {
    if(p_) delete[] p_;
    p_ = NULL;

    if(p2_) delete[] p2_;
    p2_ = NULL;

    n_ = n;
    gl_ = gl;
    p_ = new Individual[n_];
    for(int i=0; i<n_; i++) {
      p_[i].setGeneLength(gl_);
    }
    random();
  }
}

void
Population::random() {
  for(int i=0; i<n_; i++) {
    p_[i].random();
  }
}

int
Population::individualCompare(const void *a, const void *b) {
  const Individual &in1=*(const Individual *)a;
  const Individual &in2=*(const Individual *)b;

  if(in1 < in2) return -1;
  if(in1 > in2) return 1;
  return 0;
}

void
Population::sort() {
  qsort(p_, n_, sizeof(Individual), individualCompare);
}

const Individual &
Population::operator()(int pos) const {
  assert(0 <= pos && pos < n_);
  return p_[pos];
}

Individual &
Population::operator()(int pos) {
  assert(0 <= pos && pos < n_);
  return p_[pos];
}

void
Population::print() const {
  for(int i=0; i<n_; i++) {
    p_[i].print();
  }
}

void
Population::printStats() const {
  double sum = 0.0;
  double min = DBL_MAX;
  double max = -DBL_MAX;
  for(int i=0; i<n_; i++) {
    sum += p_[i].fitness();
    if(min > p_[i].fitness()) min = p_[i].fitness();
    if(max < p_[i].fitness()) max = p_[i].fitness();
  }
  double avg = sum / ep.pop_size;
  printf("Fitness Avg:%f\tMin:%f\tMax:%f\n", avg, min, max);
}

void
Population::printAverage() const {
  Matrix sums(gl_+1, 1, 0.0);
  int i, j;
  
  for(i=0; i<n_; i++) {
    sums(0, 0) += p_[i].fitness();
    for(j=0; j<gl_; j++) {
      sums(j+1, 0) += p_[i](j);
    }
  }

  printf("%f", sums(0, 0)/n_);
  for(j=0; j<gl_; j++) {
    printf("\t%.2f", sums(j+1, 0)/n_);
  }
  putchar('\n');
}

void
Population::prune() {
  int i, j, n;
  IMatrix identical(n_, 1, 0);
  for(n=0, i=0, j=1; j<n_; j++) {
    if(p_[i] == p_[j]) {
      //printf("%d %d identical\n", i, j);
      identical(j, 0) = 1;
      n++;
    } else {
      i=j;
    }
  }
  double per = n / (double) ep.pop_size;
  //printf("%f of pop identical\n", per);
  if(per > ep.population_identity_limit) {
    puts("Genetic explosion");
    for(i=0; i<n_; i++) {
      if(identical(i, 0)) {
	p_[i].random();
      }
    }
  }
}

void
Population::crossover() {
  int i;
  switch(ep.crossover) {
  case EvolveParams::None:
    break;
  case EvolveParams::TwoPt:
    for(i=0; i<n_/2; i+=2) {
      twoPointCross(p_[i], p_[i+1], p_[n_-1-i], p_[n_-1-(i+1)]);
    }
    break;
  case EvolveParams::NPt:
    if(!p2_) {
      p2_ = new Individual[n_];
    }
    for(i=0; i<n_; i++) {
      p2_[i].setGeneLength(gl_);
      int a = rng.integer(0, roundToInt(ep.spress*(n_-1)));
      int b = rng.integer(0, roundToInt(ep.spress*(n_-1)));
      nPointCross(p_[a], p_[b], p2_[i]);
    }
    for(i=0; i<n_; i++) {
      p_[i] = p2_[i];
    }
    break;
  }
}

void
Population::twoPointCross(const Individual &p1, const Individual &p2,
			  Individual &c1, Individual &c2) {
  int sp1, sp2;
  int i;

  sp1=rng.integer(0, gl_-1);
  sp2=rng.integer(0, gl_-1);

  if(sp1>sp2) {
    int tmp=sp1;
    sp1=sp2;
    sp2=tmp;
  }

  for(i=0; i<sp1; i++) {
    c1.ref(i)=p1(i);
    c2.ref(i)=p2(i);
  }

  for(i=sp1; i<sp2; i++) {
    c1.ref(i)=p2(i);
    c2.ref(i)=p1(i);
  }

  for(i=sp2; i<gl_; i++) {
    c1.ref(i)=p1(i);
    c2.ref(i)=p2(i);
  }
  c1.enforceConstraints();
  c2.enforceConstraints();
}

void
Population::nPointCross(const Individual &p1, const Individual &p2,
			Individual &c) {
  for(int i=0; i<gl_; i++) {
    c.ref(i) = rng.boolean() ? p1(i) : p2(i);
  }
  c.enforceConstraints();
}

void
Population::setChanged() {
  for(int i=0; i<n_; i++) {
    p_[i].setChanged();
  }
}
