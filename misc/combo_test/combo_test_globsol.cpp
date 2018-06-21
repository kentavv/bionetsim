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
#include <unistd.h>

#include <qstring.h>

#include "log.h"
#include "pathway_load.h"

void
run_globsol(const double *a) {
  for(int i=0; i<9; i++) {
    printf("%f ", a[i]);
  }
  putchar('\n');
  char hn[1024];
  gethostname(hn, 1024);
  int pid = getpid();
  char fn_base[64];
  char fn_f90[64];
  char fn_dt[64];
  sprintf(fn_base, "gs_%s_%d", hn, pid);
  sprintf(fn_f90, "%s.f90", fn_base);
  sprintf(fn_dt, "%s.DT1", fn_base);

  FILE *file;

  file = fopen(fn_f90, "w");
  fprintf(file,
	  "!  This is a simple example of mixed equality and inequality\n"
	  "!  constraints, one of which is binding at the solution.\n"
	  "\n"
	  "PROGRAM VELDEN1\n"
	  "\n"
	  "  USE CODELIST_CREATION\n"
	  "  IMPLICIT NONE\n"
	  "\n"
	  "      TYPE(CDLVAR), DIMENSION(4):: X\n"
	  "      TYPE(CDLLHS):: PHI\n"
	  "      TYPE(CDLEQ), DIMENSION(4) :: C\n"
	  "\n"
	  "      TYPE(CDLVAR), DIMENSION(4) :: C_TMP\n"
	  "      TYPE(CDLVAR) :: SUM\n"
	  "      INTEGER :: I\n"
	  "\n"
	  "\n"
	"      CALL INITIALIZE_CODELIST(X)\n"
	  "\n"
	  "      CALL SET_RESIDUALS( X(1), X(2), X(3), X(4) )\n"
	  "\n"
	  "      SUM = 0\n"
	  "      DO I = 1,4\n"
	  "         C(I) = C_TMP(I)\n"
	  "         SUM = SUM + C_TMP(I)**2\n"
	  "      END DO\n"
	  "      PHI = SUM\n"
	  "\n"
	  "      CALL FINISH_CODELIST\n"
	  "\n"
	  "CONTAINS\n"
	  "\n"
	  "   SUBROUTINE SET_RESIDUALS( a, b, BB, d )\n"
	  "      IMPLICIT NONE\n"
	  "      TYPE(CDLVAR) :: a, b, BB, d\n"
	  "      DOUBLE PRECISION :: k1, k2, k3, k4, k5, k6, k7, k8, k9\n"
	"\n"
	  "    ! Constants\n");
  for(int i=0; i<9; i++) {
    fprintf(file, 
	    "      k%d  = %f\n", i+1, a[i]);
  }
  fprintf(file,
	  "\n"
	  "    ! Polynomials\n"
	  "      C_TMP(1) = k1*1 -k2*A -k5*A*BB -k7*b*A + k8*(1-b)\n"
	  "      C_TMP(2) = -k7*b*A + k8*(1-b)\n"
	  "      C_TMP(3) = k3*b -k4*BB -k5*A*BB + k9*(1-b)\n"
	  "      C_TMP(4) = k5*A*BB -k6*d\n"
	  "   END SUBROUTINE SET_RESIDUALS\n"
	  "\n"
	  "END PROGRAM VELDEN1\n");
  fclose(file);
 
  file = fopen(fn_dt, "w");
  fprintf(file,
	  "1D-10\n"
	  "0 20\n"
	  "0 1.1\n"
	  "0 20\n"
	  "0 20\n"
	  "F F\n"
	  "F F\n"
	  "F F\n"
	  "F F\n");
  fclose(file);

  char cmd[1024];
  sprintf(cmd, "~/globsol_clean/globsol %s 1", fn_base);
  system(cmd);
  sprintf(cmd, "cat %s.OT1 | ~/globsol_clean/dual/parse.pl", fn_base);
  system(cmd);
}

int
main(int argc, char *argv[]) {
  if(argc != 2) {
    fprintf(stderr, "%s: <params filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *params_filename = argv[1];

  setLogFP(stdout, NULL, NULL, NULL);

  printf("Base: ");
  double a[9] = {.2, .0085, .37, .034, .72, .53, .19, .42, .027};
  run_globsol(a);
  
  FILE *file = fopen(params_filename, "r");
  if(!file) {
    fprintf(stderr, "Unable to open %s\n", params_filename);
    exit(EXIT_FAILURE);
  }
  char line[1024];
  int i=0;
  while(fgets(line, 1024, file)) {
    int j = 0;
    char *p = strtok(line, " ");
    while(p) {
      a[j] = atof(p);
      p = strtok(NULL, " ");
      j++;
    }
    if(j != 9) {
      fprintf(stderr, "Insufficient params founnd\n");
      exit(EXIT_FAILURE);
    }
    printf("%d: ", i);
    run_globsol(a);
    i++;
  }

  fclose(file);

  return 0;
}
