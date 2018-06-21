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

#include <qstring.h>

#include "matrix.h"
#include "pathway_load.h"
#include "scan_params.h"

ScanParams::ScanParams() {
}

bool
ScanParams::load(const QString &filename) {
  FILE *file = fopen(filename, "r");
  if(!file) return false;

  int line_num = 0;
  char line[1024];
  while(fgets(line, 1024, file)) {
    line_num++;
    QString str = line;
    int p = str.find('#');
    if(p >= 0) {
      str.truncate(p);
    }
    str.stripWhiteSpace();
    QString label, value;
    label = str.section(':', 0, 0); label = label.stripWhiteSpace();
    value = str.section(':', 1);    value = value.stripWhiteSpace();
    if(label.isEmpty()) continue;
    else if(label == "Network file") {
      network_filename = value;
      if(!pathwayLoad(value, pem, fitness)) {
	fprintf(stderr, "Unable to load %s\n", (const char*)value);
	fclose(file);
	return false;
      }      
    } else if(label == "Range") {
      min = value.section(' ', 0, 0).toDouble();
      max = value.section(' ', 1).toDouble();
    } else if(label == "Steps") {
      steps = value.toInt();
    } else {
      fprintf(stderr, "Parse error on line %d\n", line_num);
      fclose(file);
      return false;
    }
  }
  
  fclose(file);

  return true;
}

void
ScanParams::print(FILE *fp) {
  fprintf(fp, "Network file: %s\n", (const char*)network_filename);
  fprintf(fp, "Range: %f %f\n", min, max);
  fprintf(fp, "Steps: %d\n", steps);
}
