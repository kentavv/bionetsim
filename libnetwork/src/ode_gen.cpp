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
#include <string.h>

#include "ode_gen.h"
#include "matrix.h"

#if 0
static bool
load_matrix(const char *filename, Matrix &m) {
  FILE *file = fopen(filename, "r");
  if(!file) return false;

  char line[1024];
  int line_num = 0;
  int nr=0, nc=0;
  while(fgets(line, 1024, file)) {
    line_num++;
    if(line_num == 1) {
      if(sscanf(line, "%d %d", &nr, &nc) != 2) {
	fclose(file);
	return false;
      }
      m.resize(nr, nc);
    } else {
      char *p = strtok(line, "\t");
      int c = 0;
      while(p && c<nc) {
	double v;
	if(sscanf(p, "%lf", &v) != 1) {
	  fclose(file);
	  return false;
	}
	m(line_num-2, c) = v;
	p = strtok(NULL, "\t");
	c++;
      }
    }
  }

  fclose(file);
  return true;
}
#endif

static void
gen_ode(FILE *fp, const Matrix &var, const Matrix &/*kc*/,
	const IMatrix &re, const IMatrix &pro, ode_format_t format) {
  int nm = re.nrows();
  int nr = re.ncolumns();

  switch(format) {
  case Matlab:
    break;
  case CPP:
  case CVODE:
    fprintf(fp,
	    "static void\n"
	    "f(integer N, real t, N_Vector Y, N_Vector Y2, void *f_data) {\n"
	    "  const double *KC = (double*)f_data;\n\n");
    break;
  case Matrices:
    break;
  }
  
  for(int r=0; r<nr; r++) {
#if 0
    if(kc(r, 0) != 2) {
      // Reaction is not mass-action and we can not handle it
      abort();
    }
#endif
    switch(format) {
    case Matlab:
      fprintf(fp, "r%d = KC(%d)", r+1, r+1);
      break;
    case CPP:
    case CVODE:
      //fprintf(fp, "  double r%d = KC(%d, %d)", r, r, 2);
      fprintf(fp, "  double r%d = KC[%d]", r, r);
      break;
    case Matrices:
      break;
    }
    for(int i=0; i<nm; i++) {
      if(re(i, r) != 0) {
	switch(format) {
	case Matlab:
	  fprintf(fp, " * Y(%d)", i+1);
	  if(re(i, r) != 1) {
	    fprintf(fp, " ^ %d", re(i, r));
	  }
	  break;
	case CPP:
	  if(re(i, r) == 1) {
	    fprintf(fp, " * Y(%d)", i);
	  } else {
	    fprintf(fp, " * pow(Y(%d), %d)", i, re(i, r));
	  }
	  break;
	case CVODE:
	  if(re(i, r) == 1) {
	    fprintf(fp, " * N_VIth(Y, %d)", i);
	  } else {
	    fprintf(fp, " * pow(N_VIth(Y, %d), %d)", i, re(i, r));
	  }
	  break;
	case Matrices:
	  break;
	}
      }
    }
    fprintf(fp, ";\n");
  }
  switch(format) {
  case Matlab:
    fprintf(fp, "Y2 = zeros(%d, 1);\n", nm);
    break;
  case CPP:
    fprintf(fp, "  Matrix Y2(%d, 1);\n", nm);
    break;
  case CVODE:
    break;
  case Matrices:
    break;
  }
  for(int i=0; i<nm; i++) {
    if(var(i, 0) != 5) {
      // Molecule concentration is not constant
      switch(format) {
      case Matlab:
	fprintf(fp, "Y2(%d) =", i+1);
	break;
      case CPP:
	fprintf(fp, "  Y2(%d) =", i);
	break;
      case CVODE:
	fprintf(fp, "  N_VIth(Y2, %d) =", i);
	break;
      case Matrices:
	break;
      }
      bool first_term = true;
      for(int r=0; r<nr; r++) {
	if(re(i, r) != pro(i, r)) {
	  if(first_term) {
	    first_term = false;
	  } else {
	    fprintf(fp, " +");
	  }
	  switch(format) {
	  case Matlab:
	    fprintf(fp, " %d * r%d", pro(i, r) - re(i, r), r+1);
	    break;
	  case CPP:
	  case CVODE:
	    fprintf(fp, " %d * r%d", pro(i, r) - re(i, r), r);
	    break;
	  case Matrices:
	    break;
	  }
	}
      }
      fprintf(fp, ";\n");
    }
  }

  switch(format) {
  case Matlab:
    break;
  case CPP:
  case CVODE:
    // Need to print the control variables yet...
    fprintf(fp,
	    "}\n\n");
    break;
  case Matrices:
    break;
  }
}

