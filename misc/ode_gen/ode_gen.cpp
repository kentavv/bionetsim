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

#include "ode_gen.h"
#include "pathway_load.h"

int
main(int argc, char *argv[]) {
  if(argc != 5) {
    fprintf(stderr, "%s: <pathway filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  const char *filename = argv[1];

  Editor editor;
  Simulation simulation;
  QList<PathwayCanvasItem> items;
  if(!pathwayLoad(filename, editor, simulation, items)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }
  
  PathwayMatrix pem;
  pem.setup(editor, simulation, items);

  ode_format_t format = CVODE;
  if(!gen_ode(filename, pem, format)) {
    fprintf(stderr, "Unable to convert %s\n", filename);
    exit(EXIT_FAILURE);
  }

  return 0;
}
