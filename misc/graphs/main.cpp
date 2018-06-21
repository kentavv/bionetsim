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

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "log.h"
#include "matrix.h"
#include "version.h"

#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_sort_vector.h>

gsl_vector *
laplacianEigs(const IMatrix a) {
  int n = a.nrows();
  Matrix lap(n, n);

  for(int i=0; i<n; i++) {
    int k=0;
    for(int j=0; j<n; j++) {
      if(a(i, j) && i!=j) {
	lap(i, j) = -1;
	k++;
      }
    }
    lap(i, i) = k;
  }

#if 0
  for(int i=0; i<n; i++) {
    for(int j=0; j<n; j++) {
      if(lap(i, j) == -1) {
	lap(i, j) /= sqrt(lap(i, i) * lap(j, j));
      }
    }
  }
  for(int i=0; i<n; i++) {
    if(lap(i, i) > 1) lap(i, i) = 1;
  }
#endif
    
  //a.print();
  //printf("=================\n");
  //lap.print();

  gsl_matrix_view m = gsl_matrix_view_array((double*)lap.raw(), n, n);

  gsl_vector *eval = gsl_vector_alloc(n);

  gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(n);
  
  gsl_eigen_symm(&m.matrix, eval, w);

  gsl_eigen_symm_free(w);

  gsl_sort_vector(eval);
  gsl_vector_reverse(eval);

  for(int i=0; i<n; i++) {
    if(fabs(gsl_vector_get(eval, i)) < 1e-12) {
      gsl_vector_set(eval, i, 0.0);
    }
  }
  
  return eval;
}

bool
next_graph(FILE *file, IMatrix &g) {
  bool graph_read = false;
  
  char line[1024];
  while(fgets(line, 1024, file)) {
    if(line[0] == 'G') {
      graph_read = true;
      int n;
      if(sscanf(line, "Graph %*d, order %d.", &n) != 1) break;
      g.resize(n, n);
      g.fill(0);
      for(int i=0; i<n; i++) {
	if(fgets(line, 1024, file) == NULL) return false;
	for(int j=0; j<n; j++) {
	  if(line[j] == '1') {
	    g(i, j) = 1;
	  }
	}
      }
      break;
    }
  }
  
  return graph_read && !g.empty();
}

int
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s <graph file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *file1 = fopen(argv[1], "r");
  FILE *file2 = fopen(argv[1], "r");
  if(!file1 || !file2) {
    fprintf(stderr, "Unable to open %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  int ng = 0;
  
  char line[1024];
  while(fgets(line, 1024, file1)) {
    if(line[0] == 'G') ng++;
  }

  Matrix D(ng, ng);
  IMatrix g1, g2;

  rewind(file1);

  for(int f1=0; next_graph(file1, g1); f1++) {
    printf("%d/%d\n", f1, ng);

    {
      char str[1024];
      sprintf(str, "graph%03d.dot", f1);
      FILE *dot = fopen(str, "w");
      if(!dot) abort();
      fprintf(dot, "Graph G%03d {\n", f1);
      for(int i=0; i<g1.nrows(); i++) {
	fprintf(dot, "  %c [shape=circle];\n", 'A'+i);
      }
      for(int i=0; i<g1.nrows(); i++) {
	for(int j=i+1; j<g1.nrows(); j++) {
	  if(g1(i, j)) {
	    fprintf(dot, "  %c -- %c;\n", 'A'+i, 'A'+j);
	  }
	}
      }
      fprintf(dot, "}\n");
      fclose(dot);
    }
    
    gsl_vector *e1 = laplacianEigs(g1);

#if 1
    {
      int i;
      int n = g1.nrows();
      
      printf ("eigenvalues = ");
      for (i = 0; i < n; i++)
	{
	  printf("%g\t", gsl_vector_get (e1, i));
	}
      putchar('\n');
    }
#endif

    fseek(file2, ftell(file1), SEEK_SET);

    for(int f2=f1+1; next_graph(file2, g2); f2++) {
      gsl_vector *e2 = laplacianEigs(g2);
      
      double d1 = 0.0;
      double d2 = 0.0;
      double d3 = 0.0;
      
      int m = min(e1->size, e2->size);
      //printf("m:%d\n", m);
      for(int i=0; i<m; i++) {
	double a = gsl_vector_get(e1, i);
	double b = gsl_vector_get(e2, i);
	d1 += (a-b)*(a-b);
	d2 += (a-b)*(a-b);
	if(a > 0 && b > 0) d3 += (a-b)*(a-b);
      }

      if(e1->size > e2->size) {
	for(int i=m; i<e1->size; i++) {
	  double a = gsl_vector_get(e1, i);
	  d2 += (a)*(a);
	}
      } else if(e2->size > e1->size) {
	for(int i=m; i<e2->size; i++) {
	  double a = gsl_vector_get(e2, i);
	  d2 += (a)*(a);
	}
      }

      gsl_vector_free(e2);
      
      //D(f1, f2) = D(f2, f1) = sqrt(d1);
      D(f1, f2) = D(f2, f1) = sqrt(d2);
      //D(f1, f2) = D(f2, f1) = sqrt(d3);
    }
    gsl_vector_free(e1);
  }

  D.print();

  int smallest[5][2];
  int largest[5][2];
  for(int i=0; i<5; i++) {
    smallest[i][0] = 0;
    smallest[i][1] = 1;
    largest[i][0] = 0;
    largest[i][1] = 1;
  }
  
  for(int i=0; i<ng; i++) {
    for(int j=i+1; j<ng; j++) {
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

  puts("Worst fits");
  for(int i=0; i<5; i++) {
    printf("%d %d %f\n", largest[i][0], largest[i][1], D(largest[i][0], largest[i][1]));
  }

  for(int i=0; i<ng; i++) {
    printf("Graph %d ==================================\n", i+1);
    for(int j=0; j<ng; j++) {
      printf("%.04f\t\t", D(i, j));
      if(j > 0 && ((j+1) % 6) == 0) putchar('\n');
    }
    putchar('\n');
  }

  return 0;
}
