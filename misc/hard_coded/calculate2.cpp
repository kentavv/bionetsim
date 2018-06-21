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
#include <math.h>

#include "calculate.h"
#include "matrix.h"
#include "squarewave.h"

#include "sundialstypes.h"
#include "cvode.h"
#include "cvdense.h"
#include "nvector_serial.h"
#include "dense.h"

#define Ith(v,i)    NV_Ith_S(v,i)
#define IJth(A,i,j) DENSE_ELEM(A,i,j)

static void F(integertype N, realtype t, N_Vector y, N_Vector ydot, void *f_data);

static void Jac(integertype N, DenseMat J, RhsFn f, void *f_data, realtype t,
		    N_Vector y, N_Vector fy, N_Vector ewt, realtype h, realtype uround,
		    void *jac_data, long int *nfePtr, N_Vector vtemp1,
		    N_Vector vtemp2, N_Vector vtemp3);

/***************************** Main Program ******************************/

const int env = 0;

static int
calculate_results(const Matrix &kc_mat, Matrix &results) {
  const int ncvs = 2;
  const int nmol = 16;
  const int nrxn = 27;
  const double tend = 230.0;
  const double tstep = 0.1;
  const double t0 = 0.0;
  double reltol = 0.0001;
  const realtype min_step = 0.000000;
  const realtype max_step = 1.000000;

  const double kc[nrxn] = { kc_mat(0, 0),
			    kc_mat(1, 0),
			    kc_mat(2, 0),
			    kc_mat(3, 0),
			    kc_mat(4, 0),
			    kc_mat(5, 0),
			    kc_mat(6, 0),
			    kc_mat(7, 0),
			    kc_mat(8, 0),
			    kc_mat(9, 0),
			    kc_mat(10, 0),
			    kc_mat(11, 0),
			    kc_mat(12, 0),
			    kc_mat(13, 0),
			    kc_mat(14, 0),
			    kc_mat(15, 0),
			    kc_mat(16, 0),
			    kc_mat(17, 0),
			    kc_mat(18, 0),
			    kc_mat(19, 0),
			    kc_mat(20, 0),
			    kc_mat(21, 0),
			    kc_mat(22, 0),
			    kc_mat(23, 0),
			    kc_mat(24, 0),
			    kc_mat(25, 0),
			    kc_mat(26, 0) };

  M_Env machEnv;
  realtype ropt[OPT_SIZE], t;
  long int iopt[OPT_SIZE];
  N_Vector y, abstol;
  void *cvode_mem;
  int flag, i;

  const int neq = nmol - ncvs;
  const int nsteps = int(tend/tstep + 0.5);

  results.resize(nsteps+1, nmol+1);

  if(neq == 0) {
    for(i=0; i<=nsteps; i++) {
      double tout = t0+i*tstep;
      results(i, 0) = tout;
      if(env == 0) {
	results(i, 1) = squarewave(tout, 1, 0, 20, 20, 10, 10); // GalExt
	results(i, 2) = squarewave(tout, 1, 0, 50, 50, 10, 10); // GluExt
      }
    }
    return 1;
  }

  machEnv = M_EnvInit_Serial(neq);
  
  y = N_VNew(neq, machEnv);
  abstol = N_VNew(neq, machEnv); 
  
  Ith(y, 0)      = 0.0; // Gal
  Ith(y, 1)      = 0.0; // Glu-6P
  Ith(y, 2)      = 0.0; // Gal80p
  Ith(y, 3)      = 0.0; // Gal4p
  Ith(y, 4)      = 0.0; // Gal3p
  Ith(y, 5)      = 0.0; // GAL-4-80
  Ith(y, 6)      = 0.0; // GAL-4-80-3
  Ith(y, 7)      = 0.0; // Glu
  Ith(y, 8)      = 0.0; // gal4g
  Ith(y, 9)      = 1.0; // gal4gX
  Ith(y, 10)      = 0.0; // Gal1-2-5-7-10p
  Ith(y, 11)      = 1.0; // GAL
  Ith(y, 12)      = 0.0; // GAL-4
  Ith(y, 13)      = 0.0; // Gal3p*
  Ith(abstol, 0) = 1e-8;
  Ith(abstol, 1) = 1e-8;
  Ith(abstol, 2) = 1e-8;
  Ith(abstol, 3) = 1e-8;
  Ith(abstol, 4) = 1e-8;
  Ith(abstol, 5) = 1e-8;
  Ith(abstol, 6) = 1e-8;
  Ith(abstol, 7) = 1e-8;
  Ith(abstol, 8) = 1e-8;
  Ith(abstol, 9) = 1e-8;
  Ith(abstol, 10) = 1e-8;
  Ith(abstol, 11) = 1e-8;
  Ith(abstol, 12) = 1e-8;
  Ith(abstol, 13) = 1e-8;
  
  memset(ropt, 0, sizeof(ropt));
  memset(iopt, 0, sizeof(iopt));
  ropt[HMIN] = min_step;
  ropt[HMAX] = max_step;

  cvode_mem = CVodeMalloc(neq, F, t0, y, BDF, NEWTON, SV, &reltol, abstol,
                          (void*)kc, NULL, TRUE, iopt, ropt, machEnv);
  if(!cvode_mem) {
    fprintf(stderr, "CVodeMalloc failed.\n");
    return -2;
  }

  flag = CVDense(cvode_mem, Jac, NULL);
  if(flag != SUCCESS) {
    fprintf(stderr, "CVDense failed.\n");
    return -2;
  }

  results(0, 0) = t0;
  if(env == 0) {
    results(0, 1) = squarewave(t0, 1, 0, 20, 20, 10, 10);
    results(0, 2) = squarewave(t0, 1, 0, 50, 50, 10, 10);
  }
  results(0, 0+2+1) = Ith(y,0);
  results(0, 1+2+1) = Ith(y,1);
  results(0, 2+2+1) = Ith(y,2);
  results(0, 3+2+1) = Ith(y,3);
  results(0, 4+2+1) = Ith(y,4);
  results(0, 5+2+1) = Ith(y,5);
  results(0, 6+2+1) = Ith(y,6);
  results(0, 7+2+1) = Ith(y,7);
  results(0, 8+2+1) = Ith(y,8);
  results(0, 9+2+1) = Ith(y,9);
  results(0, 10+2+1) = Ith(y,10);
  results(0, 11+2+1) = Ith(y,11);
  results(0, 12+2+1) = Ith(y,12);
  results(0, 13+2+1) = Ith(y,13);
  
  for(i=1; i<=nsteps; i++) {
    //printf("%d/%d\n", i, nsteps);
    double tout = t0+i*tstep;
    flag = CVode(cvode_mem, tout, y, &t, NORMAL);
    if (flag != SUCCESS) {
      fprintf(stderr, "CVode failed, flag=%d.\n", flag);
      break;
    }

    results(i, 0) = t;
    if(env == 0) {
      results(i, 1) = squarewave(t, 1, 0, 20, 20, 10, 10);
      results(i, 2) = squarewave(t, 1, 0, 50, 50, 10, 10);
    }
    results(i, 0+2+1) = Ith(y,0);
    results(i, 1+2+1) = Ith(y,1);
    results(i, 2+2+1) = Ith(y,2);
    results(i, 3+2+1) = Ith(y,3);
    results(i, 4+2+1) = Ith(y,4);
    results(i, 5+2+1) = Ith(y,5);
    results(i, 6+2+1) = Ith(y,6);
    results(i, 7+2+1) = Ith(y,7);
    results(i, 8+2+1) = Ith(y,8);
    results(i, 9+2+1) = Ith(y,9);
    results(i, 10+2+1) = Ith(y,10);
    results(i, 11+2+1) = Ith(y,11);
    results(i, 12+2+1) = Ith(y,12);
    results(i, 13+2+1) = Ith(y,13);
  }
  for(; i<=nsteps; i++) {
    double tout = t0+i*tstep;
    results(i, 0) = tout;
    results(i, 1) = -1;
    results(i, 2) = -1;
    results(i, 3) = -1;
    results(i, 4) = -1;
    results(i, 5) = -1;
    results(i, 6) = -1;
    results(i, 7) = -1;
    results(i, 8) = -1;
    results(i, 9) = -1;
    results(i, 10) = -1;
    results(i, 11) = -1;
    results(i, 12) = -1;
    results(i, 13) = -1;
    results(i, 14) = -1;
    results(i, 15) = -1;
  }

  //results.print();
  
  N_VFree(y);
  N_VFree(abstol);   
  CVodeFree(cvode_mem);
  M_EnvFree_Serial(machEnv);
  
#if 0
  printf("\nFinal Statistics.. \n\n"
	 "nst = %-6ld nfe  = %-6ld nsetups = %-6ld nje = %ld\n"
	 "nni = %-6ld ncfn = %-6ld netf = %ld\n \n",
	 iopt[NST], iopt[NFE], iopt[NSETUPS], iopt[DENSE_NJE],
	 iopt[NNI], iopt[NCFN], iopt[NETF]);
#endif

  return (flag == SUCCESS) ? 1 : -1;
}

