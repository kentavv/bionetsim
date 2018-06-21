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

#include <iostream>

#include <stdlib.h>
#include <math.h>

#include "RP_AstNode.h"

using namespace std;

static int debugf = 0;
#ifdef STAND_ALONE
ostream &rp_log = cout;
#else
#include "log.h"
static ostream *prp_log = new ostream(new logbuf);
ostream &rp_log = *prp_log;
#endif

int
reactionParserDebug() {
	return debugf;
}

void
setReactionParserDebug(int v) {
	debugf = v;
}

ReactionNode::ReactionNode(int line, const string *rxn_name, const list<pair<int, string> > *left_side, int rxn_dir, const list<pair<int, string> > *right_side, const pair<double, double> *rxn_rate)
    : line_(line),
    rxn_name_(*rxn_name),
   rxn_dir_(rxn_dir)
 {
   if(left_side) left_side_ = *left_side;
   if(right_side) right_side_ = *right_side;

   if(rxn_rate) rxn_rate_ = *rxn_rate;

    if(debugf > 0) {
      rp_log << "Created ReactionNode on line " << line_ << '\n';
    }
 }
