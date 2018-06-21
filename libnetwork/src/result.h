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

#ifndef _RESULT_H_
#define _RESULT_H_

#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include "matrix.h"

class Result {
public:
  Result();
  Result(const Result &);
  Result &operator=(const Result &);
  ~Result();

  void clear();
  
  bool success;

  Matrix m;
  Matrix jacobian;
  QPtrList <Matrix> tel;
  QString env;
  QString exp;
  QStringList column_labels;
  QValueVector<bool> cont;
};

#endif