static void
gen_jacobian(FILE *fp, const Matrix &var, const Matrix &/*kc*/,
	     const IMatrix &re, const IMatrix &pro, ode_format_t format) {
  int nm = re.nrows();
  int nr = re.ncolumns();

  switch(format) {
  case Matlab:
    break;
  case CPP:
  case CVODE:
    fprintf(fp,
	    "static void\n"
	    "Jac(integer N, DenseMat J, RhsFn f, void *f_data, real t,\n"
	    "    N_Vector Y, N_Vector fy, N_Vector ewt, real h, real uround,\n"
	    "    void *jac_data, long int *nfePtr, N_Vector vtemp1,\n"
	    "    N_Vector vtemp2, N_Vector vtemp3) {\n");
    break;
  case Matrices:
    break;
  }

  for(int r=0; r<nr; r++) {
#if 0
    if(kc(r, 0) != 2) {
      // Reaction is not mass-action and we can not handle it
      abort();
    }
#endif
    for(int j=0; j<nm; j++) {
      if(re(j, r) != 0) {
	// reaction is dependent on j
	switch(format) {
	case Matlab:
	  fprintf(fp, "r%d%d = KC(%d)", r+1, j+1, r+1);
	  break;
	case CPP:
	case CVODE:
	  //fprintf(fp, "  double r%d%d = KC(%d, %d)", r, j, r, 2);
	  fprintf(fp, "  double r%d%d = KC[%d]", r, j, r);
	  break;
	case Matrices:
	  break;
	}
	for(int i=0; i<nm; i++) {
	  if(re(i, r) != 0) {
	    if(i == j) {
	      if(re(i, r) != 1) {
		switch(format) {
		case Matlab:
		  if(re(i, r) - 1 == 1) {
		    fprintf(fp, " * %d * Y(%d)", re(i, r), i+1);
		  } else {
		    fprintf(fp, " * %d * Y(%d) ^ %d", re(i, r), i+1, re(i, r) - 1);
		  }
		  break;
		case CPP:
		  if(re(i, r) - 1 == 1) {
		    fprintf(fp, " * %d * Y(%d)", re(i, r), i);
		  } else {
		    fprintf(fp, " * %d * pow(Y(%d), %d)", re(i, r), i, re(i, r) - 1);
		  }
		  break;
		case CVODE:
		  if(re(i, r) - 1 == 1) {
		    fprintf(fp, " * %d * N_VIth(Y, %d)", re(i, r), i);
		  } else {
		    fprintf(fp, " * %d * pow(N_VIth(Y, %d), %d)", re(i, r), i, re(i, r) - 1);
		  }
		  break;
		case Matrices:
		  break;
		}
	      }
	    } else {
	      switch(format) {
	      case Matlab:
		fprintf(fp, " * Y(%d)", i+1);
		if(re(i, r) != 1) {
		  fprintf(fp, " ^ %d", re(i, r));
		}
		break;
	      case CPP:
		if(re(i, r) == 1) {
		  fprintf(fp, " * Y(%d)", i);
		} else {
		  fprintf(fp, " * pow(Y(%d), %d)", i, re(i, r));
		}
		break;
	      case CVODE:
		if(re(i, r) == 1) {
		  fprintf(fp, " * N_VIth(Y, %d)", i);
		} else {
		  fprintf(fp, " * pow(N_VIth(Y, %d), %d)", i, re(i, r));
		}
		break;
	      case Matrices:
		break;
	      }
	    }
	  }
	}
	fprintf(fp, ";\n");
      }
    }
  }
  switch(format) {
  case Matlab:
    fprintf(fp, "J = sparse(%d, %d);\n", nm, nm);
    break;
  case CPP:
    fprintf(fp, "  Matrix J(%d, %d);\n", nm, nm);
    break;
  case CVODE:
    break;
  case Matrices:
    break;
  }
  for(int i=0; i<nm; i++) {
    if(var(i, 0) != 5) {
      // Molecule concentration is not constant
      for(int j=0; j<nm; j++) {
	bool first_term = true;
	for(int r=0; r<nr; r++) {
	  if(re(i, r) != pro(i, r) && re(j, r) != 0) {
	    if(first_term) {
	      switch(format) {
	      case Matlab:
		fprintf(fp, "J(%d, %d) =", i+1, j+1);
		break;
	      case CPP:
		fprintf(fp, "  J(%d, %d) =", i, j);
		break;
	      case CVODE:
		fprintf(fp, "  DENSE_ELEM(J, %d, %d) =", i, j);
		break;		
	      case Matrices:
		break;
	      }
	      first_term = false;
	    } else {
	      fprintf(fp, " +");
	    }
	    switch(format) {
	    case Matlab:
	      fprintf(fp, " %d * r%d%d", pro(i, r) - re(i, r), r+1, j+1);
	      break;
	    case CPP:
	    case CVODE:
	      fprintf(fp, " %d * r%d%d", pro(i, r) - re(i, r), r, j);
	      break;
	    case Matrices:
	      break;
	    }
	  }
	}
	if(!first_term) {
	  fprintf(fp, ";\n");
	}
      }
    }
  }

  switch(format) {
  case Matlab:
    break;
  case CPP:
  case CVODE:
    // Need to do something with the control variables?
    fprintf(fp,
	    "}\n\n");
    break;
  case Matrices:
    break;
  }
}

