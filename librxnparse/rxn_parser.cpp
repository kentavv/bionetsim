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

#ifndef STAND_ALONE
#include <iostream>
#include <map>

#include "RP_AstNode.h"

#include "log.h"
#include "pathway_canvas_items.h"
#include "rxn_parser.h"

static ostream *prp_log = new ostream(new logbuf);
static ostream &rp_log = *prp_log;

extern int RPyyline;
extern char *yytext;
extern int RPparse();

static int num_err = 0;

using std::endl;
using std::map;

void
RPerror(char *s) {
	if(num_err < 100) {
		if(reactionParserDebug > 0) {
			rp_log << " line " << RPyyline << ": ";
		}
		if (s == NULL) {
			if(reactionParserDebug > 0) {
				rp_log << " line " << RPyyline << ": syntax error\n";
			}
			pneWarning("RXN PARSE: Syntax error detected near line %d.\n", RPyyline);
		} else {
			if(reactionParserDebug > 0) {
				rp_log << " line " << RPyyline << ": " << s << '\n';
			}
			pneWarning("RXN PARSE: Parse error (%s) detected near line %d.\n", s, RPyyline);
		}

		if(++num_err == 100) {
			if(reactionParserDebug > 0) {
				rp_log << "Too many errors\n";
			}
			pneWarning("RXN PARSE: No further errors will be reported.\n");
		}
	}
}

static char *RPinput;
static char *RPinputptr;
static int RPinputlen;

template<class T>
T RPmin(const T &a, const T &b) {
	return a <= b ? a : b; 
}

int
RPyyinput(char *buf, int max_size) {
	int n = RPmin(max_size, RPinputlen);
	if(n > 0) {
		memcpy(buf, RPinputptr, n);
		RPinputptr += n;
		RPinputlen -= n;
	}
	return n;
}

bool
reaction_parse(const QString &script, QList<PathwayCanvasItem> &items) {
	items.clear();
	num_err = 0;
	RPyyline = 1;
	{
		extern list<ReactionNode> RP_top;
		extern list<pair<string, double> > RP_top2;
		extern int RP_rxn_num;

		RP_top.clear();
		RP_top2.clear();
		RP_rxn_num = 0;

		void RPrestart  (FILE * input_file );
		RPrestart(NULL);
	}

	{
		string str = (const char*)script;
		RPinputlen = str.length();
		RPinput = new char[RPinputlen+1];
		RPinputptr = RPinput;
		strcpy(RPinput, str.c_str());
	}

	if(RPparse() == 0 && num_err == 0) {
		extern list<ReactionNode> RP_top;
		extern list<pair<string, double> > RP_top2;

		map<string, DNAMoleculeItem*> table;

		{
			list<pair<string, double> >::const_iterator i;
			for(i=RP_top2.begin(); i!=RP_top2.end(); i++) {
				DNAMoleculeItem *mol;
				map<string, DNAMoleculeItem*>::iterator k = table.find(i->first);
				if(k == table.end()) {
					mol = new DNAMoleculeItem(NULL);
					items.append(mol);
					mol->setInitialized(true);
					mol->setLabel(i->first.c_str());
					table[i->first] = mol;
				} else {
					mol = k->second;
				}
				mol->setInitialConcentration(i->second);
			}
		}

		list<ReactionNode>::const_iterator i;

		for(i=RP_top.begin(); i!=RP_top.end(); i++) {
			MassActionReactionItem *rxn = new MassActionReactionItem(NULL);
			items.append(rxn);
			rxn->setInitialized(true);
			rxn->setLabel(i->label().c_str());
			rxn->setKf(i->Kf());
			rxn->setKr(i->Kr());
			rxn->setReversible(i->reversible());

			list<pair<int, string> >::const_iterator j;
			for(j=i->leftSide().begin(); j!=i->leftSide().end(); j++) {
				DNAMoleculeItem *mol;
				map<string, DNAMoleculeItem*>::iterator k = table.find(j->second);
				if(k == table.end()) {
					mol = new DNAMoleculeItem(NULL);
					items.append(mol);
					mol->setInitialized(true);
					mol->setLabel(j->second.c_str());
					table[j->second] = mol;
				} else {
					mol = k->second;
				}
				ReactionEdgeItem *edge = new ReactionEdgeItem(NULL);
				items.append(edge);
				edge->setCoefficient(j->first);
				edge->addPoint(mol);
				edge->addPoint(rxn);
				//mol->addEdge(edge);
				//rxn->addEdge(edge);
			}

			for(j=i->rightSide().begin(); j!=i->rightSide().end(); j++) {
				DNAMoleculeItem *mol;
				map<string, DNAMoleculeItem*>::iterator k = table.find(j->second);
				if(k == table.end()) {
					mol = new DNAMoleculeItem(NULL);
					items.append(mol);
					mol->setInitialized(true);
					mol->setLabel(j->second.c_str());
					table[j->second] = mol;
				} else {
					mol = k->second;
				}
				ReactionEdgeItem *edge = new ReactionEdgeItem(NULL);
				items.append(edge);
				edge->setCoefficient(j->first);
				edge->addPoint(rxn);
				edge->addPoint(mol);
				//rxn->addEdge(edge);
				//mol->addEdge(edge);
			}
		}
	}

	if(num_err > 0) {
		if(reactionParserDebug > 0) {
			rp_log << num_err << " errors detected during execution\n";
		}
		pneWarning("RXN PARSE: Please correct errors in equations.\n");
		return false;
	}

	return true;
}
#endif