extern "C" bool
run_gal4_simulation(const Matrix &kc, Matrix &results) {
  int flag = calculate_results(kc, results);
  return flag == 1;
}

/***************** Functions Called by the CVODE Solver ******************/

/* f routine. Compute f(t,y). */

#define N_VIth(v,i)    NV_Ith_S(v,i)

static void
F(integertype, realtype t, N_Vector Y, N_Vector Y2, void *f_data) {
  const double *kc = (double*)f_data;
  double cv1, cv2;
  
  if(env == 0) {
    cv1 = squarewave(t, 1, 0, 20, 20, 10, 10);
    cv2 = squarewave(t, 1, 0, 50, 50, 10, 10);
  }
  
  double r0 = kc[0] * cv1;
  double r1 = kc[1] * cv1 * N_VIth(Y, 10);
  double r2 = kc[2] * N_VIth(Y, 0) * N_VIth(Y, 10);
  double r3 = kc[3] * N_VIth(Y, 1);
  double r4 = kc[4] * N_VIth(Y, 7) * N_VIth(Y, 8);
  double r5 = kc[5] * N_VIth(Y, 9);
  double r6 = kc[6] * N_VIth(Y, 8);
  double r7 = kc[7] * N_VIth(Y, 3) * N_VIth(Y, 11);
  double r8 = kc[8] * N_VIth(Y, 12);
  double r9 = kc[9] * N_VIth(Y, 2) * N_VIth(Y, 12);
  double r10 = kc[10] * N_VIth(Y, 5);
  double r11 = kc[11] * N_VIth(Y, 5) * N_VIth(Y, 13);
  double r12 = kc[12] * N_VIth(Y, 6);
  double r13 = kc[13] * N_VIth(Y, 12);
  double r14 = kc[14] * N_VIth(Y, 12);
  double r15 = kc[15] * N_VIth(Y, 6);
  double r16 = kc[16] * N_VIth(Y, 6);
  double r17 = kc[17] * N_VIth(Y, 6);
  double r18 = kc[18] * N_VIth(Y, 12);
  double r19 = kc[19] * cv2;
  double r20 = kc[20] * N_VIth(Y, 7);
  double r21 = kc[21] * N_VIth(Y, 3);
  double r22 = kc[22] * N_VIth(Y, 2);
  double r23 = kc[23] * N_VIth(Y, 4);
  double r24 = kc[24] * N_VIth(Y, 10);
  double r25 = kc[25] * N_VIth(Y, 0) * N_VIth(Y, 4);
  double r26 = kc[26] * N_VIth(Y, 13);
  N_VIth(Y2, 0) = 1.000000 * r0 + 1.000000 * r1 + -1.000000 * r2;
  N_VIth(Y2, 1) = 1.000000 * r2 + -1.000000 * r3;
  N_VIth(Y2, 2) = -1.000000 * r9 + 1.000000 * r10 + 1.000000 * r13 + 1.000000 * r15 + -1.000000 * r22;
  N_VIth(Y2, 3) = 1.000000 * r6 + -1.000000 * r7 + 1.000000 * r8 + -1.000000 * r21;
  N_VIth(Y2, 4) = 1.000000 * r14 + 1.000000 * r16 + -1.000000 * r23 + -1.000000 * r25 + 1.000000 * r26;
  N_VIth(Y2, 5) = 1.000000 * r9 + -1.000000 * r10 + -1.000000 * r11 + 1.000000 * r12;
  N_VIth(Y2, 6) = 1.000000 * r11 + -1.000000 * r12;
  N_VIth(Y2, 7) = 1.000000 * r3 + 1.000000 * r19 + -1.000000 * r20;
  N_VIth(Y2, 8) = -1.000000 * r4 + 1.000000 * r5;
  N_VIth(Y2, 9) = 1.000000 * r4 + -1.000000 * r5;
  N_VIth(Y2, 10) = 1.000000 * r17 + 1.000000 * r18 + -1.000000 * r24;
  N_VIth(Y2, 11) = -1.000000 * r7 + 1.000000 * r8;
  N_VIth(Y2, 12) = 1.000000 * r7 + -1.000000 * r8 + -1.000000 * r9 + 1.000000 * r10;
  N_VIth(Y2, 13) = 1.000000 * r25 + -1.000000 * r26;
}

