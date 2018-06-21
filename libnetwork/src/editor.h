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

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <qdom.h>

class Editor {
public:
  Editor();

  void save(QDomDocument &doc, QDomElement &n) const;

  int gsx_;
  int gsy_;
  bool use_grid_;
  bool display_grid_;
  bool auto_sim_;
};

#endif
