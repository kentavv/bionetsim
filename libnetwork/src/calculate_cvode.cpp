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
#include <math.h>

#ifdef GUI
#include <qapplication.h>
#endif

#include "log.h"
#include "math_utils.h"
#include "matrix.h"
#include "pathway_matrix.h"

#ifdef EIGEN
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#endif

#include "sundialstypes.h"
#include "cvode.h"
#include "cvdense.h"
#include "nvector_serial.h"
#include "dense.h"

#define Ith(v,i)    NV_Ith_S(v,i)
#define IJth(A,i,j) DENSE_ELEM(A,i,j)

static void genericF(realtype t, N_Vector y, N_Vector ydot, void *f_data);

static void genericJac(long int N, DenseMat J, realtype t,
		       N_Vector y, N_Vector fy, void *jac_data, 
		       N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

/***************************** Main Program ******************************/

int
calculate_results_cvode(const PathwayMatrix &pem, Matrix &results, Matrix &jacobian) {
  N_Vector y = NULL;
  realtype t, reltol, abstol;
  void *cvode_mem = NULL;
  int flag, i, k, m;
  int ncvs = pem.cvs.count();
  int nmol = pem.re.nrows();
  int neq = nmol - ncvs;

  double t0 = pem.simulation.timeBegin();
  double tstep = pem.simulation.timeStepSize();
  double tend = pem.simulation.timeEnd();
  
  int nsteps = GenoDYN::roundToInt((tend - t0)/tstep);
  results.resize(nsteps+1, pem.nPlotted()+1);
  jacobian.resize(neq, neq);

#ifdef GUI
  if(pem.progress) {
    pem.progress->setTotalSteps(100);
  }
#endif
  
  if(neq == 0) {
    for(i=0; i<=nsteps; i++) {
      double tout = t0+i*tstep;
      results(i, 0) = tout;
      for(m=0, k=1; m<ncvs; m++) {
	if(pem.plotted(m)) {
	  results(i, k) = pem.cvs[m]->concentration(tout);
	  k++;
	}
      }
    }
    return 1;
  }

  y = N_VNew_Serial(neq);
  if(!y) {
    pneFatal("N_VNew_Serial failed.\n");
    return -2;
  }
  //abstol = N_VNew(neq, machEnv); 

  reltol = pem.simulation.relativeTolerance();
  //abstol = pem.simulation.absoluteTolerance();
  abstol = pem.atol(0, 0);
  for(i=0; i<neq; i++) {
    Ith(y, i)      = pem.y0(i+ncvs, 0);
  }

  cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
  if(!cvode_mem) {
    pneFatal("CVodeCreate failed.\n");
    return -2;
  }

  flag = CVodeMalloc(cvode_mem, genericF, t0, y, CV_SS, &reltol, &abstol);
  if(flag != CV_SUCCESS) {
    pneFatal("CVodeMalloc failed.\n");
    return -2;
  }

  flag = CVodeSetMinStep(cvode_mem, GenoDYN::max(1e-16, pem.simulation.minStep())); // 0.0 is not allowed
  if(flag != CV_SUCCESS) {
    pneFatal("CVodeSetMinStep failed.\n");
    return -2;
  }

  flag = CVodeSetMaxStep(cvode_mem, pem.simulation.maxStep());
  if(flag != CV_SUCCESS) {
    pneFatal("CVodeSetMaxStep failed.\n");
    return -2;
  }

  flag = CVDense(cvode_mem, neq);
  if(flag != CV_SUCCESS) {
    pneFatal("CVDense failed.\n");
    return -2;
  }

  flag = CVDenseSetJacFn(cvode_mem, genericJac);
  if(flag != CV_SUCCESS) {
    pneFatal("CVDenseSetJacFn failed.\n");
    return -2;
  }

  flag = CVDenseSetJacData(cvode_mem, (void*)&pem);
  if(flag != CV_SUCCESS) {
    pneFatal("CVDenseSetJacData failed.\n");
    return -2;
  }

  flag = CVodeSetFdata(cvode_mem, (void*)&pem);
  if(flag != CV_SUCCESS) {
    pneFatal("CVodeSetFdata failed.\n");
    return -2;
  }

  results(0, 0) = t0;
  for(m=0, k=1; m<ncvs; m++) {
    if(pem.plotted(m)) {
      results(0, k) = pem.cvs[m]->concentration(t0);
      k++;
    }
  }
  for(m=0; m<neq; m++) {
    if(pem.plotted(m+ncvs)) {
      results(0, k) = Ith(y, m);
      k++;
    }
  }

  int absadj = 0;
  N_Vector y2 = NULL;
  if(pem.simulation.simulateToSteadyState()) {
    y2 = N_VNew_Serial(neq);
    if(!y2) {
      pneFatal("N_VNew_Serial failed.\n");
      return -2;
    }
  }
  for(i=1; i<=nsteps; i++) {
    //printf("%d/%d\n", i, nsteps);
    double tout = t0+i*tstep;
    flag = CVode(cvode_mem, tout, y, &t, CV_NORMAL);
    if(flag != CV_SUCCESS) {
      //fprintf(stderr, "CVode failed, flag=%d.\n", flag);
      if(absadj < 10) {
	//fprintf(stderr, "Attempting to adjust abstol to %g\n", abstol * 10.0);
	abstol *= 10.0;
	absadj++;
	i--;
	continue;
      } else {
	break;
      }
    }
    
    results(i, 0) = t;
    for(m=0, k=1; m<ncvs; m++) {
      if(pem.plotted(m)) {
	results(i, k) = pem.cvs[m]->concentration(t);
	k++;
      }
    }
    for(m=0; m<neq; m++) {
      if(pem.plotted(m+ncvs)) {
	results(i, k) = Ith(y,m);
	k++;
      }
    }    
    
    if(pem.simulation.simulateToSteadyState()) {
      bool ss = true;
      genericF(tout, y, y2, (void*)&pem);
      for(int j=0; j<neq; j++) {
	if(fabs(Ith(y2, j)) > pem.simulation.steadyStateTolerance()) {
	  ss = false;
	  break;
	}
      }
      if(ss) {
	pneDebug("Steady state reached at t=%g\n", t);
	break;
      }
    }

#ifdef GUI
    int prog = int(i*100.0/nsteps + 0.5);
    if(pem.progress && prog > pem.progress->progress()) {
      pem.progress->setProgress(prog);
      qApp->processEvents(100);
      if(pem.quit) break;
    }
#endif
  }
  
  if(!pem.simulation.simulateToSteadyState()) {
    for(; i<=nsteps; i++) {
      double tout = t0+i*tstep;
      results(i, 0) = tout;
      for(m=0; m<results.ncolumns()-1; m++) {
	results(i, m+1) = -1;
      }
    }
  } else {
    N_VDestroy_Serial(y2);
    if(i < nsteps) {
      Matrix results2(i, results.ncolumns());
      for(int r=0; r<i; r++) {
	for(int c=0; c<results.ncolumns(); c++) {
	  results2(r, c) = results(r, c);
	}
      }
      results = results2;
    } else if(i > nsteps) {
      pneWarning("Steady state not reached in selected time period\n");
      flag = !CV_SUCCESS;
    }
  }
  
  if(absadj > 0 && i == nsteps + 1) {
    pneWarning("Successfully completed with abstol adjusted to %g from %g\n", abstol, pem.atol(0, 0));
  }

  {
    DenseMat J = DenseAllocMat(neq);
    genericJac(0, J, t, y, NULL, (void*)&pem, NULL, NULL, NULL);
    for(int r=0; r<neq; r++) {
      for(int c=0; c<neq; c++) {
	jacobian(r, c) = IJth(J, r, c);
      }
    }
    DenseFreeMat(J);
  }

#ifdef EIGEN
  {
    gsl_matrix *A = gsl_matrix_alloc(neq, neq);
    for(int i=0; i<neq; i++) {
      for(int j=0; j<neq; j++) {
        gsl_matrix_set(A, i, j, jacobian(i,j));
      }
    }
    gsl_eigen_symm_workspace *w = gsl_eigen_symm_alloc(neq);
    gsl_vector *eval = gsl_vector_alloc(neq);
    gsl_eigen_symm(A, eval, w);
    printf("EVals: ");
    for(int i=0; i<neq; i++) {
      printf("%f\t", gsl_vector_get(eval, i));
    }
    putchar('\n');
    gsl_eigen_symm_free(w);
    gsl_matrix_free(A);
    gsl_vector_free(eval);
  }
#endif

  N_VDestroy_Serial(y);   
  //N_VDestroy_Serial(abstol);   
  CVodeFree(cvode_mem);
 
#ifdef GUI
  if(pem.progress) {
    pem.progress->reset();
  }
  if(pem.quit) return -1;
#endif

  return (flag == CV_SUCCESS) ? 1 : -1;
}

/***************** Functions Called by the CVODE Solver ******************/

/* f routine. Compute f(t,y). */

static void
genericF(realtype t, N_Vector y, N_Vector ydot, void *f_data) {
  const PathwayMatrix *pem = (PathwayMatrix*)f_data;

  const Matrix &kc  = pem->kc;
  const IMatrix &pro = pem->pro;
  const IMatrix &re  = pem->re;  

  const IMatrix &isto = pem->isto;
  const IMatrix &ire  = pem->ire;
  
  int i;
  int nm = re.nrows();
  int nr = re.ncolumns();
  int nc = pem->cvs.count();
  int ne = nm - nc;
  
  for(i=0; i<ne; i++) {
    Ith(ydot, i) = 0.0;
  }

  for(int r=0; r<nr; r++) {
    double der = kc(r, 0);

#if 0
    for(i=0; i<nm; i++) {
      if(re(i, r) != 0) {
	double v;
	
	if(i < nc) {
	  v = pem->cvs[i]->concentration(t); 
	} else {
	  v = Ith(y, i-nc);
	}
	
	if(re(i, r) == 1) {
	  der *= v;
	} else {
	  der *= pow(v, re(i, r));
	}
      }
    }
    for(i=nc; i<nm; i++) {
      if(pro(i, r) != re(i, r)) {
	Ith(ydot, i-nc) += der * (pro(i, r) - re(i, r));
      }
    }
    
#else
    
    nm = ire(0, r);
    for(i=1; i<=nm; i++) {
      int i2 = ire(i, r);
      double v;
	
      if(i2 < nc) {
	v = pem->cvs[i2]->concentration(t); 
      } else {
	v = Ith(y, i2-nc);
      }
	
      if(re(i2, r) == 1) {
	der *= v;
      } else {
	der *= pow(v, re(i2, r));
      }
    }

    nm = isto(0, r);
    for(i=1; i<=nm; i++) {
      int i2 = isto(i, r);
      Ith(ydot, i2-nc) += der * (pro(i2, r) - re(i2, r));
    }
#endif  
  }
}

/* Jacobian routine. Compute J(t,y). */

static void
genericJac(long int /*N*/, DenseMat J, realtype t,
           N_Vector y, N_Vector /*fy*/, void *jac_data, 
	   N_Vector /*tmp1*/, N_Vector /*tmp2*/, N_Vector /*tmp3*/) {
  const PathwayMatrix *pem = (PathwayMatrix*)jac_data;

  const Matrix &kc  = pem->kc;
  const IMatrix &pro = pem->pro;
  const IMatrix &re  = pem->re;
  
  //const IMatrix &isto = pem->isto;
  const IMatrix &ire  = pem->ire;
  const IMatrix &jmat = pem->jmat;

  int nm = re.nrows();
  //int nr = re.ncolumns();
  int nc = pem->cvs.count();
  //int ne = nm - nc;

  int i, j, k, m;
  double v;

  for(i=0; i<nm-nc; i++) {
    for(j=0; j<nm-nc; j++) {
      IJth(J, i, j) = 0.0;
    }
  }
      
#if 0
  for(i=nc; i<nm; i++) {
    //if(pem->cvs[i]) continue;
    for(j=nc; j<nm; j++) {
      double der = 0.0;
      for(k=0; k<nr; k++) {
	if(pro(i, k) != re(i, k) && re(j, k) != 0.0) {
	  double der2 = kc(k, 0);

	  for(int m=0; m<nm; m++) {
	    if(m == j) continue;
	    
	    if(re(m, k) != 0) {
	      if(m < nc) {
		v = pem->cvs[m]->concentration(t); 
	      } else {
		v = Ith(y, m-nc);
	      }
	      
	      if(re(m, k) == 1) {
		der2 *= v;
	      } else {
		der2 *= pow(v, re(m, k));
	      }
	    }
	  }
	  
	  if(re(j, k) != 1.0) {
	    if(j < nc) {
	      v = pem->cvs[j]->concentration(t); 
	    } else {
	      v = Ith(y, j-nc);
	    }
	    der2 *= re(j, k) * pow(v, re(j, k) - 1.0);
	  }
	  
	  der += der2 * (pro(i, k) - re(i, k));
	}
      }
      IJth(J, i-nc, j-nc) = der;
    }
  }

#else

  int nj = jmat.nrows();
  for(m=0; m<nj; m++) {
    i = jmat(m, 0);
    j = jmat(m, 1);
    k = jmat(m, 2);
    double der = kc(k, 0);
    
    int nm = ire(0, k);
    for(int m=1; m<=nm; m++) {
      int m2 = ire(m, k);
      
      if(m2 == j) continue;

      if(m2 < nc) {
	v = pem->cvs[m2]->concentration(t); 
      } else {
	v = Ith(y, m2-nc);
      }
      
      if(re(m2, k) == 1) {
	der *= v;
      } else {
	der *= pow(v, re(m2, k));
      }
    }
    
    if(re(j, k) != 1.0) {
      if(j < nc) {
	v = pem->cvs[j]->concentration(t); 
      } else {
	v = Ith(y, j-nc);
      }
      der *= re(j, k) * pow(v, re(j, k) - 1.0);
    }
    
    IJth(J, i-nc, j-nc) += der * (pro(i, k) - re(i, k));
  }

  //DensePrint(J);

#endif
}
