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

#ifndef _INDIVIDUAL_H_
#define _INDIVIDUAL_H_

#include "matrix.h"

class Individual {
public:
  Individual(int gene_length = 0);
  Individual(const double *, int);
  Individual(const Individual &);
  ~Individual();
  Individual &operator = (const Individual &);

  int geneLength() const;
  void setGeneLength(int);

  double operator()(int) const;
  double &ref(int);

  void load(const double *, int);
  
  void random();
  double fitness() const;

  double localAreaOptimization();
  double simplex();
  double simulatedAnnealing();

  bool operator == (const Individual &) const;
  bool operator != (const Individual &) const;
  bool operator < (const Individual &) const;
  bool operator > (const Individual &) const;
  
  void mutate();
  void shiftMutate();
  void helpfulMutate();
  void helpfulShiftMutate();

  void print() const;

  static void extractKC(const double *v, int n, Matrix &kc);

  bool save(const char *filename) const;
  bool saveResults(const char *filename) const;

  bool checkConstraints() const;
  void enforceConstraints();

  bool send(int fd) const;
  bool receive(int fd);
  
  void setChanged();

 protected:
  double *g_;
  int gl_;
  double f_;
  Matrix fl_;
  bool current_;
  bool local_opt_converged_;

  void allele_range_check(int, double &);
  double my_random(int);  
};

#endif
