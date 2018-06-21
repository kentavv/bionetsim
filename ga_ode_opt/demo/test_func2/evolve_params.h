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

#ifndef _EVOLVE_PARAMS_H_
#define _EVOLVE_PARAMS_H_

#include <stdio.h>

#include <qstring.h>

#include "matrix.h"
#include "pathway_matrix.h"

class EvolveParams {
public:
  EvolveParams();

  bool load(const QString &filename);
  void print(FILE *fp = stdout);
  
  PathwayMatrix pem;

  QString network_filename;
  
  int pop_size;
  int ngen;
  double spress;

  bool maximize;
  bool elitism;
  bool replace_failures;
  int simplex_size;
  
  enum { None, NPt, TwoPt } crossover;
  double shift_mutation_range;
  int save_best_interval;
  QString save_best_prefix;
  int print_population_interval;
  int print_population_stats_interval;
  int population_identity_check_interval;
  double population_identity_limit;
  unsigned long random_seed;
  bool integer_parameters;
  Matrix param_ranges;

protected:
  EvolveParams(const EvolveParams &);
  EvolveParams &operator = (const EvolveParams &) const;
};

#endif
