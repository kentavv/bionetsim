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

#include <qlist.h>
#include <qobject.h>
#include <qstring.h>

#include "pathway_canvas_utils.h"

static QString
moleculeBindings(SubnetworkItem *sn) {
  QString str;
  QList<EdgeBaseItem> edges = sn->edges();
  SubnetworkEdgeItem *e;
  for(e=(SubnetworkEdgeItem*)edges.first(); e; ) {
    if(e->subnetworkMolecule() && e->molecule()) {
      str += e->subnetworkMolecule()->label();
      str += "="  + e->molecule()->label();
    }
    e=(SubnetworkEdgeItem*)edges.next();
    if(e) str += ", ";
  }
  return str;
}

static QString
indent(int ws) {
  QString str;
  for(int i=0; i<ws; i++) {
    str += " ";
  }
  return str;
}

void
buildMoleculeList(const QList<PathwayCanvasItem> &items, QString &text, int ws) {
  PathwayCanvasItem *i;
  QListIterator<PathwayCanvasItem> it(items);
  QStringList names;
  QStringList::Iterator nit;

  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isMolecule(i)) {
      QString str;
      
      MoleculeItem *p = (MoleculeItem*)i;
      str = indent(ws) + p->label();
      if(p->initialConcentration() != 0.0) {
	str += QObject::tr("  IC=%1").arg(p->initialConcentration());
      }
      if(p->decays()) {
	str += QObject::tr("  DR=%1").arg(p->decayRate());
      }
      if(isControlVariable(p)) {
	str += "  (Control Variable)";
      }
      if(!p->initialized()) {
	str += "  (Uninitialized)";
      }
      str += "\n";

      names.append(str);
    }
  }

  names.sort();
  for(nit = names.begin(); nit != names.end(); ++nit) {
    text += *nit;
  }

  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isSubnetwork(i)) {
      text += (indent(ws) + ((SubnetworkItem*)i)->label() +
	       "  (" + moleculeBindings((SubnetworkItem*)i) + "):\n");
      buildMoleculeList(((SubnetworkItem*)i)->items(), text, ws+2);
    }
  }
}


void
buildReactionList(const QList<PathwayCanvasItem> &items, QString &text, int ws) {
  PathwayCanvasItem *i;
  QListIterator<PathwayCanvasItem> it(items);
  QStringList names;
  QStringList::Iterator nit;

  int dnum = 0;
  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isReaction(i)) {
      QString str;

      ReactionItem *p = (ReactionItem*)i;

      QString eqn = p->equation();
      
      if(p->reversible()) {
	eqn.replace("<->", "-->");
      }
      
      str = indent(ws) + p->label() + "\t" + eqn;
      str += QObject::tr("\t%1").arg(((MassActionReactionItem*)p)->Kf());
      if(!p->initialized()) {
	str += "  (Uninitialized)";
      }
      str += "\n";
      names.append(str);

      if(p->reversible()) {
	eqn.replace("-->", "<--");
	str = indent(ws) + p->label() + "_r\t" + eqn;
	str += QObject::tr("\t%1").arg(((MassActionReactionItem*)p)->Kr());
	if(!p->initialized()) {
	  str += "  (Uninitialized)";
	}
	str += "\n";
	names.append(str);
      }
    } else if(isMolecule(i) && ((MoleculeItem*)i)->decays()) {
      QString str;
      
      MoleculeItem *p = (MoleculeItem*)i;
      str = indent(ws) + QObject::tr("D%1\t").arg(++dnum) + p->label() + QObject::tr(" --> 0\t%1\n").arg(p->decayRate());
      
      names.append(str);
    }
  }

  names.sort();
  for(nit = names.begin(); nit != names.end(); ++nit) {
    text += *nit;
  }

  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isSubnetwork(i)) {
      text += (indent(ws) + ((SubnetworkItem*)i)->label() +
	       "  (" + moleculeBindings((SubnetworkItem*)i) + "):\n");
      buildReactionList(((SubnetworkItem*)i)->items(), text, ws+2);
    }
  }
}

void
buildControlVariableList(const QList<PathwayCanvasItem> &items, const QString &path, QList<PathwayCanvasItem> &cvs, QStringList &labels) {
  PathwayCanvasItem *i;
  QListIterator<PathwayCanvasItem> it(items);

  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isControlVariable(i)) {
      cvs.append(i);
      labels.append(path + ((MoleculeItem*)i)->label());
    }
  }

  for(it.toFirst(); it.current(); ++it) {
    i = it.current();
    if(isSubnetwork(i)) {
      buildControlVariableList(((SubnetworkItem*)i)->items(),
			       path + ((SubnetworkItem*)i)->label() + "::",
			       cvs, labels);
    }
  }
}
