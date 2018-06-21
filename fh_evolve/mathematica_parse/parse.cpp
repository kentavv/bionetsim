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

#include <iostream>
#include <stdlib.h>
#include <qfile.h>
#include <qstring.h>
#include <qregexp.h>

using namespace std;

int
main(int argc, char *argv[]) {
  for(int i=1; i<argc; i++) {
  const char *fn = argv[i];
  QFile file(fn);
  if(!file.open(IO_ReadOnly)) {
    cerr << "Unable to open " << fn << endl;
    exit(EXIT_FAILURE);
  }
  QByteArray ba = file.readAll();
  QString str(ba);

  QRegExp re("\\{(\\{[^{}]*\\}?(,\\s*\\{[^{}]*\\})*)\\}");
  int pos = 0;
  while ( pos >= 0 ) {
    pos = re.search( str, pos );
    if ( pos > -1 ) {
      QString match = re.cap(0);
      cout << match << endl;
      
      match = re.cap(1);
      cout << match << endl;
      
      QRegExp re2("\\{([^{}]*)\\}");
      int pos2 = 0;
      while ( pos2 >= 0 ) {
        pos2 = re2.search(match, pos2);
        if ( pos2 > -1 ) {
	  QString match2 = re2.cap( 1 );
	  cout << pos2 << '\t' << match2 << endl;
	  
#if 1
	  QRegExp re3("([a-zA-Z]+[a-zA-Z0-9]*)"  // Variable name
		      "\\s*->\\s*"               // Assignment
		      "([+-]?[0-9]*\\.?[0-9]+)"  // Real part
		      "(`[0-9]*\\.?[0-9]+)?"     // Optional precision of real part
		      "(\\*\\^([0-9]+))?"        // Optional exponent of real part
		      "(\\s*(([+-]\\s*"          // +/- of the imaginary part
		      "([+-]?[0-9]*\\.?[0-9]+))" // Imaginary part
		      "(`[0-9]*\\.?[0-9]+)?"     // Optional precision of imaginary part
		      ")\\*I)?");                // The I part of the imaginary part
	  int pos3 = 0;
	  while ( pos3 >= 0 ) {
	    pos3 = re3.search(match2, pos3);
	    if ( pos3 > -1 ) {
	      QString var      = re3.cap( 1 );
	      QString real     = re3.cap( 2 );
	      QString exponent = re3.cap( 5 );
	      QString imag     = re3.cap( 8 );
	      cout << pos2 << '\t' << pos3 << '\t' << var << '\t' << real << '\t' << exponent << '\t' << imag << endl;
	      pos3  += re3.matchedLength();
	    }
	  }
#endif
	  
	  pos2  += re2.matchedLength();
	}
      }
      pos += re.matchedLength();
    }
  }
  }
  
  return 0;
}

