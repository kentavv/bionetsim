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

#include <cfloat>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "calculate.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "log.h"
#include "matrix.h"
#include "version.h"

double
fitness(int nc, const Matrix &r) {
  double f = 0.0;
  int i;
  for(i=0; i<1000*10; i++) {
    f += (r(i, 1+1+nc) - 10.0) * (r(i, 1+1+nc) - 10.0);
    f += (r(i, 3+1+nc) -  0.0) * (r(i, 3+1+nc) -  0.0);
  }
  for(; i<2000*10; i++) {
    f += (r(i, 1+1+nc) -  0.0) * (r(i, 1+1+nc) -  0.0);
    f += (r(i, 3+1+nc) - 10.0) * (r(i, 3+1+nc) - 10.0);
  }
  for(; i<3000*10; i++) {
    f += (r(i, 1+1+nc) - 10.0) * (r(i, 1+1+nc) - 10.0);
    f += (r(i, 3+1+nc) -  0.0) * (r(i, 3+1+nc) -  0.0);
  }
  for(; i<4000*10; i++) {
    f += (r(i, 1+1+nc) -  0.0) * (r(i, 1+1+nc) -  0.0);
    f += (r(i, 3+1+nc) - 10.0) * (r(i, 3+1+nc) - 10.0);
  }
  f /= 4000*10;
  return f;
}

int
toggle(int nc, const Matrix &r) {
  return ((((r(1000*10-1, 1+1+nc) - r(2000*10-1, 1+1+nc) > 1.0) &&
	    (r(2000*10-1, 1+1+nc) - r(3000*10-1, 1+1+nc) < 1.0) &&
	    (r(3000*10-1, 1+1+nc) - r(4000*10-1, 1+1+nc) > 1.0)) ? 1 : 0) +
	  (((r(1000*10-1, 3+1+nc) - r(2000*10-1, 3+1+nc) < 1.0) &&
	    (r(2000*10-1, 3+1+nc) - r(3000*10-1, 3+1+nc) > 1.0) &&
	    (r(3000*10-1, 3+1+nc) - r(4000*10-1, 3+1+nc) < 1.0)) ? 1 : 0));
}

int
main(int argc, char *argv[]) {
  if(argc != 5) {
    fprintf(stderr, "%s <directory> <# dirs> <# files> <# reps>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Disable debug and warning output
  setLogFP(NULL, NULL); 

  PathwayMatrix pm;
  pm.items.setAutoDelete(true);
  Fitness ff;
  
  Matrix r;
  double f;

  const char *dir = argv[1];
  int nd = atoi(argv[2]);
  int nf = atoi(argv[3]);
  int nr = atoi(argv[4]);
  
  for(int i=0; i<nd; i++) {
    for(int j=0; j<nf; j++) {
      for(int k=0; k<nr; k++) {
	char filename[1024];
	sprintf(filename, "%s/%03d/gen%05d_%02d_orig.net", dir, i, j, k);
	if(!pathwayLoad(filename, pm, ff)) {
	  fprintf(stderr, "Unable to load %s\n", filename);
	  abort();
	}
#if 0
	if(!run_simulation(pm, r)) abort();
	f = fitness(pm.cvs.count(), r);
	int t =  toggle(pm.cvs.count(), r);

	printf("%d %f %s\n", t, f, filename);
#endif
      }
    }
  }
    
  return 0;
}
