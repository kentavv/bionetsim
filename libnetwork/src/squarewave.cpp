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

#include <math.h>

#include "nr.h"
#include "squarewave.h"

static double
betai_store(double x) {
  static double px=-1, pv=-1;

  if(x != px) {
    pv = betai(10.0, 10.0, x);
    px = x;
  }
  
  return pv;
}

double
squarewave(double t, double val1, double val2, double dur1, double dur2,
	   double trn1, double trn2, double phase_shift) {
  double val[2] = {val1, val2};
  double dur[2] = {dur1, dur2};
  double trn[2] = {trn1, trn2};
  return squarewave(t, val, dur, trn, phase_shift);
}

double
squarewave(double t, const double *val, const double *dur,
	   const double *trn, double phase_shift) {
  double v = 0.0;  
  double period = dur[0] + trn[0] + dur[1] + trn[1];

  if(t <= phase_shift) return val[0];
  t -= phase_shift;
  
  double t2 = fmod(t, period);

  if(0.0 <= t2 && t2 < dur[0]) {
    v = val[0];
  } else if(dur[0] <= t2 && t2 < dur[0] + trn[1]) {
    //v = betai(10.0, 10.0, (t2-dur[0])/trn[1]);
    v = betai_store((t2-dur[0])/trn[1]);
    if(val[0] < val[1]) {
      v = v * (val[1] - val[0]) + val[0];
    } else {
      v = (1.0-v) * (val[0] - val[1]) + val[1];
    }
  } else if(dur[0] + trn[1] <= t2 && t2 < period - trn[0]) {
    v = val[1];
  } else if(period - trn[0] <= t2) {
    //v = betai(10.0, 10.0, (t2-(period-trn[0]))/trn[0]);
    v = betai_store((t2-(period-trn[0]))/trn[0]);
    if(val[0] < val[1]) {
      v = (1.0-v) * (val[1] - val[0]) + val[0];
    } else {
      v = v * (val[0] - val[1]) + val[1];
    }
  }

  //printf("%d %f %f %f %f\n", cur_env_, trn[0], trn[1], t, v);
  
  return v;
}
