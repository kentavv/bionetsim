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

#ifdef STAND_ALONE
#include <iostream>

#include "STEClasses.h"
#include "SymTabMgr.h"
#include "AstNode.h"

extern int yyline;
extern char *yytext;
extern int FFparse();

extern SymTab global;
//SymTabMgr symTabMgr(&global);

using namespace std;

static int num_err = 0;

void
FFerror(char *s)
{
  cerr << " line " << yyline << ": ";
  if (s == NULL) cerr << "syntax error";
  else cerr << s;
  cerr << endl;
  if(++num_err > 50) {
    cerr << "Too many errors\n";
  }
}

char *myinput;
char *myinputptr;
int myinputlen;

/*
template<class T>
T min(const T &a, const T &b) {
  return a <= b ? a : b;
}
*/

int
my_yyinput(char *buf, int max_size) {
  int n = min(max_size, myinputlen);
  if( n > 0) {
    memcpy(buf, myinputptr, n);
    myinputptr += n;
    myinputlen -= n;
  }
  return n;
}

#if 0
int symTabErrCheck(SymTabEntry *se)
{
  if (symTabMgr.enter(*se) != OK) {
    cout << " line " << yyline << ": ";
	cout << "Error: Duplicate Symbol `" << se->name() << "'\n";
	return 0;
  }
  return 1;
}
#endif

int
main(int argc, char *argv[])
{
  if(argc != 2) {
    cerr << argv[0] << ": <filename>\n";
    exit(EXIT_FAILURE);
  }
  
  if(global.enter(new VarEntry("fitness", 0, FLOAT_TYPE, NULL)) != OK ||
     global.enter(new VarEntry("objective", 0, FLOAT_TYPE, NULL)) != OK) {
    cerr << "Unable to setup global domain variables\n";
    exit(EXIT_FAILURE);
  }

  {
    string str;
    char line[1024];
    FILE *file = fopen(argv[1], "r");
    if(!file) {
      cerr << "Unable to open " << argv[1] << '\n';
      exit(EXIT_FAILURE);
    }
    while(fgets(line, 1024, file)) {
      str += line;
    }
    fclose(file);
    myinputlen = str.length();
    myinput = new char[myinputlen+1];
    myinputptr = myinput;
    strcpy(myinput, str.c_str());
  }
  
  if(FFparse() == 0) {
    // 1. check for duplicate names (functions and variables) and ghosted variables
    // 2. typecheck
    
    // start with top and build symbol table entries for global vars and functions
    {
      extern SeqNode *FF_top;
      if(FF_top) {
	if(!FF_top->populateSymTable()) {
	  puts("Failed to populate global symbol table");
	  exit(EXIT_FAILURE);
	}
      }
    }
    
    MethodEntry *me = (MethodEntry*)(global.lookUp("main"));
    if(!me) {
      puts("no main found"); 
    } else {
      Value *v = me->method()->eval();
      if(v && v->type() == ERROR_TYPE) {
	//puts("error propagated");
      }
    }
  }
  return 0;
}
#endif
