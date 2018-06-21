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

#ifndef _MATH_UTILS_H_
#define _MATH_UTILS_H_

namespace GenoDYN {

inline int
roundToInt(double x) {
  return (x < 0.0) ? (int)(x - 0.5) : int(x + 0.5);
}

inline double sqr(double x) { return x * x; }

template<class T>
inline T
min(const T &a, const T &b) { return a <= b ? a : b; }

template<class T>
inline T
max(const T &a, const T &b) { return a >= b ? a : b; }

template<class T>
inline T clamp(T x, T min, T max) {
  if(x < min) return min;
  if(x > max) return max;
  return x;
}

}

#endif
