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

#include <stdio.h>
#include <stdlib.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "matrix.h"
#include "param_file.h"

int
main(int argc, char *argv[]) {
  double pheno;
  int i, j, k;

  if(argc != 2) {
    fprintf(stderr, "%s <param filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  Matrix KC;
  PathwayMatrix pem;
  if(!load_parameters(filename, KC, pem)) {
    fprintf(stderr, "Unable to load parameter file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  KC.print();

  int nl = KC.nrows();
  int na = KC.ncolumns();
  int ne = pem.simulation.numEnvironment();
  
  for(k=0; k<ne; k++) {
    printf("Environment %d of %d\n", k+1, ne);
    pem.setCurEnvironment(k);
    
    for(i=0; i<na; i++) {
      for(j=0; j<nl; j++) {
	pem.kc(j, 0) = KC(j, i);
      }
      pheno = -1.0;
      Matrix results;
      if(run_simulation(pem, results)) {
	pheno = calculate_fitness(pem, results);
      }
      printf("%f\t", pheno);
    }
    putchar('\n');
    
    if(na == 2) {
      for(j=0; j<nl; j++) {
	pem.kc(j, 0) = (KC(j, 0) + KC(j, 1)) / 2.0;
	printf("\n%f", pem.kc(j, 0));
      }
      pheno = -1.0;
      Matrix results;
      if(run_simulation(pem, results)) {
	pheno = calculate_fitness(pem, results);
      }
      printf("%f\t", pheno);
      putchar('\n');
    }
  }

  return 0;
}
