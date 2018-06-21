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
#include <unistd.h>

#include <qstring.h>

#include "log.h"
#include "pathway_load.h"

void
export_mathematica(const char *str, const double *a) {
  printf("Print[\"%s: ", str);
  for(int i=0; i<9; i++) {
    printf("%f ", a[i]);
  }
  printf("\"]\n");

  printf("TableForm[Timing[NSolve[{k1 == %f, k2 == %f, k3 == %f, k4 == %f, k5 == %f, k6 == %f, k7 == %f, k70 == %f, k8 == %f, "
	 "0 == k1*1 -k2*A -k5*A*B -k7*b*A + k70*(1-b), 0 == -k7*b*A + k70*(1-b), 0 == k3*b -k4*B -k5*A*B + k8*(1-b), 0 == k5*A*B -k6*d, 0 == k7*b*A - k70*(1-b)}, {A, b, B, d}]]]\n", 
	 a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
}

int
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s: <params filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *params_filename = argv[1];

  setLogFP(stdout, NULL, NULL, NULL);

  double a[9] = {.2, .0085, .37, .034, .72, .53, .19, .42, .027};
  export_mathematica("Base: ", a);
  
  FILE *file = fopen(params_filename, "r");
  if(!file) {
    fprintf(stderr, "Unable to open %s\n", params_filename);
    exit(EXIT_FAILURE);
  }
  char line[1024];
  int i=0;
  while(fgets(line, 1024, file)) {
    int j = 0;
    char *p = strtok(line, " ");
    while(p) {
      a[j] = atof(p);
      p = strtok(NULL, " ");
      j++;
    }
    if(j != 9) {
      fprintf(stderr, "Insufficient params founnd\n");
      exit(EXIT_FAILURE);
    }
    char str[64];
    sprintf(str, "%d", i);
    export_mathematica(str, a);
    i++;
  }

  fclose(file);

  return 0;
}
