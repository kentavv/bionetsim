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
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s: <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  setLogFP(stdout, NULL, NULL, NULL);

  QList<PathwayCanvasItem> items;
  
  PathwayMatrix pem;
  Fitness fitness;
  if(!pathwayLoad(filename, pem, fitness)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  int ne = pem.simulation.numEnvironment();
  QStringList envs = pem.simulation.environmentDescriptions();
  QStringList names = pem.plottedMoleculeNames();
  Matrix results;
#if 0
  for(int i=0; i<ne; i++) {
    if(i > 0) putchar('\n');
    if(ne > 1) printf("Env: %d\n", i);
    printf("Time");
    for(int j=0; j<names.count(); j++) {
      printf("\t%s", (const char*)names[j]);
    }
    putchar('\n');
    pem.setCurEnvironment(i);
    if(run_simulation(pem, results)) {
      results.print();
    }
  }
#else
  printf("Time");
  for(int j=0; j<names.count(); j++) {
    printf("\t%s", (const char*)names[j]);
  }
  putchar('\n');
  double fit = -1;
  fitness.calculate(pem, fit);
  printf("Fitness: %f\n", fit);
#endif
  
  return 0;
}
