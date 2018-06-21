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

#include <qstring.h>
#include <qstringlist.h>

#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"

void
buildReactionList(const QList<PathwayCanvasItem> &items,
		  const QString &path, int &nrxn,
		  QStringList &rxns, QStringList &rxn_comments,
		  QStringList &params) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *i;
  while((i = it.current())) {
    if(isMolecule(i)) {
      MoleculeItem *mol = (MoleculeItem*)i;
      if(mol->decays()) {
	QString str;
	str.sprintf("%s\t%d\t\t# %-60s\t%f\n",
		    (const char*)(path + mol->label()), nrxn,
		    (const char*)(path + mol->label() + " --> 0"),
		    mol->decayRate());
	rxns.append(str);

	str.sprintf("# %4d\t%-20s\t%-80s\t%f\n",
		    nrxn, (const char*)(path + mol->label()),
		    (const char*)(path + mol->label() + " --> 0"),
		    mol->decayRate());
	rxn_comments.append(str);

	str.sprintf("%d\t%f\t%f\t# %f\n", nrxn, 0.0, 1000000.0, mol->decayRate());
	params.append(str);

      	nrxn++;
      }
    } else if(isReaction(i)) {
      MassActionReactionItem *rxn = (MassActionReactionItem*)i;
      QString str;
      
      if(rxn->reversible()) {	
	str.sprintf("%s\t%d\t%d\t# %-60s\t%f\n",
		    (const char*)(path + rxn->label()), nrxn, nrxn+1,
		    (const char*)rxn->equation(), rxn->Kr());
	rxns.append(str);

      	str.sprintf("# %4d/%d\t%-20s\t%-80s\t%f\n",
		    nrxn, nrxn+1, (const char*)(path + rxn->label()),
		    (const char*)rxn->equation(), rxn->Kr());
	rxn_comments.append(str);

	str.sprintf("%d\t%f\t%f\t# %f\n", nrxn, 0.0, 1000000.0, rxn->Kf());
	params.append(str);
	str.sprintf("%d\t%f\t%f\t# %f\n", nrxn+1, 0.0, 1000000.0, rxn->Kr());
	params.append(str);

	nrxn+=2;
      } else {
	str.sprintf("%s\t%d\t\t# %-60s\t%f\n",
		    (const char*)(path + rxn->label()), nrxn,
		    (const char*)rxn->equation(), rxn->Kf());
	rxns.append(str);

	str.sprintf("# %4d\t%-20s\t%-80s\t%f\n",
		    nrxn, (const char*)(path + rxn->label()),
		    (const char*)rxn->equation(), rxn->Kf());
	rxn_comments.append(str);
	
	str.sprintf("%d\t%f\t%f\t# %f\n", nrxn, 0.0, 1000000.0, rxn->Kf());
	params.append(str);
	
	nrxn++;
      }
    } else if(isSubnetwork(i)) {
      SubnetworkItem *sn = (SubnetworkItem*)i;
      buildReactionList(sn->rawItems(), path + sn->label() + "::",
			nrxn, rxns, rxn_comments, params);
    }
    ++it;
  }
}

void
buildReactionList(const QList<PathwayCanvasItem> &items,
		  QStringList &rxns, QStringList &rxn_comments,
		  QStringList &params) {
  int nrxn = 0;
  buildReactionList(items, "", nrxn, rxns, rxn_comments, params);
}

int
main(int argc, char *argv[]) {
  bool sort = false;
		     
  if(argc != 2) {
    fprintf(stderr, "%s: <network file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  QList<PathwayCanvasItem> items;
  if(!pathwayLoad(filename, items)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  QStringList rxns;
  QStringList rxn_comments;
  QStringList params;
  
  buildReactionList(items, rxns, rxn_comments, params);
  
  QStringList::iterator it;

  printf("# Original model\n");
  for(it = rxn_comments.begin(); it != rxn_comments.end(); ++it) {
    printf("%s", (const char*)*it);
  }

  if(sort) {
    printf("# Warning: Output has been sorted and may not be in the same"
	   " order as in reaction list\n");
    rxns.sort();
  }

  printf("# Reactions\n");
  printf("# RxnName\tIndex into param table\tRxn Equation\tKinetic value\n");
  for(it = rxns.begin(); it != rxns.end(); ++it) {
    printf("%s", (const char*)*it);
  }

  printf("# Parameters\n");
  printf("# Param Index\tMin\tMax\t\t\t-or-\n");
  printf("# Param Index\tFixed Value\n");
  for(it = params.begin(); it != params.end(); ++it) {
    printf("%s", (const char*)*it);
  }
  
  return 0;
}
