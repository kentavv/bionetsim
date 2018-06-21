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

#ifndef STAND_ALONE
#include <iostream>

#include "STEClasses.h"
#include "SymTabMgr.h"
#include "AstNode.h"

#include "fitness_lang.h"
#include "log.h"

extern int yyline;
extern char *yytext;
extern int FFparse();

extern SymTab global;

static int num_err = 0;

using std::endl;

void
FFerror(char *s) {
	if(num_err < 100) {
		if(debugf > 0) {
			ff_log << " line " << yyline << ": ";
		}
		if (s == NULL) {
			if(debugf > 0) {
				ff_log << " line " << yyline << ": syntax error\n";
			}
			pneWarning("SCRIPT: Syntax error detected near line %d.\n", yyline);
		} else {
			if(debugf > 0) {
				ff_log << " line " << yyline << ": " << s << '\n';
			}
			pneWarning("SCRIPT: Parse error (%s) detected near line %d.\n", s, yyline);
		}

		if(++num_err == 100) {
			if(debugf > 0) {
				ff_log << "Too many errors\n";
			}
			pneWarning("SCRIPT: No further errors will be reported.\n");
		}
  }
}

char *myinput;
char *myinputptr;
int myinputlen;

#include "result.h"

PathwayMatrix FFpem;
Result FFresults;

template<class T>
T FFmin(const T &a, const T &b) {
  return a <= b ? a : b; 
}

int
my_yyinput(char *buf, int max_size) {
  int n = FFmin(max_size, myinputlen);
  if( n > 0) {
    memcpy(buf, myinputptr, n);
    myinputptr += n;
    myinputlen -= n;
  }
  return n;
}

bool
fitness_lang_exec(QString &script, const PathwayMatrix &pem, double &fitness, Matrix &mat) {
	num_err = 0;
  fitness = -1.0;
  mat.clear();
  yyline = 1;

  FFpem = pem;

  global.clear();

  if(global.enter(new VarEntry("fitness", 0, FLOAT_TYPE, NULL)) != OK ||
     global.enter(new VarEntry("objective", 0, FLOAT_TYPE, NULL)) != OK) {
			if(debugf > 0) {
				ff_log << "Unable to setup global domain variables\n";
			}
			pneWarning("SCRIPT: Unable to setup global domain variables.\n");
    return false;
  }

  {
    string str = (const char*)script;
    myinputlen = str.length();
    myinput = new char[myinputlen+1];
    myinputptr = myinput;
    strcpy(myinput, str.c_str());
  }
  
	if(FFparse() == 0 && num_err == 0) {
    // 1. check for duplicate names (functions and variables) and ghosted variables
    // 2. typecheck
    
    // start with top and build symbol table entries for global vars and functions
    {
      extern SeqNode *FF_top;
      if(FF_top) {
	if(!FF_top->populateSymTable()) {
					if(debugf > 0) {
						ff_log << "Failed to populate global symbol table\n";
					}
					pneWarning("SCRIPT: Failed to populate global symbol table.\n");
	  return false; 
	}
      }
    }
    
    MethodEntry *me = (MethodEntry*)(global.lookUp("main"));
    if(!me) {
			if(debugf > 0) {
				ff_log << "no main found\n"; 
			}
			pneWarning("SCRIPT: No main() function found in script.\n");
      return false;
    } else {
      Value *v = me->method()->eval();
      if(v && v->type() == ERROR_TYPE) {
				if(debugf > 0) {
					ff_log << "error propagated\n";
				}
				pneWarning("SCRIPT: Error propagated. Examine script for errors.\n");
	return false;
      }
    }

		if(num_err > 0) {
			if(debugf > 0) {
				ff_log << num_err << " errors detected during execution\n";
			}
			pneWarning("SCRIPT: Please correct errors in script.\n");
			return false;
		}

    VarEntry *ve = (VarEntry*)(global.lookUp("fitness"));
    fitness = ve->value()->getFVal();
		if(debugf > 0) {
			ff_log << "Fitness: " << fitness << '\n';
		}
  }
  return true;
}
#endif
