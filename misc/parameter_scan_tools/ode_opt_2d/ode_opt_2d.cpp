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
  if(argc != 4) {
    fprintf(stderr, "%s <parameter filename> <param 1> <param 2>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  //setLogFP(NULL, NULL);

  const char *param_filename = argv[1];
  ScanParams sp;
  if(!sp.load(param_filename)) {
    fprintf(stderr, "Unable to load parameter file %s\n", param_filename);
    exit(EXIT_FAILURE);
  }

  sp.print();

  int magic_id = 0xfe76;
  int version = 3;
  double start[2] = {0, 0};
  double end[2] = {100, 100};
  int nsteps[2] = {200, 200};
  int param[2] = {atoi(argv[2]), atoi(argv[3])};
  
  int i, ndim = sp.pem.kc.nrows();

  char raw_filename[128];
  sprintf(raw_filename, "%02d_%02d.raw", param[0], param[1]);
  FILE *file = fopen(raw_filename, "w");
  if(!file) {
    fprintf(stderr, "Unable to open %s\n", raw_filename);
    exit(EXIT_FAILURE);
  }

  fwrite(&magic_id, sizeof(magic_id), 1, file);
  fwrite(&version, sizeof(version), 1, file);
  //int ff = sp.pem.fitness.function();
  //fwrite(&ff, sizeof(ff), 1, file);
  fwrite(param, sizeof(param), 1, file);
  fwrite(start, sizeof(start), 1, file);
  fwrite(end, sizeof(end), 1, file);
  fwrite(nsteps, sizeof(nsteps), 1, file);
  fwrite(&ndim, sizeof(ndim), 1, file);

  for(i=0; i<ndim; i++) {
    fwrite(&sp.pem.kc(i, 0), sizeof(double), 1, file);
  }
  
  for(i=0; i<=nsteps[0]; i++) {
    sp.pem.kc(param[0], 0) = start[0] + (end[0] - start[0])/nsteps[0] * i;
    for(int j=0; j<=nsteps[1]; j++) {
      sp.pem.kc(param[1], 0) = start[1] + (end[1] - start[1])/nsteps[1] * j;
      double val = -1.0;
      sp.fitness.calculate(sp.pem, val);
      // Use a float and save a little space
      double tmp = val;
      fwrite(&tmp, sizeof(tmp), 1, file);
    }
  }
  fclose(file);
  
  return(0);
}