/* Jacobian routine. Compute J(t,y). */

static void
Jac(integertype, DenseMat J, RhsFn, void *f_data, realtype t,
	N_Vector Y, N_Vector, N_Vector, realtype, realtype,
	void*, long int*, N_Vector, N_Vector, N_Vector) {
  const double *kc = (double*)f_data;
  double cv1, cv2;
  
  if(env == 0) {
    cv1 = squarewave(t, 1, 0, 20, 20, 10, 10);
    cv2 = squarewave(t, 1, 0, 50, 50, 10, 10);
  }
  
  double r00 = kc[0];
  double r10 = kc[1] * N_VIth(Y, 10);
  double r112 = kc[1] * cv1;
  double r22 = kc[2] * N_VIth(Y, 10);
  double r212 = kc[2] * N_VIth(Y, 0);
  double r33 = kc[3];
  double r49 = kc[4] * N_VIth(Y, 8);
  double r410 = kc[4] * N_VIth(Y, 7);
  double r511 = kc[5];
  double r610 = kc[6];
  double r75 = kc[7] * N_VIth(Y, 11);
  double r713 = kc[7] * N_VIth(Y, 3);
  double r814 = kc[8];
  double r94 = kc[9] * N_VIth(Y, 12);
  double r914 = kc[9] * N_VIth(Y, 2);
  double r107 = kc[10];
  double r117 = kc[11] * N_VIth(Y, 13);
  double r1115 = kc[11] * N_VIth(Y, 5);
  double r128 = kc[12];
  double r1314 = kc[13];
  double r1414 = kc[14];
  double r158 = kc[15];
  double r168 = kc[16];
  double r178 = kc[17];
  double r1814 = kc[18];
  double r191 = kc[19];
  double r209 = kc[20];
  double r215 = kc[21];
  double r224 = kc[22];
  double r236 = kc[23];
  double r2412 = kc[24];
  double r252 = kc[25] * N_VIth(Y, 4);
  double r256 = kc[25] * N_VIth(Y, 0);
  double r2615 = kc[26];
  //DENSE_ELEM(J, 0, -2) = 1.000000 * r00 + 1.000000 * r10;
  DENSE_ELEM(J, 0, 0) = -1.000000 * r22;
  DENSE_ELEM(J, 0, 10) = 1.000000 * r112 + -1.000000 * r212;
  DENSE_ELEM(J, 1, 0) = 1.000000 * r22;
  DENSE_ELEM(J, 1, 1) = -1.000000 * r33;
  DENSE_ELEM(J, 1, 10) = 1.000000 * r212;
  DENSE_ELEM(J, 2, 2) = -1.000000 * r94 + -1.000000 * r224;
  DENSE_ELEM(J, 2, 5) = 1.000000 * r107;
  DENSE_ELEM(J, 2, 6) = 1.000000 * r158;
  DENSE_ELEM(J, 2, 12) = -1.000000 * r914 + 1.000000 * r1314;
  DENSE_ELEM(J, 3, 3) = -1.000000 * r75 + -1.000000 * r215;
  DENSE_ELEM(J, 3, 8) = 1.000000 * r610;
  DENSE_ELEM(J, 3, 11) = -1.000000 * r713;
  DENSE_ELEM(J, 3, 12) = 1.000000 * r814;
  DENSE_ELEM(J, 4, 0) = -1.000000 * r252;
  DENSE_ELEM(J, 4, 4) = -1.000000 * r236 + -1.000000 * r256;
  DENSE_ELEM(J, 4, 6) = 1.000000 * r168;
  DENSE_ELEM(J, 4, 12) = 1.000000 * r1414;
  DENSE_ELEM(J, 4, 13) = 1.000000 * r2615;
  DENSE_ELEM(J, 5, 2) = 1.000000 * r94;
  DENSE_ELEM(J, 5, 5) = -1.000000 * r107 + -1.000000 * r117;
  DENSE_ELEM(J, 5, 6) = 1.000000 * r128;
  DENSE_ELEM(J, 5, 12) = 1.000000 * r914;
  DENSE_ELEM(J, 5, 13) = -1.000000 * r1115;
  DENSE_ELEM(J, 6, 5) = 1.000000 * r117;
  DENSE_ELEM(J, 6, 6) = -1.000000 * r128;
  DENSE_ELEM(J, 6, 13) = 1.000000 * r1115;
  //DENSE_ELEM(J, 7, -1) = 1.000000 * r191;
  DENSE_ELEM(J, 7, 1) = 1.000000 * r33;
  DENSE_ELEM(J, 7, 7) = -1.000000 * r209;
  DENSE_ELEM(J, 8, 7) = -1.000000 * r49;
  DENSE_ELEM(J, 8, 8) = -1.000000 * r410;
  DENSE_ELEM(J, 8, 9) = 1.000000 * r511;
  DENSE_ELEM(J, 9, 7) = 1.000000 * r49;
  DENSE_ELEM(J, 9, 8) = 1.000000 * r410;
  DENSE_ELEM(J, 9, 9) = -1.000000 * r511;
  DENSE_ELEM(J, 10, 6) = 1.000000 * r178;
  DENSE_ELEM(J, 10, 10) = -1.000000 * r2412;
  DENSE_ELEM(J, 10, 12) = 1.000000 * r1814;
  DENSE_ELEM(J, 11, 3) = -1.000000 * r75;
  DENSE_ELEM(J, 11, 11) = -1.000000 * r713;
  DENSE_ELEM(J, 11, 12) = 1.000000 * r814;
  DENSE_ELEM(J, 12, 2) = -1.000000 * r94;
  DENSE_ELEM(J, 12, 3) = 1.000000 * r75;
  DENSE_ELEM(J, 12, 5) = 1.000000 * r107;
  DENSE_ELEM(J, 12, 11) = 1.000000 * r713;
  DENSE_ELEM(J, 12, 12) = -1.000000 * r814 + -1.000000 * r914;
  DENSE_ELEM(J, 13, 0) = 1.000000 * r252;
  DENSE_ELEM(J, 13, 4) = 1.000000 * r256;
  DENSE_ELEM(J, 13, 13) = -1.000000 * r2615;
}
