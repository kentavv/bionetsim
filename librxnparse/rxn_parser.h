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

#ifndef _RXN_PARSER_H_
#define _RXN_PARSER_H_

#include <qstring.h>

#include "matrix.h"
#include "pathway_matrix.h"

int reactionParserDebug();
void setReactionParserDebug(int v);

bool reaction_parse(const QString &script, QList<PathwayCanvasItem> &items);

#endif
