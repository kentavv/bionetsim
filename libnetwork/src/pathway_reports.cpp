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
#include <qregexp.h>
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
      str = indent(ws) + p->label() + ":  " + p->equation();
      str += QObject::tr("  Kf=%1").arg(((MassActionReactionItem*)p)->Kf());
      if(p->reversible()) {
	str += QObject::tr("  Kr=%1").arg(((MassActionReactionItem*)p)->Kr());
      }
      if(!p->initialized()) {
	str += "  (Uninitialized)";
      }
      str += "\n";

      names.append(str);
    } else if(isMolecule(i) && ((MoleculeItem*)i)->decays()) {
      QString str;
      
      MoleculeItem *p = (MoleculeItem*)i;
      str = indent(ws) + QObject::tr("D%1:  ").arg(++dnum) + p->label() + QObject::tr(" --> 0  Kf=%1\n").arg(p->decayRate());
      
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

void
buildMoleculeEquations(const QList<PathwayCanvasItem> &items, QString &text, int ws) {
  QListIterator<PathwayCanvasItem> it(items), it2(items);
  QStringList eqns;
  QStringList::Iterator nit;
  int nd=0;
  int ndt=0;
  int n=0;
  
  for(it.toFirst(); it.current(); ++it) {
    if(isMolecule(it.current())) {
      MoleculeItem *mol = (MoleculeItem*)it.current();
      if(mol->decays()) {
	text += indent(ws) + QString("k%1 = %2\n").arg(n+1).arg(mol->decayRate());
	n++;
      }
    }
  }
  ndt = n;
  
  for(it.toFirst(); it.current(); ++it) {
    if(isReaction(it.current())) {
      MassActionReactionItem *rxn = (MassActionReactionItem*)it.current();
      text += indent(ws) + QString("k%1 = %2\n").arg(n+1).arg(rxn->Kf());
      if(rxn->reversible()) {
	text += indent(ws) + QString("k%1r = %2\n").arg(n+1).arg(rxn->Kr());
      }
      n++;
    }
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isMolecule(it.current())) {  // For each molecule
      MoleculeItem *mol = (MoleculeItem*)it.current();
      QString eqn = indent(ws) + mol->label() + " = ";
      n=ndt;
      for(it2.toFirst(); it2.current(); ++it2) {
	if(isReaction(it2.current())) { // For each reaction	  
	  ReactionItem *rxn = (ReactionItem*)it2.current();
	  QString lhs, rhs;
	  rxn->equation(lhs, rhs, false);
	  QList<EdgeBaseItem> edges = rxn->edges();
	  EdgeBaseItem *e;
	  for(e=edges.first(); e; e=edges.next()) { 
	    if(isReactionEdge(e)) {   // For each edge
	      ReactionEdgeItem *re = (ReactionEdgeItem*)e;
	      if(re->molecule() == mol) { // Is this molecule on this edge?
		QString coeff = "";
		if(re->coefficient() > 1) {
		  coeff = QString("%1*").arg(re->coefficient());
		}
		if(re->moleculeIsProduct()) {
		  eqn += "+" + coeff + QString("k%1*").arg(n+1) + lhs;
		  if(rxn->reversible()) {
		    eqn += "-" + coeff + QString("k%1r*").arg(n+1) + rhs;
		  }
		} else if(re->moleculeIsReactant()) {
		  eqn += "-" + coeff + QString("k%1*").arg(n+1) + lhs;
		  if(rxn->reversible()) {
		    eqn += "+" + coeff + QString("k%1r*").arg(n+1) + rhs;
		  }
		}
	      }
	    }
	  }
	  n++;
	}
      }
      if(mol->decays()) {
	eqn += QString("-k%1*[%2]").arg(nd+1).arg(mol->label());
	nd++;
      }
      eqns.append(eqn);
    }
  }

  eqns.sort();
  for(nit = eqns.begin(); nit != eqns.end(); ++nit) {
    text += *nit + "\n";
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isSubnetwork(it.current())) {
      SubnetworkItem *sn = (SubnetworkItem*)it.current();
      text += indent(ws) + sn->label() + "  (" + moleculeBindings(sn) + "):\n";
      buildMoleculeEquations(sn->items(), text, ws+2);
    }
  }
}

