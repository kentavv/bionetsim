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

#include "gsl/gsl_siman.h"
#include <gsl/gsl_vector.h>

extern EvolveParams ep;

/* set up parameters for this simulated annealing run */

/* how many points do we try before stepping */
#define N_TRIES 200
/* how many iterations for each T? */
#define ITERS_FIXED_T 100

/* max step size in random walk */
#define STEP_SIZE 10

/* Boltzmann constant */
#define K 1.0                   

/* initial temperature */
#define T_INITIAL 0.2

/* damping factor for temperature */
#define MU_T 1.005              
#define T_MIN 2.0e-6

static int gl = 0;

/* now some functions to test in one dimension */
double E1(void *xp)
{
  const double *v = (double*)xp;
  Individual in(v, gl);
  return in.fitness();
}

double M1(void *xp, void *yp)
{
  const double *x = (double*)xp;
  const double *y = (double*)yp;

  double sum = 0.0;
  for(int i=0; i<gl; i++) {
    sum += (x[i] - y[i]) * (x[i] - y[i]);
  }
  return sqrt(sum);
}

void S1(const gsl_rng * r, void *xp, double step_size)
{
  double *x = (double*)xp;

  for(int i=0; i<gl; i++) {
    double u = gsl_rng_uniform(r);
    x[i] += u * 2 * step_size - step_size;
  }
}

void P1(void *xp)
{
  const double *x = (double*)xp;
  putchar('{');
  for(int i=0; i<gl; i++) {
    if(i > 0) putchar(',');
    printf ("%12g", x[i]);
  }
  putchar('}');
}

bool
do_sa(double *v, size_t n, double &score) {
  double *x;
  
  /* Starting point */

  gl = n;
  x = (double*)malloc(sizeof(double)*n);
  
  if(x == NULL) {
    //GSL_ERROR_VAL("failed to allocate space for x", GSL_ENOMEM, 0);
  }
  
  for(size_t i=0; i<n; i++) {
    x[i] = v[i];
  }
  
  /* Initialize method and iterate */
  
  const gsl_rng_type * T;
  gsl_rng * r;

  gsl_rng_env_setup();

  T = gsl_rng_default;
  r = gsl_rng_alloc(T);

  gsl_siman_params_t params 
    = {N_TRIES, ITERS_FIXED_T, STEP_SIZE,
       K, T_INITIAL, MU_T, T_MIN};
  
  gsl_siman_solve(r, x, E1, S1, M1, P1,
                  NULL, NULL, NULL, 
                  sizeof(double)*n, params);
  
  for(size_t i=0; i<n; i++) {
    v[i] = x[i];
  }

  score = E1(x);
  
  free(x);

  return true;
}
