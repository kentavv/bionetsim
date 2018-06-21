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

#ifndef _POPULATION_H_
#define _POPULATION_H_

#include "individual.h"

class Population {
public:
  Population(int n=0, int gl=0);
  Population(const Population &);
  ~Population();

  void resize(int n, int gl);
  void random();

  void sort();

  const Individual &operator()(int) const;
  Individual &operator()(int);

  void print() const;
  void printStats() const;
  void printAverage() const;
  
  void prune();

  void crossover();

protected:
  Individual *p_, *p2_;
  int n_;
  int gl_;
  
  static int individualCompare(const void *, const void *);
  void twoPointCross(const Individual &p1, const Individual &p2,
		     Individual &c1, Individual &c2);
  void nPointCross(const Individual &p1, const Individual &p2,
		   Individual &c);
};

#endif
