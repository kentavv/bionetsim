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
#include "log.h"
#include "matrix.h"
#include "scan_params.h"

int
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s <parameter filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  setLogFP(stdout, NULL, NULL, NULL);  

  const char *filename = argv[1];
  ScanParams sp;
  if(!sp.load(filename)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  sp.print();

  Matrix KC = sp.pem.kc;
  int ndim = KC.nrows();
  int i, j;
  
  for(j=0; j<ndim; j++) {
    printf("\t%s", (const char*)sp.pem.rxn_names[j]);
  }
  putchar('\n');
  
  for(i=0; i<=sp.steps; i++) {
    double x = i/(double)sp.steps * (sp.max - sp.min) + sp.min;
    printf("%f", x);
    for(j=0; j<ndim; j++) {
      sp.pem.kc(j, 0) = x;
      double val = -1.0;
      sp.fitness.calculate(sp.pem, val);
      printf("\t%f", val);
      sp.pem.kc(j, 0) = KC(j, 0);
    }
    putchar('\n');
  }
  
  return 0;
}
