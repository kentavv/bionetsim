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
laplacianEigs(QList<PathwayCanvasItem> &items) {
  PathwayCanvasItem *i;
  int n = 0;
  for(i=items.last(); i; i=items.prev()) {
    if(!(isEdge(i) || isNode(i))) {
      i->setId(n++);
      //printf("%d %s\n", i->id(), (const char*)((MolRxnBaseItem*)i)->label());
    }
  }
  Matrix lap(n, n);
  for(i=items.last(); i; i=items.prev()) {
    if(!(isEdge(i) || isNode(i))) {
      lap(i->id(), i->id()) = ((MolRxnBaseItem*)i)->edges().count();
    }
  }
  for(i=items.last(); i; i=items.prev()) {
    if(isCompleteEdge(i)) {
      if(isReactionEdge(i)) {
	ReactionEdgeItem *e = (ReactionEdgeItem*)i;
	lap(e->molecule()->id(), e->reaction()->id()) = -1;
	lap(e->reaction()->id(), e->molecule()->id()) = -1;
      } else if(isSubnetworkEdge(i)) {
	SubnetworkEdgeItem *e = (SubnetworkEdgeItem*)i;
	lap(e->molecule()->id(), e->subnetwork()->id()) = -1;
	lap(e->subnetwork()->id(), e->molecule()->id()) = -1;
      }
    }
  }

  //  lap.print();

  gsl_matrix_view m 
    = gsl_matrix_view_array ((double *)lap.raw(), n, n);

  gsl_vector *eval = gsl_vector_alloc (n);

  gsl_eigen_symm_workspace * w = 
    gsl_eigen_symm_alloc (n);
  
  gsl_eigen_symm (&m.matrix, eval, w);

  gsl_eigen_symm_free (w);

  gsl_sort_vector(eval);
  gsl_vector_reverse(eval);

  for(int i=0; i<n; i++) {
    if(gsl_vector_get (eval, i) < 0.0) {
      gsl_vector_set(eval, i, 0.0);
    }
  }
    
#if 0
  {
    int i;

    printf ("eigenvalues = ");
    for (i = 0; i < n; i++)
      {
        printf("%g\t", gsl_vector_get (eval, i));
      }
    putchar('\n');
  }
#endif
  
  return eval;
}

int
main(int argc, char *argv[]) {
  if(argc < 3) {
    fprintf(stderr, "%s <network file1> <network file2> ...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  QList<PathwayCanvasItem> net1, net2;

  int nf = argc-1;
  Matrix D(nf, nf);
  
  for(int f1=0; f1<nf; f1++) {
    printf("%d:%s\n", f1, argv[f1+1]);
    if(!pathwayLoad(argv[f1+1], net1)) {
      abort();
    }
    
    gsl_vector *e1 = laplacianEigs(net1);
    
    for(int f2=f1+1; f2<nf; f2++) {
      if(!pathwayLoad(argv[f2+1], net2)) {
	abort();
      }

      gsl_vector *e2 = laplacianEigs(net2);
      
      double d = 0.0;
      int m = min(e1->size, e2->size);
      //printf("m:%d\n", m);
      for(int i=0; i<m; i++) {
	double a = gsl_vector_get(e1, i);
	double b = gsl_vector_get(e2, i);
	d += (a-b)*(a-b);
      }
      if(e1->size > e2->size) {
	for(int i=m; i<e1->size; i++) {
	  double a = gsl_vector_get(e1, i);
	  d += (a)*(a);
	}
      } else if(e2->size > e1->size) {
	for(int i=m; i<e2->size; i++) {
	  double a = gsl_vector_get(e2, i);
	  d += (a)*(a);
	}
      }
      
      gsl_vector_free(e2);
      
      D(f1, f2) = D(f2, f1) = sqrt(d);
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

  return 0;
}
