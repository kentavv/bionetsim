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
#include "fitness.h"
#include "matrix.h"
#include "pathway_matrix.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "pathway_save.h"

#include "gsl/gsl_multimin.h"

static PathwayMatrix pem;
static Fitness fitness;
static size_t np = 0;

//#define MAXIMIZE

static double
my_f (const gsl_vector *v, void *params) {
  int i;
  
  for(i=0; i<np; i++) {
    pem.kc(i, 0) = gsl_vector_get(v, i);
  }

#ifdef MAXIMIZE
  double val = -DBL_MAX;
#else
  double val = DBL_MAX;
#endif

  if(!fitness.calculate(pem, val)) {
    fprintf(stderr, "Failed to calculate fitness\n");
  }

#ifdef MAXIMIZE
  return -val;
#else
  return val;
#endif
}

bool
save_best(/*int gen*/const char *filename) {
  //int env = 0;
  pem.applyKCToItems();
  //pem.setCurEnvironment(env);
  //pem.simulation.setCurEnvironment(env);
  
  //  char filename[1024];
  //sprintf(filename, (const char*)(save_best_prefix + "_%04d.net"), gen);
  if(!pathwaySave(filename, "Simplex Test"/*appname*/, "beta"/*appversion*/,
                  pem.editor, fitness, pem.simulation, pem.items)) {
    return false;
  }

  return true;
}

int
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s <network filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  QList<PathwayCanvasItem> items;
  if(!pathwayLoad(filename, pem, fitness)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  Matrix KC;







  {
    np = pem.kc.nrows();

    const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer *s = NULL;
    gsl_vector *ss, *x;
    gsl_multimin_function minex_func;

    size_t iter = 0, i;
    int rval = GSL_CONTINUE;
    int status = GSL_SUCCESS;
    double ssval;

    /* Initial vertex size vector */

    ss = gsl_vector_alloc (np);

    if (ss == NULL) {
      GSL_ERROR_VAL ("failed to allocate space for ss", GSL_ENOMEM, 0);
    }
  
    gsl_vector_set_all(ss, 1.0);

    /* Starting point */

    x = gsl_vector_alloc (np);

    if (x == NULL) {
      gsl_vector_free(ss);
      GSL_ERROR_VAL ("failed to allocate space for x", GSL_ENOMEM, 0);
    }
  
    for(size_t i=0; i<np; i++) {
      gsl_vector_set (x, i, pem.kc(i, 0));
    }

    /* Initialize method and iterate */

    minex_func.f = &my_f;
    minex_func.n = np;
    minex_func.params = NULL;

    s = gsl_multimin_fminimizer_alloc (T, np);
    gsl_multimin_fminimizer_set (s, &minex_func, x, ss);

    while (rval == GSL_CONTINUE) {
      iter++;
      status = gsl_multimin_fminimizer_iterate(s);
      
      if (status) 
        break;

      //rval = gsl_multimin_test_size (gsl_multimin_fminimizer_size (s), 1e-2);
      rval = gsl_multimin_test_size (gsl_multimin_fminimizer_size (s), 1e-4);
      ssval = gsl_multimin_fminimizer_size (s);
      
      if (rval == GSL_SUCCESS)
        printf ("converged to minimum at\n");
      
      printf("%5d ", iter);
      for (i = 0; i < np; i++)
        {
          printf ("%10.3e ", gsl_vector_get (s->x, i));
        }
      printf("f() = %-20.8f ssize = %.3f\n", s->fval, ssval);
    }
    
    gsl_vector_free(x);
    gsl_vector_free(ss);
    gsl_multimin_fminimizer_free (s);
  }

  //if(!pathwaySave("best.net", pem.editor, pem.simulation, items)) {
  if(!save_best("best.net")) {
    fprintf(stderr, "Unable to save result file\n");
  }

  return 0;
}
