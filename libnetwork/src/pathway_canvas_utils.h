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

#ifndef _PATHWAY_CANVAS_UTILS_H_
#define _PATHWAY_CANVAS_UTILS_H_

#include "pathway_canvas_items.h"

QList<PathwayCanvasItem> copy(QList<PathwayCanvasItem> &, bool only_selected=false);
QStringList moleculeNames(const QList<PathwayCanvasItem> &);
void assignIds(QList<PathwayCanvasItem> &);

void setAllControlVariablesEnvironment(QList<PathwayCanvasItem> &, int ce, int ne=-1);

int numControlVariables(const QList<PathwayCanvasItem> &);
int numMolecules(const QList<PathwayCanvasItem> &);
int numReactions(const QList<PathwayCanvasItem> &);
int numSubnetworks(const QList<PathwayCanvasItem> &);

inline bool isMolecule(const PathwayCanvasItem *i) {
  return i->rtti() == MoleculeItem::RTTI;
}
inline bool isReaction(const PathwayCanvasItem *i) {
  return i->rtti() == ReactionItem::RTTI;
}
inline bool isReactionEdge(const PathwayCanvasItem *i) {
  return i->rtti() == ReactionEdgeItem::RTTI;
}
inline bool isSubnetworkEdge(const PathwayCanvasItem *i) {
  return i->rtti() == SubnetworkEdgeItem::RTTI;
}
inline bool isEdge(const PathwayCanvasItem *i) {
  return isReactionEdge(i) || isSubnetworkEdge(i);
}
inline bool isCompleteEdge(const PathwayCanvasItem *i) {
  return ((isReactionEdge(i) &&
	   ((ReactionEdgeItem*)i)->molecule() &&
	   ((ReactionEdgeItem*)i)->reaction()) ||
	  (isSubnetworkEdge(i) &&
	   ((SubnetworkEdgeItem*)i)->molecule() &&
	   ((SubnetworkEdgeItem*)i)->subnetwork()));
}
inline bool isNode(const PathwayCanvasItem *i) {
  return i->rtti() == InternalNodeItem::RTTI;
}
inline bool isAnnotation(const PathwayCanvasItem *i) {
  return i->rtti() == AnnotationItem::RTTI;
}
inline bool isControlVariable(const PathwayCanvasItem *i) {
  return isMolecule(i) && i->sub_rtti() == ControlVariableItem::sub_RTTI;
}
inline bool isMolRxn(const PathwayCanvasItem *i) {
  return isMolecule(i) || isReaction(i);
}
inline bool isSubnetwork(const PathwayCanvasItem *i) {
  return i->rtti() == SubnetworkItem::RTTI;
}
inline bool isMovable(const PathwayCanvasItem *i) {
  return isMolecule(i) || isReaction(i) || isNode(i) || isAnnotation(i) || isSubnetwork(i);
}

#endif
