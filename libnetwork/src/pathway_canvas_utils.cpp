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

#include <stdlib.h>

#include <qlist.h>

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"

QList<PathwayCanvasItem>
copy(QList<PathwayCanvasItem> &items, bool only_selected) {
  //puts("in copy");
  //printf("%d items\n", items.count());
  QList<PathwayCanvasItem> copied_items;
  //copied_items.setAutoDelete(true);
  
  assignIds(items);
  
  PathwayCanvasItem *i, *j;  
  for(i=items.first(); i; i=items.next()) {
    if(!only_selected || i->selected()) {
      if(isMolRxn(i) || isAnnotation(i) || isSubnetwork(i)) {
	copied_items.append(i->clone());
      }
    }
  }

  for(i=items.first(); i; i=items.next()) {
    //if(isSubnetworkEdge(i)) puts("subnetwork edge present in copy");
    if(!only_selected || i->selected()) {
      if(isCompleteEdge(i)) {
	EdgeBaseItem *e  = (EdgeBaseItem*)i;
	//if(isSubnetworkEdge(i)) puts("subnetwork edge being copied");
	if(!only_selected ||
	   (isReactionEdge(i) &&
	    ((ReactionEdgeItem*)e)->molecule()->selected() &&
	    ((ReactionEdgeItem*)e)->reaction()->selected()) ||
	   (isSubnetworkEdge(i) &&
	    ((SubnetworkEdgeItem*)e)->molecule()->selected() &&
	    ((SubnetworkEdgeItem*)e)->subnetwork()->selected())) {
	  EdgeBaseItem *ce = (EdgeBaseItem*)e->clone();
	  copied_items.append(ce);
	  QList<MolRxnBaseItem> nodes = e->nodes();
	  for(j=nodes.first(); j; j=nodes.next()) {
	    if(!only_selected || j->selected()) {
	      if(isNode(j)) {
		PathwayCanvasItem *in = j->clone();
		copied_items.append(in);
		ce->appendNode(in);
	      } else {
		PathwayCanvasItem *k;
		for(k=copied_items.first(); k; k=copied_items.next()) {
		  if(k->id() == j->id()) break;
		}
		if(!k) abort();
		if(!isSubnetwork(k)) {
		  ce->appendNode(k);
		} else {
		  if(isSubnetworkEdge(e)) {
		    //printf("e label: %s\n", (const char*)(((SubnetworkEdgeItem*)e)->subnetworkMolecule()->label()));
		    if(isSubnetwork(k)) {
		      ((SubnetworkEdgeItem*)ce)->appendNode((SubnetworkItem*)k,
							    ((SubnetworkEdgeItem*)e)->subnetworkMolecule()->label());
		    } else {
		      ce->appendNode(k);
		    }
		  } else {
		    abort();
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  return copied_items;
}

static void
moleculeNames(const QList<PathwayCanvasItem> &items, QStringList &labels, const QString &path) {
  QListIterator<PathwayCanvasItem> i(items);
  for(; i.current(); ++i) {
    if(isMolecule(i.current())) {
      labels.append(path + ((MoleculeItem*)i.current())->label());
    }
  }
  for(; i.current(); ++i) {
    if(isSubnetwork(i.current())) {
      SubnetworkItem *sn = (SubnetworkItem*)i.current();
      moleculeNames(sn->items(), labels, path + sn->label() + "::");
    }
  }
}


QStringList
moleculeNames(const QList<PathwayCanvasItem> &items) {
  QStringList labels;
  moleculeNames(items, labels, "");
  return labels;
}

void
assignIds(QList<PathwayCanvasItem> &items) {
  int n=0;
  for(PathwayCanvasItem *i=items.first(); i; i=items.next()) {
    i->setId(++n);
  }
}

void
setAllControlVariablesEnvironment(QList<PathwayCanvasItem> &items, int ce, int ne) {
  PathwayCanvasItem *item;
  for(item=items.first(); item; item=items.next()) {
    if(isControlVariable(item)) {
      if(ne != -1) {
	((ControlVariableItem*)item)->setNumEnvironment(ne);
      }
      ((ControlVariableItem*)item)->setCurEnvironment(ce);
    }
  }
}

int
numControlVariables(const QList<PathwayCanvasItem> &items) {
  int n=0;
  QListIterator<PathwayCanvasItem> i(items);
  for(; i.current(); ++i) {
    if(isControlVariable(i)) {
      n++;
    }
  }
  return n;
}

int
numMolecules(const QList<PathwayCanvasItem> &items) {
  int n=0;
  QListIterator<PathwayCanvasItem> i(items);
  for(; i.current(); ++i) {
    if(isMolecule(i) && !isControlVariable(i)) {
      n++;
    }
  }
  return n;
}

int
numReactions(const QList<PathwayCanvasItem> &items) {
  int n=0;
  QListIterator<PathwayCanvasItem> i(items);
  for(; i.current(); ++i) {
    if(isReaction(i)) {
      n++;
    }
  }
  return n;
}

int
numSubnetworks(const QList<PathwayCanvasItem> &items) {
  int n=0;
  QListIterator<PathwayCanvasItem> i(items);
  for(; i.current(); ++i) {
    if(isSubnetwork(i)) {
      n++;
    }
  }
  return n;
}
