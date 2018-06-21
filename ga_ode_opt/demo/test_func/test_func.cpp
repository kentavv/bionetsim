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

#include <cfloat>

#include <qcolor.h>
#include <qimage.h>

#include "matrix.h"

double
func(double x, double y) {
  return 2 * (3.75 - sqrt(x*x + y*y)) + sin(1.55 * 3.14159 * (3.75 - sqrt(x*x + y*y)) * (6.0 - sqrt(x*x + y*y)) / 3.0);
}


int
main(int argc, char *argv[]) {
  int w=600, h=600;
  int x, y;
  double sx = (3.75 - -3.75) / w;
  double sy = (3.75 - -3.75) / h;

  Matrix m(w, h);
  double min = DBL_MIN, max = -DBL_MAX;

  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      double v = func(x * sx - 3.75, y * sy - 3.75);
      m(x, y) = v;
      if(v < min) min = v;
      if(v > max) max = v;
    }
  }

  QImage image(w, h, 32);
  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      double v = (m(x, y) - min) / (max - min);
      image.setPixel(x, y, QColor((int)((1.0-v)*240+0.5), 255, 255, QColor::Hsv).rgb());
    }
  }

  image.save("func.ppm", "PPM");

  return 0;
}
