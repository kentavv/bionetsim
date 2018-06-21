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

#include "editor.h"
#include "pathway_save.h"

Editor::Editor()
  : gsx_(16),
    gsy_(16),
    use_grid_(true),
    display_grid_(false)
{}

void
Editor::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement info = doc.createElement("Editor");
  n.appendChild(info);

  e = doc.createElement("GridSpacing");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(gsx_).arg(gsy_));
  info.appendChild(e); e.appendChild(t);
  
  if(use_grid_) {
    e = doc.createElement("SnapToGrid");
    info.appendChild(e);
  }

  e = doc.createElement("Grid");
  e.setAttribute("display", display_grid_ ? "true" : "false");
  info.appendChild(e);
}