void
gen_comments(FILE *fp, const PathwayMatrix &pem, ode_format_t format) {
  int i;
  const char *comment = "";
  switch(format) {
  case Matlab:
    comment = "%";
    break;
  case CPP:
  case CVODE:
    comment = "//";
    break;
  case Matrices:
    break;
  }

  QStringList mol_names = pem.moleculeNames();
  fprintf(fp, "%s Molecule labels\n", comment);
  for(i=0; i<(int)mol_names.count(); i++) {
    fprintf(fp, "%s %s\n", comment, (const char*)mol_names[i]);
  }  

  fprintf(fp, "%s Reactions\n", comment);
  for(i=0; i<(int)pem.rxn_equations.count(); i++) {
    fprintf(fp, "%s %s\n", comment, (const char*)pem.rxn_equations[i]);
  }  
}

void
gen_initial_values(FILE *fp, const PathwayMatrix &pem, ode_format_t format) {
  const Matrix &kc = pem.kc;
  const Matrix &y0 = pem.y0;

  int nm = y0.nrows();
  int nr = kc.nrows();
  int i;

  switch(format) {
  case Matlab:
    fprintf(fp, "Y0 = zeros(%d, 1);\n", nm);
    break;
  case CPP:
  case CVODE:
    fprintf(fp, "  Matrix Y0(%d, 1);\n", nm);
    break;
  case Matrices:
    break;
  }
  QStringList mol_names = pem.moleculeNames();
  for(i=0; i<nm; i++) {
    switch(format) {
    case Matlab:
      fprintf(fp, "Y0(%d) = %f;  %% %s\n", i+1, y0(i, 0), (const char*)mol_names[i]);
      break;
    case CPP:
      fprintf(fp, "  Y0(%d) = %f;  // %s\n", i, y0(i, 0), (const char*)mol_names[i]);
      break;
    case CVODE:
      fprintf(fp, "  N_VIth(Y0, %d) = %f;  /* %s */\n", i, y0(i, 0), (const char*)mol_names[i]);
      break;
    case Matrices:
      break;
    }
  }

  switch(format) {
  case Matlab:
    fprintf(fp, "KC = zeros(%d, 1);\n", nr);
    break;
  case CPP:
  case CVODE:
    fprintf(fp, "  Matrix KC(%d, 1);\n", nr);
    break;
  case Matrices:
    break;
  }
  for(i=0; i<nr; i++) {
    switch(format) {
    case Matlab:
      fprintf(fp, "KC(%d) = %f;  %% %s\n", i+1, kc(i, 0), (const char*)pem.rxn_equations[i]);
      break;
    case CPP:
      fprintf(fp, "  KC(%d) = %f;  // %s\n", i, kc(i, 0), (const char*)pem.rxn_equations[i]);
      break;
    case CVODE:
      fprintf(fp, "  N_VIth(KC, %d) = %f;  /* %s */\n", i, kc(i, 0), (const char*)pem.rxn_equations[i]);
      break;
    case Matrices:
      break;
    }
  }
}

bool
gen_ode(FILE *fp, const PathwayMatrix &pem, ode_format_t format) {
  if(format == Matrices) {
    pem.print(fp);
  } else {
    gen_comments(fp, pem, format);
    gen_initial_values(fp, pem, format);
    int nm = pem.pro.nrows();
    int nc = pem.cvs.count();
    Matrix var(nm, 1);
    for(int i=0; i<nm; i++) {
      if(i<nc) {
	var(i, 0) = 5;
      } else {
	var(i, 0) = 1;
      }
    }
    gen_ode(fp, var, pem.kc, pem.re, pem.pro, format);
    gen_jacobian(fp, var, pem.kc, pem.re, pem.pro, format);
  }
  return true;
}

bool
gen_ode(const char *filename, const PathwayMatrix &pem, ode_format_t format) {
  FILE *file = fopen(filename, "w");
  if(!file) return false;
  bool rv = gen_ode(file, pem, format);
  fclose(file);
  return rv;
}

