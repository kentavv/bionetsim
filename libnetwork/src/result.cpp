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

#include <qstring.h>

#include "matrix.h"
#include "result.h"

Result::Result() {
  clear();
}

Result::Result(const Result &r) 
  : success(r.success),
    m(r.m),
    jacobian(r.jacobian),
    tel(r.tel),
    env(r.env),
    exp(r.exp),
    column_labels(r.column_labels),
    cont(r.cont)
{}

Result &
Result::operator=(const Result &r) {
  success = r.success;
  m = r.m;
  jacobian = r.jacobian;
  tel = r.tel;
  env = r.env;
  exp = r.exp;
  column_labels = r.column_labels;
  cont = r.cont;

  return *this;
}

Result::~Result() {
  clear();
}

void
Result::clear() {
  success = false;
  m.clear();
  jacobian.clear();
  tel.clear();
  env = "";
  exp = "";
  column_labels.clear();
  cont.clear();
}
