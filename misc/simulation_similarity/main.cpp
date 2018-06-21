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
  if(argc < 3) {
    fprintf(stderr, "%s <network file1> <network file2> ...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Disable debug and warning output
  setLogFP(NULL, NULL); 

  PathwayMatrix pm1, pm2;
  Fitness fitness1, fitness2;
  
  int nf = argc-1;
  IMatrix T(nf, 1);
  Matrix D(nf, nf);
  Matrix r1, r2;
  double fit1, fit2;
  
  for(int f1=0; f1<nf; f1++) {
    //printf("%d:%s\n", f1, argv[f1+1]);
    if(!pathwayLoad(argv[f1+1], pm1, fitness1)) {
      abort();
    }

    if(!run_simulation(pm1, r1)) abort();
    fit1 = fitness(pm1.cvs.count(), r1);
    T(f1, 0) =  toggle(pm1.cvs.count(), r1);

    printf("%d %f %s<p>\n", T(f1, 0), fit1, argv[f1+1]);
    printf("<img src=\"%03d.png\"><p>\n", f1);

    {
      FILE *file;

      file = fopen("data.txt", "w");
      if(!file) abort();

      r1.print(file);

      fclose(file);
      
      file = fopen("graph.gp", "w");
      if(!file) abort();

      fprintf(file,
	      "set terminal png\n"
	      "set output \"%03d.png\"\n"
	      "plot \"data.txt\" using 1:5 title \"P000\" with lines, "
	      "\"data.txt\" using 1:7 title \"P001\" with lines\n", f1);

      fclose(file);

      system("gnuplot graph.gp");
    }
    
#if 0
    for(int f2=f1+1; f2<nf; f2++) {
      if(!pathwayLoad(argv[f2+1], pm2, fitness2)) {
	abort();
      }

      if(!run_simulation(pm2, r2)) abort();
      fit2 = fitness(pm2.cvs.count(), r2);

      int nc = pm2.cvs.count();
      double d = 0.0;
      d += sqr(r1(1000*10-1, 1+1+nc) - r2(1000*10-1, 1+1+nc));
      d += sqr(r1(2000*10-1, 1+1+nc) - r2(2000*10-1, 1+1+nc));
      d += sqr(r1(3000*10-1, 1+1+nc) - r2(3000*10-1, 1+1+nc));
      d += sqr(r1(4000*10-1, 1+1+nc) - r2(4000*10-1, 1+1+nc));

      d += sqr(r1(1000*10-1, 3+1+nc) - r2(1000*10-1, 3+1+nc));
      d += sqr(r1(2000*10-1, 3+1+nc) - r2(2000*10-1, 3+1+nc));
      d += sqr(r1(3000*10-1, 3+1+nc) - r2(3000*10-1, 3+1+nc));
      d += sqr(r1(4000*10-1, 3+1+nc) - r2(4000*10-1, 3+1+nc));
            
      D(f1, f2) = D(f2, f1) = sqrt(d);
    }
#endif
  }

#if 0
  T.print();
  D.print();

  int smallest[5][2];
  int largest[5][2];
  for(int i=0; i<5; i++) {
    smallest[i][0] = 0;
    smallest[i][1] = 1;
    largest[i][0] = 0;
    largest[i][1] = 1;
  }
  
  for(int i=0; i<nf; i++) {
    for(int j=i+1; j<nf; j++) {
      for(int k=0; k<5; k++) {
	if(D(i, j) < D(smallest[k][0], smallest[k][1])) {
	  for(int m=5-1; m>k; m--) {
	    smallest[m][0] = smallest[m-1][0];
	    smallest[m][1] = smallest[m-1][1];
	  }
	  smallest[k][0] = i;
	  smallest[k][1] = j;
	  break;
	}
      }
      for(int k=0; k<5; k++) {
	if(D(i, j) > D(largest[k][0], largest[k][1])) {
	  for(int m=5-1; m>k; m--) {
	    largest[m][0] = largest[m-1][0];
	    largest[m][1] = largest[m-1][1];
	  }
	  largest[k][0] = i;
	  largest[k][1] = j;
	  break;
	}
      }
    }
  }

  puts("Best fits");
  for(int i=0; i<5; i++) {
    printf("%d %d %f\n", smallest[i][0], smallest[i][1], D(smallest[i][0], smallest[i][1]));
  }
  for(int i=0; i<5; i++) {
    printf("%s %s %f\n", argv[smallest[i][0]+1], argv[smallest[i][1]+1], D(smallest[i][0], smallest[i][1]));
  }

  puts("Worst fits");
  for(int i=0; i<5; i++) {
    printf("%d %d %f\n", largest[i][0], largest[i][1], D(largest[i][0], largest[i][1]));
  }
  for(int i=0; i<5; i++) {
    printf("%s %s %f\n", argv[largest[i][0]+1], argv[largest[i][1]+1], D(largest[i][0], largest[i][1]));
  }
#endif
  
  return 0;
}
