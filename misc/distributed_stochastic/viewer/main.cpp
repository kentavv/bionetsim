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

#include <qapplication.h>
#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>

#include "histogram_dialog.h"
#include "matrix.h"

int
main(int argc, char *argv[]) {
  qApp = new QApplication(argc, argv);

  QPtrList<Matrix> tel;
  QStringList labels;

  {
    tel.clear();
    for(int i=1; i<argc; i++) {
      const char *fn = argv[i];
      
      printf("Loading %s\n", fn);
      
      FILE *file = fopen(fn, "r");
      if(!file) {
	fprintf(stderr, "Unable to open %s for reading\n", fn);
	continue;
      }
      
      {
	QFile f;
	f.open(IO_ReadOnly, file);
	
	QString str;
	
	f.readLine(str, 102400);
	QStringList labels2 = QStringList::split('\t', str);
	if(i == 1) {
	  labels = labels2;
	} else {
	  if(labels != labels2) {
	    fprintf(stderr, 
		    "The labels used in %s do not match first file loaded\n",
		    fn);
	  }
	}
      }
      
      Matrix mat;
      mat.load(file);
      fclose(file);
      
      if(i == 1) {
	tel.append(new Matrix(mat));
      } else {
	if(mat.nrows() != tel.at(0)->nrows() ||
	   mat.ncolumns() != tel.at(0)->ncolumns()) {
	  fprintf(stderr, "File %s is of a different size than the first matrix\n", fn);
	  continue;
	}
	tel.append(new Matrix(mat));
      }
    }
    
    printf("%d matrices loaded\n", tel.count());
  }

  if(labels.first() == "Time") labels.pop_front();

  QValueVector<bool> cont(labels.count(), false);


  HistogramDialog *hd = new HistogramDialog(NULL);
  hd->resize(450, 400);
  hd->setData(tel, labels, cont);
  hd->show();

  return qApp->exec();
}