void
buildMoleculeReducedEquationsCVODE(const QList<PathwayCanvasItem> &items, QString &text, int ws) {
  QListIterator<PathwayCanvasItem> it(items), it2(items);
  QStringList eqns;
  QStringList::Iterator nit;
  int nd=0;
  int ndt=0;
  int n=0;

  QString body;
  
  QString mol_labels, const_labels;
  for(it.toFirst(); it.current(); ++it) {
    if(isMolecule(it.current()) && !isControlVariable(it.current())) {
      MoleculeItem *mol = (MoleculeItem*)it.current();
      if(mol->label().right(2) == "gX") continue;
      QString str = mol->label();
      str.replace(' ', '_');
      if(!mol_labels.isEmpty()) {
	mol_labels += ", ";
      }
      mol_labels += str;
    } else if(isControlVariable(it.current())) {
      QString str = ((ControlVariableItem*)it.current())->label();
      str.replace(' ', '_');
      if(!const_labels.isEmpty()) {
	const_labels += ", ";
      }
      const_labels += str;
    }
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isControlVariable(it.current())) {
      ControlVariableItem *cv = (ControlVariableItem*)it.current();
      QString str = cv->label();
      str.replace(' ', '_');
      body += indent(ws) + QString("      ") + str + " = 1\n";
    }
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isMolecule(it.current())) {
      MoleculeItem *mol = (MoleculeItem*)it.current();
      if(mol->label().right(2) == "gX") continue; // Should not happen - genes do not degrade
      if(mol->decays()) {
	body += indent(ws) + QString("      k%1 = %2\n").arg(n+1).arg(mol->decayRate());
	if(!const_labels.isEmpty()) {
	  const_labels += ", ";
	}
	const_labels += QString("k%1").arg(n+1);
	n++;
      }
    }
  }
  ndt = n;
  
  for(it.toFirst(); it.current(); ++it) {
    if(isReaction(it.current())) {
      MassActionReactionItem *rxn = (MassActionReactionItem*)it.current();
      body += indent(ws) + QString("      k%1 = %2\n").arg(n+1).arg(rxn->Kf());
      if(!const_labels.isEmpty()) {
	const_labels += ", ";
      }
      const_labels += QString("k%1").arg(n+1);
      if(rxn->reversible()) {
	body += indent(ws) + QString("      k%1r = %2\n").arg(n+1).arg(rxn->Kr());
	const_labels += QString(", k%1r").arg(n+1);
      }
      n++;
    }
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isControlVariable(it.current())) continue;
    if(isMolecule(it.current())) {  // For each molecule
      MoleculeItem *mol = (MoleculeItem*)it.current();
      if(mol->label().right(2) == "gX") continue;
      QString eqn = indent(ws) + mol->label() + " = ";
      n=ndt;
      for(it2.toFirst(); it2.current(); ++it2) {
	if(isReaction(it2.current())) { // For each reaction	  
	  ReactionItem *rxn = (ReactionItem*)it2.current();
	  QString lhs, rhs;
	  rxn->equation(lhs, rhs, false);
	  QList<EdgeBaseItem> edges = rxn->edges();
	  EdgeBaseItem *e;
	  for(e=edges.first(); e; e=edges.next()) { 
	    if(isReactionEdge(e)) {   // For each edge
	      ReactionEdgeItem *re = (ReactionEdgeItem*)e;
	      if(re->molecule() == mol) { // Is this molecule on this edge?
		QString coeff = "";
		if(re->coefficient() > 1) {
		  coeff = QString("%1*").arg(re->coefficient());
		}
		if(re->moleculeIsProduct()) {
		  eqn += "+" + coeff + QString("k%1*").arg(n+1) + lhs;
		  if(rxn->reversible()) {
		    eqn += "-" + coeff + QString("k%1r*").arg(n+1) + rhs;
		  }
		} else if(re->moleculeIsReactant()) {
		  eqn += "-" + coeff + QString("k%1*").arg(n+1) + lhs;
		  if(rxn->reversible()) {
		    eqn += "+" + coeff + QString("k%1r*").arg(n+1) + rhs;
		  }
		}
	      }
	    }
	  }
	  n++;
	}
      }
      if(mol->decays()) {
	eqn += QString("-k%1*[%2]").arg(nd+1).arg(mol->label());
	nd++;
      }
      eqns.append(eqn);
    }
  }

  int num = 1;
  //eqns.sort();
  for(nit = eqns.begin(); nit != eqns.end(); ++nit) {
    QString label = (*nit).section(" = ", 0, 0);
    QString eqn = (*nit).section(" = ", 1, 1);
    //body += QString("           (%1) = ").arg(num) + eqn + "\n";
    eqn.replace('[', '(');
    eqn.replace(']', ')');
    eqn.replace(' ', '_');
    eqn.replace("^", "**");
    eqn.replace(QRegExp("\\(([^()]+)gX\\)"), "(1-\\1g)");
    eqn.replace(QRegExp("^\\+"), "");
    body += "    ! " + label + "\n";
    body += QString("      C_TMP(%1) = ").arg(num) + eqn + "\n";
    num++;
  }

  for(it.toFirst(); it.current(); ++it) {
    if(isSubnetwork(it.current())) {
      SubnetworkItem *sn = (SubnetworkItem*)it.current();
      body += indent(ws) + sn->label() + "  (" + moleculeBindings(sn) + "):\n";
      buildMoleculeReducedEquationsCVODE(sn->items(), body, ws+2);
    }
  }

  QString header;
  header += "   SUBROUTINE SET_RESIDUALS( " + mol_labels + " )\n" +
            "     IMPLICIT NONE\n" +
            "     TYPE(CDLVAR) :: " + mol_labels + "\n" +
            "     DOUBLE PRECISION :: " + const_labels + "\n";
  QString trailer;
  
  QString tmp = header + body + trailer;;
  text += tmp;
}

