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

#include <qdatetime.h>
#include <qdom.h>
#include <qfile.h>
#include <qlist.h>
#include <qwidget.h>

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_save.h"
#include "editor.h"
#include "fitness.h"
#include "simulation.h"
#include "version.h"

static void
saveInfo(QDomDocument &doc, QDomElement &n,
	 const char *appname, const char *appversion, int version) {
  QDomElement e;
  QDomText t;

  QDomElement info = doc.createElement("Info");
  n.appendChild(info);

  if(appname && appversion) {
    e = doc.createElement("Program");
    t = doc.createTextNode(QString(appname) + " " + QString(appversion));
    info.appendChild(e); e.appendChild(t);
  }
  
  e = doc.createElement("Format"); 
  t = doc.createTextNode(fileformat);
  info.appendChild(e); e.appendChild(t);
  
  e = doc.createElement("SaveDateTime");
  t = doc.createTextNode((const char*)QDateTime::currentDateTime().toString());
  info.appendChild(e); e.appendChild(t);
  
  e = doc.createElement("Version");
  t = doc.createTextNode(QObject::tr("%1").arg(version));
  info.appendChild(e); e.appendChild(t);
}

bool
pathwaySaveBuffer(QByteArray &buf,
		  Editor &editor, Fitness &fitness, Simulation &simulation,
		  QList<PathwayCanvasItem> &items, int version) {
  return pathwaySaveBuffer(buf, NULL, NULL, editor, fitness,
			   simulation, items, version);
}

bool
pathwaySave(const char *filename,
	    Editor &editor, Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, int version) {
  return pathwaySave(filename, NULL, NULL, editor, fitness, simulation, items, version);
}

bool
pathwaySaveBuffer(QByteArray &buf, const char *appname, const char *appversion,
		  Editor &editor, Fitness &fitness, Simulation &simulation,
		  QList<PathwayCanvasItem> &items, int version) {
  QDomDocument doc;

  QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" standalone=\"yes\"");
  doc.appendChild(pi);
  
  QDomElement root = doc.createElement("Network");
  doc.appendChild(root);

  saveInfo(doc, root, appname, appversion, version);

  editor.save(doc, root);
  fitness.save(doc, root);
  simulation.save(doc, root);
  pathwaySaveItems(doc, root, items);

  buf = doc.toCString();
  buf.truncate(buf.size()-1); // seem to need to remove the trailing \0 that CString adds
  
  return true;
}

bool
pathwaySave(const char *filename, const char *appname, const char *appversion,
	    Editor &editor, Fitness &fitness, Simulation &simulation,
	    QList<PathwayCanvasItem> &items, int version) {
  QFile file(filename);
  if(!file.open(IO_WriteOnly)) return false;

  QDomDocument doc;

  QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" standalone=\"yes\"");
  doc.appendChild(pi);
  
  QDomElement root = doc.createElement("Network");
  doc.appendChild(root);

  saveInfo(doc, root, appname, appversion, version);

  editor.save(doc, root);
  fitness.save(doc, root);
  simulation.save(doc, root);
  pathwaySaveItems(doc, root, items);
  
  QTextStream ts(&file);
  ts << doc.toString();
  
  return true;
}

void
pathwaySaveItems(QDomDocument &doc, QDomElement &n,
		 QList<PathwayCanvasItem> &items) {
  assignIds(items);
  
  PathwayCanvasItem *i;

  QDomElement mols = doc.createElement("Molecules");
  n.appendChild(mols);
  
  for(i=items.first(); i; i=items.next()) {
    if(isMolecule(i) || isAnnotation(i)) {
      i->save(doc, mols);
    }
  }

  QDomElement sns = doc.createElement("Subnetworks");
  n.appendChild(sns);
  
  for(i=items.first(); i; i=items.next()) {
    if(isSubnetwork(i)) {
      i->save(doc, sns);
    }
  }
  
  QDomElement rxns = doc.createElement("Reactions");
  n.appendChild(rxns);
  
  for(i=items.first(); i; i=items.next()) {
    if(isReaction(i)) {
      i->save(doc, rxns);
    }
  }  
}
