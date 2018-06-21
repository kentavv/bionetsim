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

#ifndef _PATHWAY_LOADER_H_
#define _PATHWAY_LOADER_H_

#include <qcstring.h>
#include <qlist.h>

#include "editor.h"
#include "fitness.h"
#include "pathway_matrix.h"
#include "simulation.h"

class PathwayCanvasItem;
class QWidget;

bool pathwayLoadBuffer(const QByteArray &buf,
		       PathwayMatrix &pem,
		       QWidget *parent=NULL);

bool pathwayLoad(const char *filename,
		 PathwayMatrix &pem,
                 QWidget *parent=NULL);


bool pathwayLoadBuffer(const QByteArray &buf,
		       PathwayMatrix &pem,
		       Fitness &fitness,
		       QWidget *parent=NULL);

bool pathwayLoad(const char *filename,
		 PathwayMatrix &pem,
		 Fitness &fitness,
                 QWidget *parent=NULL);


bool pathwayLoadBuffer(const QByteArray &buf,
		       QList<PathwayCanvasItem> &items,
		       QWidget *parent=NULL);

bool pathwayLoad(const char *filename,
		 QList<PathwayCanvasItem> &items,
		 QWidget *parent=NULL);


bool pathwayLoadBuffer(const QByteArray &buf,
		       Editor &editor,
		       Fitness &fitness,
		       Simulation &simulation,
		       QList<PathwayCanvasItem> &items,
		       QWidget *parent=NULL);

bool pathwayLoad(const char *filename,
		 Editor &editor,
		 Fitness &fitness,
		 Simulation &simulation,
		 QList<PathwayCanvasItem> &items,
		 QWidget *parent=NULL);


bool pathwayLoadBuffer(const QByteArray &buf,
		       Editor &editor,
		       Fitness &fitness,
		       Simulation &simulation,
		       QList<PathwayCanvasItem> &items,
		       int &version,
		       QWidget *parent=NULL);

bool pathwayLoad(const char *filename,
		 Editor &editor,
		 Fitness &fitness,
		 Simulation &simulation,
		 QList<PathwayCanvasItem> &items,
		 int &version,
		 QWidget *parent=NULL);

#endif
