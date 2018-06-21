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

#include "RP_AstNode.h"

extern int RPyyline;
extern char *yytext;
extern int RPparse();

using namespace std;

static int num_err = 0;

void
RPerror(char *s)
{
  cerr << " line " << RPyyline << ": ";
  if (s == NULL) cerr << "syntax error";
  else cerr << s;
  cerr << endl;
  if(++num_err > 50) {
    cerr << "Too many errors\n";
  }
}

char *RPinput;
char *RPinputptr;
int RPinputlen;

int
RPyyinput(char *buf, int max_size) {
  int n = min(max_size, RPinputlen);
  if( n > 0) {
    memcpy(buf, RPinputptr, n);
    RPinputptr += n;
    RPinputlen -= n;
  }
  return n;
}

extern int RPdebug;

int
main(int argc, char *argv[])
{
  RPdebug=1;
  if(argc != 2) {
    cerr << argv[0] << ": <filename>\n";
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
    RPinputlen = str.length();
    RPinput = new char[RPinputlen+1];
    RPinputptr = RPinput;
    strcpy(RPinput, str.c_str());
  }
  
  if(RPparse() == 0) {
    // 1. check for duplicate names (functions and variables) and ghosted variables
    // 2. typecheck
    
    {
      extern list<ReactionNode> RP_top;
      cout << "RP_top.size(): " << RP_top.size() << '\n';
      list<ReactionNode>::const_iterator i;
      for(i=RP_top.begin(); i!=RP_top.end(); i++) {
	i->print(cout);
      }
    }
  }
  return 0;
}
#endif