#include <vector>
#include "editor.h"
#include "fitness.h"
#include "simulation.h"
#include "pathway_matrix.h"
using namespace std;
void
buildMoleculeReducedEquationsMathematica(const QList<PathwayCanvasItem> &items, QString &text, int ws) {
  Editor editor;
  Fitness fitness;
  Simulation simulation;
  PathwayMatrix pm;
  pm.setup(editor, pm.simulation, items);

  QStringList mols = pm.moleculeNames();
  IMatrix isto = pm.isto;

  //pm.print();

  QString nsolve_text, groebner_text;
  
  //int i;
  const int nm = pm.re.nrows();
  const int nr = pm.re.ncolumns();
  //const int nc = pm.cvs.count();
  //const int ne = nm - nc;
  
  vector<QString> mol_mass_def(nm);
  vector<QString> mol_names(nm);
  vector<QString> mol_mass_conv(nm);
  QRegExp rx1("^G(\\d+)");
  QRegExp rx2("^P(\\d+)");
  QRegExp rx3("^PG(\\d+) .*\\(G(\\d+)\\)");
  QRegExp rx4("^PC(\\d+)");
  QRegExp rx5("^PP(\\d+)");
  for(int m=0; m<nm; m++) {
    // Mathematica uses 'E' for the mathematical constant E so avoid using that identifier
    if(rx1.search(mols[m]) != -1) {
      char tmp = 'a' + rx1.cap(1).toInt()-1;
      if(tmp >= 'e') tmp++;
      mol_names[m] = tmp;
      mol_mass_def[m] = tmp;
    } else if(rx2.search(mols[m]) != -1) {
      char tmp = 'A' + rx2.cap(1).toInt()-1;
      if(tmp >= 'E') tmp++;
      mol_names[m] = tmp;
    } else if(rx3.search(mols[m]) != -1) {
      char tmp = 'a' + rx3.cap(2).toInt()-1;
      if(tmp >= 'e') tmp++;
      mol_names[m] = QString(QChar(tmp)) + "pg" + rx3.cap(1);
      {
	int ind = -1;
	for(int i=0; i<nm; i++) {
	  if(mol_names[i] == QString(QChar(tmp))) {
	    ind = i;
	    break;
	  }
	}
	if(ind == -1) abort();
	if(mol_mass_conv[ind].isEmpty()) {
	  mol_mass_def[ind] = "";
	  mol_mass_conv[ind] = tmp;
	}
	mol_mass_conv[ind] += "+" + mol_names[m];
      }
    } else if(rx4.search(mols[m]) != -1) {
      mol_names[m] = QString("pc" + rx4.cap(1));
    } else if(rx5.search(mols[m]) != -1) {
      mol_names[m] = QString("pp" + rx5.cap(1));
    } else {
      mol_names[m] = "JUNK(" + mols[m] + ")";
    }
  }

  for(int r=0; r<nr; r++) {
    QString nm = QString::number(pm.kc(r, 0), 'f', 12);
    nm.replace(QRegExp("\\.?0+$"), "");
    text += "k" + QString::number(r+1) + " = " + nm + ";\n";
  }

  for(int m=0; m<nm; m++) {
    if(!mol_mass_conv[m].isEmpty()) {
      nsolve_text += mol_mass_conv[m] + "==1, ";
      groebner_text += mol_mass_conv[m] + "-1, ";
    }
  }

  for(int m=0; m<nm; m++) {
    if(!mol_mass_def[m].isEmpty()) {
      text += mol_mass_def[m] + " = 1;\n";
    }
  }
    
  for(int m=0; m<nm; m++) {
    QString eqn;

    for(int r=0; r<nr; r++) {
      if(pm.re(m, r) != 0 && pm.re(m, r) > pm.pro(m, r)) {
	eqn += "-";
	if(pm.re(m, r) - pm.pro(m, r) > 1) {
	  eqn += QString::number(pm.re(m, r) - pm.pro(m, r));
	}
	eqn += "k" + QString::number(r+1);
	for(int m2=0; m2<nm; m2++) {
	  int n = pm.re(m2, r);
	  if(n != 0) {
	    eqn += "*" + mol_names[m2];
	    if(n != 1) {
	      eqn += "^" + QString::number(n);
	    }
	  }
	}
      }
    }
    
    for(int r=0; r<nr; r++) {
      if(pm.pro(m, r) != 0 && pm.re(m, r) < pm.pro(m, r)) {
	if(!eqn.isEmpty()) {
	  eqn += "+";
	}
	if(pm.pro(m, r) - pm.re(m, r) > 1) {
	  eqn += QString::number(pm.pro(m, r) - pm.re(m, r));
	}
	eqn += "k" + QString::number(r+1);
	for(int m2=0; m2<nm; m2++) {
	  int n = pm.re(m2, r);
	  if(n != 0) {
	    eqn += "*" + mol_names[m2];
	    if(n != 1) {
	      eqn += "^" + QString::number(n);
	    }
	  }
	}
      }
    }

    if(eqn.isEmpty()) {
      text += "(* p" + mol_names[m] + " = 0; *)\n";
    } else {
      text += "p" + mol_names[m] + " = " + eqn + ";\n";

      nsolve_text += "p" + mol_names[m] + " == 0";
      if(m < nm-1) {
	nsolve_text += ", ";
      }
      
      groebner_text += "p" + mol_names[m];
      if(m < nm-1) {
	groebner_text += ", ";
      }
    }
  }
  //text += "Chop[TableForm[N[NSolve[{" + nsolve_text + "},\n{";
  //text += "Chop[N[NSolve[{" + nsolve_text + "},\n{";
  text += "Chop[NSolve[{" + nsolve_text + "},\n{";
  bool first = true;
  for(int m=0; m<nm; m++) {
    if(mol_mass_def[m].isEmpty()) {
      if(!first) {
	text += ", ";
      }
      text += mol_names[m];
      first = false;
    }
  }
  //text += "}], 8]]]\n";
  //text += "}], 8]]\n";
  text += "}]]\n";

  text += "(* GroebnerBasis[{" + groebner_text + "},\n{";
  first = true;
  for(int m=0; m<nm; m++) {
    if(mol_mass_def[m].isEmpty()) {
      if(!first) {
	text += ", ";
      }
      text += mol_names[m];
      first = false;
    }
  }
  text += "}] *)\n";
}
