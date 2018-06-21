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

#include "calculate.h"
#include "evolve_params.h"
#include "fitness_functions.h"
#include "individual.h"

#include "gsl/gsl_multimin.h"

extern EvolveParams ep;

static double
my_f(const gsl_vector *v, void * /*params*/) {
  Individual in(v->data, v->size);
  return in.fitness();
}

bool
do_simplex(double *v, size_t n, double &score) {
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *ss, *x;
  gsl_multimin_function minex_func;
  
  size_t iter = 0;
  int rval = GSL_CONTINUE;
  int status = GSL_SUCCESS;
  double ssval;
  
  /* Initial vertex size vector */
  
  ss = gsl_vector_alloc(n);
  
  if(ss == NULL) {
    //GSL_ERROR_VAL("failed to allocate space for ss", GSL_ENOMEM, 0);
  }

  /* Set all step sizes to 1 */
  gsl_vector_set_all(ss, 100.0);
  
  /* Starting point */
  x = gsl_vector_alloc(n);
  
  if(x == NULL) {
    gsl_vector_free(ss);
    //GSL_ERROR_VAL("failed to allocate space for x", GSL_ENOMEM, 0);
  }
  
  for(size_t i=0; i<n; i++) {
    gsl_vector_set(x, i, v[i]);
  }
  
  /* Initialize method and iterate */
  
  minex_func.f = &my_f;
  minex_func.n = n;
  minex_func.params = NULL;
  
  s = gsl_multimin_fminimizer_alloc(T, n);
  gsl_multimin_fminimizer_set(s, &minex_func, x, ss);
  
  while(rval == GSL_CONTINUE) {
    iter++;
    status = gsl_multimin_fminimizer_iterate(s);
    
    if(status) 
      break;
    
    //rval = gsl_multimin_test_size(gsl_multimin_fminimizer_size(s), 1e-2);
    rval = gsl_multimin_test_size(gsl_multimin_fminimizer_size(s), 1e-4);
    ssval = gsl_multimin_fminimizer_size(s);
    
#if 0
    printf("%5d ", iter);
    for(int i = 0; i < n; i++) {
      printf("%10.3e ", gsl_vector_get(s->x, i));
    }
    printf("f() = %-20.8f ssize = %.3f\n", s->fval, ssval);
#endif
    
    score = s->fval;

    if((ep.time_limit_type == EvolveParams::Timer     && ep.timer.elapsed() > ep.time_limit*1000) ||
       (ep.time_limit_type == EvolveParams::Iteration && iter > ep.time_limit)) {
      printf("%d iterations completed\n", iter);
      break;
    }
  }
  
  for(size_t i=0; i<n; i++) {
    v[i] = gsl_vector_get(s->x, i);
  }
  
  gsl_vector_free(x);
  gsl_vector_free(ss);
  gsl_multimin_fminimizer_free(s);

  printf("%d\n", rval == GSL_SUCCESS);
  return rval == GSL_SUCCESS;
}
