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
  if(argc != 4) {
    fprintf(stderr, "%s <param filename> <CV 1> <CV 2>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *param_filename = argv[1];
  int magic_id = 0xfe76;
  int version = 2;
  int start[2] = {0, 0};
  int end[2] = {1000, 1000};
  int step[2] = {1, 1};
  int param[2] = {atoi(argv[2]), atoi(argv[3])};

  Matrix KC;
  PathwayMatrix pem;
  if(!load_parameters(param_filename, KC, pem)) {
    fprintf(stderr, "Unable to load parameter file %s\n", param_filename);
    exit(EXIT_FAILURE);
  }

  int i, ndim = KC.nrows();

#if 0
  for(i=0; i<ndim; i++) {
    pem.kc(i, 0) = KC(i, 0);
  }
#endif

  char raw_filename[128];
  sprintf(raw_filename, "%02d_%02d.raw", param[0], param[1]);
  FILE *file = fopen(raw_filename, "w");
  if(!file) {
    fprintf(stderr, "Unable to open %s\n", raw_filename);
    exit(EXIT_FAILURE);
  }

  fwrite(&magic_id, sizeof(magic_id), 1, file);
  fwrite(&version, sizeof(version), 1, file);
  int ff = pem.simulation.fitnessFunction();
  fwrite(&ff, sizeof(ff), 1, file);
  fwrite(param, sizeof(param), 1, file);
  fwrite(start, sizeof(start), 1, file);
  fwrite(end, sizeof(end), 1, file);
  fwrite(step, sizeof(step), 1, file);
  fwrite(&ndim, sizeof(ndim), 1, file);

  for(i=0; i<ndim; i++) {
    fwrite(&pem.kc(i, 0), sizeof(double), 1, file);
  }
  
  for(i=start[0]; i<=end[0]; i+=step[0]) {
    ((SquareWaveControlVariableItem*)pem.cvs[param[0]])->setValues(i/10.0, 0.0);
    for(int j=start[1]; j<=end[1]; j+=step[1]) {
      ((SquareWaveControlVariableItem*)pem.cvs[param[1]])->setValues(j/10.0, 0.0);
      // Use a float and save a little space
      float val = -1.0;
      Matrix results;
      if(run_simulation(pem, results)) {
	val = calculate_fitness(pem, results);
      }
      printf("%f %f %f\n", i/10.0, j/10.0, val);
      fwrite(&val, sizeof(val), 1, file);
    }
  }
  fclose(file);
  
  return(0);
}
