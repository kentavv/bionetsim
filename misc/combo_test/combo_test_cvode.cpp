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

#include <qstring.h>

#include "calculate.h"
#include "log.h"
#include "pathway_load.h"

int
toggle(int nc, const Matrix &r) {
  return ((((r(1000*10-1, 1+1+nc) - r(2000*10-1, 1+1+nc) > 1.0) &&
            (r(2000*10-1, 1+1+nc) - r(3000*10-1, 1+1+nc) < 1.0) &&
            (r(3000*10-1, 1+1+nc) - r(4000*10-1, 1+1+nc) > 1.0)) ? 1 : 0) +
          (((r(1000*10-1, 3+1+nc) - r(2000*10-1, 3+1+nc) < 1.0) &&
            (r(2000*10-1, 3+1+nc) - r(3000*10-1, 3+1+nc) > 1.0) &&
            (r(3000*10-1, 3+1+nc) - r(4000*10-1, 3+1+nc) < 1.0)) ? 2 : 0));
}

void
fitnessF(int nc, Matrix &r) {
  double f_;
  int t_;

    f_ = 0.0;
    int i;
    for(i=0; i<1000*10; i++) {
      f_ += (r(i, 1+1+nc) - 10.0) * (r(i, 1+1+nc) - 10.0);
      f_ += (r(i, 3+1+nc) -  0.0) * (r(i, 3+1+nc) -  0.0);
    }
    for(; i<2000*10; i++) {
      f_ += (r(i, 1+1+nc) -  0.0) * (r(i, 1+1+nc) -  0.0);
      f_ += (r(i, 3+1+nc) - 10.0) * (r(i, 3+1+nc) - 10.0);
    }
    for(; i<3000*10; i++) {
      f_ += (r(i, 1+1+nc) - 10.0) * (r(i, 1+1+nc) - 10.0);
      f_ += (r(i, 3+1+nc) -  0.0) * (r(i, 3+1+nc) -  0.0);
    }
    for(; i<4000*10; i++) {
      f_ += (r(i, 1+1+nc) -  0.0) * (r(i, 1+1+nc) -  0.0);
      f_ += (r(i, 3+1+nc) - 10.0) * (r(i, 3+1+nc) - 10.0);
    }
    f_ /= 4000*10;

    t_ = ::toggle(2, r);

    printf("%f %d\n", f_, t_);
}

int
main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(stderr, "%s: <net filename> <params filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];
  const char *params_filename = argv[2];

  setLogFP(stdout, NULL, NULL, NULL);

  QList<PathwayCanvasItem> items;
  
  PathwayMatrix pem;
  Fitness fitness;
  if(!pathwayLoad(filename, pem, fitness)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  int ne = pem.simulation.numEnvironment();
  QStringList rxns = pem.rxn_names;
  QStringList names = pem.plottedMoleculeNames();
  Matrix results;

  for(int i=0; i<rxns.count(); i++) {
    printf("%d %f %s\n", i, pem.kc(i, 0), (const char*)rxns[i]);
  }

  {
    Result result;
    if(run_simulation(pem, result)) {
      int nc = pem.cvs.size();
      printf("Base ");
      fitnessF(nc, result.m);
    } else {
      abort();
    }
  }

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
      pem.kc(j, 0) = atof(p);
      p = strtok(NULL, " ");
      j++;
    }
    //printf("%d\n", j);
    if(j != 9) {
      fprintf(stderr, "Insufficient params founnd\n");
      exit(EXIT_FAILURE);
    }
    Result result;
    if(run_simulation(pem, result)) {
      int nc = pem.cvs.size();
      printf("%d ", i);
      fitnessF(nc, result.m);
#if 1
      {
	char fn[64];
	sprintf(fn, "out%05d.txt", i);
	FILE *file2 = fopen(fn, "w");
	result.m.print(file2);
	fclose(file2);
      }
#endif
    } else {
      abort();
    }
    i++;
  }

  fclose(file);

  return 0;
}
