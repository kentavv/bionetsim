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

#include <qcstring.h>
#include <qdom.h>
#include <qfile.h>
#include <qlist.h>
#include <qwidget.h>

#include "editor.h"
#include "fitness.h"
#include "log.h"
#include "pathway_load.h"
#include "pathway_load_v18.h"
#include "pathway_load_v19.h"
#include "pathway_load_v20.h"
#include "pathway_load_v21.h"
#include "pathway_load_v22.h"
#include "pathway_load_v23.h"
#include "pathway_load_v24.h"
#include "pathway_load_v25.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "simulation.h"
#include "version.h"

static bool
pathwayLoad(QDomDocument &doc, Editor &editor,
	    Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, int &version, QWidget *parent);

static QString
docFormat(QDomDocument &doc) {
  QDomNodeList nl = doc.elementsByTagName("Format");
  if(nl.count() != 1) return "";
  
  QDomNode n = nl.item(0);
  if(!n.isElement()) return "";
  
  QDomElement e = n.toElement();
  return e.text();
}

bool
pathwayLoadBuffer(const QByteArray &buf, PathwayMatrix &pem, QWidget *parent) {
  Fitness fitness;
  return pathwayLoadBuffer(buf, pem, fitness, parent);
}

bool
pathwayLoad(const char *filename, PathwayMatrix &pem, QWidget *parent) {
  Fitness fitness;
  return pathwayLoad(filename, pem, fitness, parent);
}

bool
pathwayLoadBuffer(const QByteArray &buf, PathwayMatrix &pem,
		  Fitness &fitness, QWidget *parent) {
  QList<PathwayCanvasItem> items;
  if(!pathwayLoadBuffer(buf, pem.editor, fitness, pem.simulation, items, parent)) {
    return false;
  }
  pem.setup(pem.editor, pem.simulation, items);
  return true;
}

bool
pathwayLoad(const char *filename, PathwayMatrix &pem, Fitness &fitness, QWidget *parent) {
  QList<PathwayCanvasItem> items;
  if(!pathwayLoad(filename, pem.editor, fitness, pem.simulation, items, parent)) {
    return false;
  }
  pem.setup(pem.editor, pem.simulation, items);
  return true;
}

bool
pathwayLoadBuffer(const QByteArray &buf,
		  QList<PathwayCanvasItem> &items, QWidget *parent) {
  int version;
  Editor editor;
  Fitness fitness;
  Simulation simulation;
  return pathwayLoadBuffer(buf, editor, fitness, simulation, items, version, parent);
}

bool
pathwayLoad(const char *filename,
	    QList<PathwayCanvasItem> &items, QWidget *parent) {
  int version;
  Editor editor;
  Fitness fitness;
  Simulation simulation;
  return pathwayLoad(filename, editor, fitness, simulation, items, version, parent);
}

bool
pathwayLoadBuffer(const QByteArray &buf, Editor &editor,
		  Fitness &fitness, Simulation &simulation,
		  QList<PathwayCanvasItem> &items, QWidget *parent) {
  int version;
  return pathwayLoadBuffer(buf, editor, fitness, simulation, items, version, parent);
}

bool
pathwayLoad(const char *filename, Editor &editor,
	    Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, QWidget *parent) {
  int version;
  return pathwayLoad(filename, editor, fitness, simulation, items, version, parent);
}

bool
pathwayLoadBuffer(const QByteArray &buf, Editor &editor,
		  Fitness &fitness, Simulation &simulation,
		  QList<PathwayCanvasItem> &items, int &version, QWidget *parent) {
  QDomDocument doc;
  if(!doc.setContent(buf)) {
    return false;
  }

  return pathwayLoad(doc, editor, fitness, simulation, items, version, parent);
}

bool
pathwayLoad(const char *filename, Editor &editor,
	    Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, int &version, QWidget *parent) {
  QFile file(filename);
  QDomDocument doc;
  if(!file.open(IO_ReadOnly)) return false;
  if(!doc.setContent(&file)) {
    file.close();
    return false;
  }
  file.close();

  return pathwayLoad(doc, editor, fitness, simulation, items, version, parent);
}


static bool
pathwayLoad(QDomDocument &doc, Editor &editor,
	    Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, int &version, QWidget *parent) {
  items.clear();
  fitness.clear();
  simulation.clear();
  
  QString format = docFormat(doc);

  if(format == "18") {
    if(!pathwayLoad_v18(doc, editor, fitness, simulation, items, version)) return false;
    pneDebug("Importing file with format 18...");
  } else if(format == "19") {
    if(!pathwayLoad_v19(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "20") {
    if(!pathwayLoad_v20(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "21") {
    if(!pathwayLoad_v21(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "22") {
    if(!pathwayLoad_v22(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "23") {
    if(!pathwayLoad_v23(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "24") {
    if(!pathwayLoad_v24(doc, editor, fitness, simulation, items, version)) return false;
  } else if(format == "25") {
    if(!pathwayLoad_v25(doc, editor, fitness, simulation, items, version)) return false;
  } else {
    return false;
  }

  PathwayCanvasItem *i;
  for(i=items.first(); i; i=items.next()) {
    i->setParent(parent);
  }

  return true;
}
