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

#ifndef _SCAN_PARAMS_H_
#define _SCAN_PARAMS_H_

#include <stdio.h>

#include <qstring.h>

#include "fitness.h"
#include "matrix.h"
#include "pathway_matrix.h"

class ScanParams {
public:
  ScanParams();

  bool load(const QString &filename);
  void print(FILE *fp = stdout);
  
  PathwayMatrix pem;
  Fitness fitness;
  
  QString network_filename;

  double min, max;
  int steps;
  
protected:
  ScanParams(const ScanParams &);
  ScanParams &operator = (const ScanParams &) const;
};

#endif
