/*
 * -----------------------------------------------------------------
 * $Revision: 1.3 $
 * $Date: 2005/02/09 22:42:02 $
 * ----------------------------------------------------------------- 
 * Programmers: Alan C. Hindmarsh and Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * Copyright (c) 2002, The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see sundials/cvode/LICENSE.
 * -----------------------------------------------------------------
 * The C function FCVPSol is to interface between the CVSPGMR module
 * and the user-supplied preconditioner solve routine FCVPSOL.
 * Note the use of the generic name FCV_PSOL below.
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "cvspgmr.h"        /* CVSpgmr prototype                       */
#include "fcvode.h"         /* actual function names, prototypes and
			       global variables                        */
#include "nvector.h"        /* definitions of type N_Vector and vector
			       kernels                                 */
#include "sundialstypes.h"  /* definition of type realtype             */

/********************************************************************/

/* Prototype of the Fortran routine */
extern void FCV_PSOL(realtype*, realtype*, realtype*, realtype*, 
		     realtype*, realtype*, realtype*, 
		     realtype*, int*, realtype*, int*);

/***************************************************************************/

void FCV_SPGMRSETPSOL(int *flag, int *ier)
{
  if (*flag == 0) CVSpgmrSetPrecSolveFn(CV_cvodemem, NULL);
  else CVSpgmrSetPrecSolveFn(CV_cvodemem, FCVPSol);
}

/***************************************************************************/

/* C function FCVPSol to interface between CVODE and a Fortran subroutine
   FCVPSOL for solution of a Krylov preconditioner.
   Addresses of t, gamma, delta, lr, y, fy, vtemp, ewt, r, and z are
   passed to FCVPSOL, using the routine N_VGetArrayPointer from NVECTOR.
   A return flag ier from FCVPSOL is returned by FCVPSol.
   Auxiliary data is assumed to be communicated by Common blocks. */

int FCVPSol(realtype t, N_Vector y, N_Vector fy, 
            N_Vector r, N_Vector z,
            realtype gamma, realtype delta,
            int lr, void *P_data, N_Vector vtemp)
{
  N_Vector ewt;
  realtype *ydata, *fydata, *vtdata, *ewtdata, *rdata, *zdata;

  int ier = 0;

  CVodeGetErrWeights(CV_cvodemem, &ewt);

  ydata   = N_VGetArrayPointer(y);
  fydata  = N_VGetArrayPointer(fy);
  vtdata  = N_VGetArrayPointer(vtemp);
  ewtdata = N_VGetArrayPointer(ewt);
  rdata   = N_VGetArrayPointer(r);
  zdata   = N_VGetArrayPointer(z);

  FCV_PSOL(&t, ydata, fydata, vtdata, &gamma, ewtdata, &delta,
           rdata, &lr, zdata, &ier);

  return(ier);
}
