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

#include "pathway_canvas_utils.h"
#include "pathway_matrix.h"

PathwayMatrix::PathwayMatrix() 
  : progress(NULL)
{}


PathwayMatrix::PathwayMatrix(const PathwayMatrix &pm)
  : editor(pm.editor),
    simulation(pm.simulation),
    kc(pm.kc),
    pro(pm.pro),
    re(pm.re),
    ipro(pm.ipro),
    ire(pm.ire),
    isto(pm.isto),
    jmat(pm.jmat),
    items(pm.items),
    cvs(pm.cvs),
    y0(pm.y0),
    atol(pm.atol),
    rxn_names(pm.rxn_names),
    rxn_equations(pm.rxn_equations),
    progress(pm.progress),
    quit(pm.quit),
    plotted_(pm.plotted_),
    mol_names_(pm.mol_names_),
    plotted_mol_names_(pm.plotted_mol_names_)
{}

PathwayMatrix &
PathwayMatrix::operator=(const PathwayMatrix &pm) {
  editor = pm.editor;
  simulation = pm.simulation;
  kc = pm.kc;
  pro = pm.pro;
  re = pm.re;
  ipro = pm.ipro;
  ire = pm.ire;
  isto = pm.isto;
  jmat = pm.jmat;
  items = pm.items;
  cvs = pm.cvs;
  y0 = pm.y0;
  atol = pm.atol;
  rxn_names = pm.rxn_names;
  rxn_equations = pm.rxn_equations;
  progress = pm.progress;
  quit = pm.quit;
  plotted_ = pm.plotted_;
  mol_names_ = pm.mol_names_;
  plotted_mol_names_ = pm.plotted_mol_names_;

  return *this;
}

PathwayMatrix::~PathwayMatrix() {
}

void
PathwayMatrix::initializeMoleculeIds(const QList<PathwayCanvasItem> &items) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *p;
  while((p = it.current())) {
    if(isMolecule(p)) {
      p->setId(-1);
    } else if(isSubnetwork(p)) {
      initializeMoleculeIds(((SubnetworkItem*)p)->rawItems());
    }
    ++it;
  }
}

int
PathwayMatrix::countControlVariables(const QList<PathwayCanvasItem> &items) {
  int n = 0;
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *item;
  while((item = it.current())) {
    if(isControlVariable(item)) n++;
    else if(isSubnetwork(item)) n += countControlVariables(((SubnetworkItem*)item)->rawItems());
    ++it;
  }
  return n;
}

void
PathwayMatrix::buildControlVariableArray(const QList<PathwayCanvasItem> &items,
					 QArray<ControlVariableItem*> &cvs,
					 QList<MoleculeItem> &mol,
					 QStringList &mol_names) {
  int ncvs = countControlVariables(items);
  cvs.resize(ncvs);
  int i = mol.count();
  buildControlVariableArray(items, "", i, cvs, mol, mol_names);
}

void
PathwayMatrix::buildControlVariableArray(const QList<PathwayCanvasItem> &items,
					 const QString &path,
					 int &i,
					 QArray<ControlVariableItem*> &cvs,
					 QList<MoleculeItem> &mol,
					 QStringList &mol_names) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *p;
  while((p = it.current())) {
    if(isControlVariable(p)) {
      cvs[i] = (ControlVariableItem*)p;
      mol.append((MoleculeItem*)p);
      mol_names.append(path + ((MoleculeItem*)p)->label());
      //printf("Assigning id %d to CV %s\n", i, (const char*)(path + ((MoleculeItem*)p)->label()));
      p->setId(i++);
    } else if(isSubnetwork(p)) {
      buildControlVariableArray(((SubnetworkItem*)p)->rawItems(),
				path + ((SubnetworkItem*)p)->label() + "::",
				i, cvs, mol, mol_names);
    }
    ++it;
  }
}

void
PathwayMatrix::buildMoleculeList(const QList<PathwayCanvasItem> &items,
				 QList<MoleculeItem> &mol,
				 QStringList &mol_names) {
  int i = mol.count();
  buildMoleculeList(items, "", i, mol, mol_names);
}

void
PathwayMatrix::buildMoleculeList(const QList<PathwayCanvasItem> &items,
				 const QString &path,
				 int &i,
				 QList<MoleculeItem> &mol,
				 QStringList &mol_names) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *p;
  while((p = it.current())) {
    if(isMolecule(p)) {
      //puts("yes");
      if(!isControlVariable(p) && p->id() == -1) {
	mol.append((MoleculeItem*)p);
	mol_names.append(path + ((MoleculeItem*)p)->label());
	//printf("Assigning id %d to Mol %s\n", i, (const char*)(path + ((MoleculeItem*)p)->label()));
	p->setId(i++);
      }
    }
    ++it;
  }
  
  it.toFirst();
  while((p = it.current())) {
    if(isSubnetwork(p)) {
      SubnetworkItem *sn = (SubnetworkItem*)p;
      QList<EdgeBaseItem> edges = sn->edges();
      for(SubnetworkEdgeItem *sne = (SubnetworkEdgeItem*)edges.first();
	  sne;
	  sne = (SubnetworkEdgeItem*)edges.next()) {
	if(isCompleteEdge(sne)) {
	  sne->subnetworkMolecule()->setId(sne->molecule()->id());
#if 0
	  printf("Assigning id %d to sn Mol %s (attaches to %s)\n",
		 sne->molecule()->id(),
		 (const char*)(path + sn->label() + "::" + sne->subnetworkMolecule()->label()),
		 (const char*)sne->molecule()->label());
#endif
	}
      }
      //puts("recurse");
      buildMoleculeList(sn->rawItems(), path + sn->label() + "::",
			i, mol, mol_names);
    }
    ++it;
  }
  //puts("leave");
}

void
PathwayMatrix::buildReactionList(const QList<PathwayCanvasItem> &items,
				 QList<ReactionItem> &rxn,
				 QStringList &rxn_names,
				 QStringList &rxn_equations) {
  int i = rxn.count();
  buildReactionList(items, "", i, rxn, rxn_names, rxn_equations);
}

void
PathwayMatrix::buildReactionList(const QList<PathwayCanvasItem> &items,
				 const QString &path,
				 int &i,
				 QList<ReactionItem> &rxn,
				 QStringList &rxn_names,
				 QStringList &rxn_equations) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *p;
  while((p = it.current())) {
    if(isMolecule(p)) {
      if(((MoleculeItem*)p)->decays()) {
	rxn_names.append(path + ((MoleculeItem*)p)->label());
	rxn_equations.append(((MoleculeItem*)p)->label() + " --> 0");
      }
    } else if(isReaction(p)) {
      rxn.append((ReactionItem*)p);
      rxn_names.append(path + ((ReactionItem*)p)->label());
      rxn_equations.append(((ReactionItem*)p)->equation());
      p->setId(i++);
      if(((ReactionItem*)p)->reversible()) {
	rxn_names.append(path + ((ReactionItem*)p)->label() + " (Kr)");
	rxn_equations.append(((ReactionItem*)p)->equation() + " (Kr)");
      }
    } else if(isSubnetwork(p)) {
      buildReactionList(((SubnetworkItem*)p)->rawItems(),
			path + ((SubnetworkItem*)p)->label() + "::",
			i, rxn, rxn_names, rxn_equations);
    }
    ++it;
  }
}

void
PathwayMatrix::setup(const Editor &editor2, const Simulation &simulation2,
		     const QList<PathwayCanvasItem> &items2, QProgressBar *progress2) {
  PathwayCanvasItem *p, *q;
  int i, j;
  QList<MoleculeItem> mol;
  QList<ReactionItem> rxn;
  int nmol = 0, nrxn = 0;

  editor = editor2;
  simulation = simulation2;
  items = items2;
  progress = progress2;
  quit = false;

  mol_names_.clear();
  plotted_mol_names_.clear();
  rxn_names.clear();
  rxn_equations.clear();

  initializeMoleculeIds(items);

  buildControlVariableArray(items, cvs, mol, mol_names_);

  // Find non-control variables in a seperate stage to insure the
  // control variables are listed first.
  buildMoleculeList(items, mol, mol_names_);
  
  buildReactionList(items, rxn, rxn_names, rxn_equations);

  nmol = mol.count();
  nrxn = rxn_equations.count();
  //printf("nmol:%d nrxn:%d\n", nmol, nrxn);
  
  kc      = Matrix(nrxn, 1, 0.0);
  pro     = IMatrix(nmol, nrxn, 0);
  re      = IMatrix(nmol, nrxn, 0);
  y0      = Matrix(nmol, 1, 0.0);
  atol    = Matrix(nmol, 1, simulation.absoluteTolerance());

  setupPlottedMoleculesList(mol);

  for(i=0, j=0, p=mol.first(); p; p=mol.next(), j++) {
    MoleculeItem *m = (MoleculeItem*)p;
    if(m->decays()) {
      re(p->id(), i) += 1;
      kc(i, 0) = m->decayRate();
      i++;
    }
  }

  for(p=rxn.first(); p; p=rxn.next()) {
    QList<EdgeBaseItem> edges = ((ReactionItem*)p)->edges();
    for(q=edges.first(); q; q=edges.next()) {
      if(isReactionEdge(q)) {
	ReactionEdgeItem *q2 = (ReactionEdgeItem*)q;
	MoleculeItem *r = q2->molecule();
	if(r && q2->moleculeIsProduct()) {
	  pro(r->id(), i) += q2->coefficient();
	} else if(r && q2->moleculeIsReactant()) {
	  re(r->id(), i) += q2->coefficient();
	}
      }
    }
    
    kc(i, 0) = ((MassActionReactionItem*)p)->Kf();

    i++;

    if(((ReactionItem*)p)->reversible()) {
      QList<EdgeBaseItem> edges = ((MoleculeItem*)p)->edges();
      for(q=edges.first(); q; q=edges.next()) {
	if(isReactionEdge(q)) {
	  ReactionEdgeItem *q2 = (ReactionEdgeItem*)q;
	  MoleculeItem *r = q2->molecule();
	  if(r && q2->moleculeIsProduct()) {
	    re(r->id(), i) += q2->coefficient();
	  } else if(r && q2->moleculeIsReactant()) {
	    pro(r->id(), i) += q2->coefficient();
	  }
	}
      }
      
      kc(i, 0) = ((MassActionReactionItem*)p)->Kr();
      
      i++;
    }
  }

  setupSparseMatrices();
  
  for(i=0, p=mol.first(); p; p=mol.next()) {
    y0(i++, 0) = ((MoleculeItem*)p)->initialConcentration();
    //printf("%s %f\n", (const char*)((MoleculeItem*)p)->label(), ((MoleculeItem*)p)->initialConcentration());
  }

  setCurEnvironment(simulation.curEnvironment());
}

void
PathwayMatrix::setupPlottedMoleculesList(QList<MoleculeItem> &mol) {
  PathwayCanvasItem *p;
  int nmol = mol.count();
  int i, j;
  
  plotted_.resize(nmol, 1);
  plotted_.fill(false);

  for(i=0, j=0, p=mol.first(); p; p=mol.next(), j++) {
    MoleculeItem *m = (MoleculeItem*)p;
    plotted_(j, 0) = m->plotted();
    if(plotted_(j, 0)) {
      plotted_mol_names_.append(m->label());
    }
  }
}


void
PathwayMatrix::setupSparseMatrices() {
  int i, j;
  int nmol = pro.nrows(), nrxn = pro.ncolumns(), ncvs = cvs.count();

  ipro = IMatrix(nmol+1, nrxn, -1);
  ire  = IMatrix(nmol+1, nrxn, -1);
  isto = IMatrix(nmol+1, nrxn, -1);
  
  for(int r=0; r<nrxn; r++) {
    int j1, j2, j3;
    for(i=0, j1=0, j2=0, j3=0; i<nmol; i++) {
      if(re(i, r) != 0) {
	ire(++j1, r) = i;
      }
      if(pro(i, r) != 0) {
	ipro(++j2, r) = i;
      }
      if(re(i, r) != pro(i, r) && i >= ncvs) {
	isto(++j3, r) = i;
      }
    }
    ire(0, r)  = j1;
    ipro(0, r) = j2;
    isto(0, r) = j3;
  }

  int nj = 0, k;
  for(i=ncvs; i<nmol; i++) {
    for(j=ncvs; j<nmol; j++) {
      for(k=0; k<nrxn; k++) {
	if(pro(i, k) != re(i, k) && re(j, k) != 0.0) {
	  nj++;
	}
      }
    }
  }

  jmat.resize(nj, 3);


  int m = 0;
  for(i=ncvs; i<nmol; i++) {
    for(j=ncvs; j<nmol; j++) {
      for(k=0; k<nrxn; k++) {
	if(pro(i, k) != re(i, k) && re(j, k) != 0.0) {
	  jmat(m, 0) = i;
	  jmat(m, 1) = j;
	  jmat(m, 2) = k;
	  m++;
	}
      }
    }
  }
}

void
PathwayMatrix::setCurEnvironment(int env) {
  simulation.setCurEnvironment(env);
  for(int i=0; i<(int)cvs.count(); i++) {
    cvs[i]->setCurEnvironment(env);
  }
}

int
PathwayMatrix::curEnvironment() const {
  return simulation.curEnvironment();
}

void
PathwayMatrix::print(FILE *fp) const {
  int i, ncvs = cvs.count();
  QStringList::ConstIterator it;
  fprintf(fp, "Molecules:\n");
  for(i=0, it = mol_names_.begin(); it != mol_names_.end(); ++it, ++i) {
    if(i < ncvs) {
      fprintf(fp, "%s\t(Control Variable)\n", (*it).latin1());
    } else {
      fprintf(fp, "%s\n", (*it).latin1());
    }
  }
  fprintf(fp, "Reactions:\n");
  for(it = rxn_equations.begin(); it != rxn_equations.end(); ++it) {
    fprintf(fp, "%s\n", (*it).latin1());
  }
  fprintf(fp, "KC:\n");
  kc.print(fp);
  fprintf(fp, "Reactants:\n");
  re.print(fp);
  fprintf(fp, "Products:\n");
  pro.print(fp);
#if 0
  fprintf(fp, "ipro:\n");
  ipro.print(fp);
  fprintf(fp, "ire:\n");
  ire.print(fp);
  fprintf(fp, "isto:\n");
  isto.print(fp);
  fprintf(fp, "jmat:\n");
  jmat.print(fp);
#endif
}

int 
PathwayMatrix::reactionLookup(const QString &name) const {
  int ind = rxn_names.findIndex(name);
  return ind;
}

int
PathwayMatrix::moleculeLookup(const QString &name) const {
  int ind = plotted_mol_names_.findIndex(name);
  return ind >= 0 ? ind+1 : ind;
}

int
PathwayMatrix::moleculeLookupAll(const QString &name) const {
  return mol_names_.findIndex(name);
}

QStringList
PathwayMatrix::plottedMoleculeNames() const {
  return plotted_mol_names_;
}

QStringList
PathwayMatrix::moleculeNames() const {
  return mol_names_;
}

int
PathwayMatrix::nPlotted() const {
  int nr = plotted_.nrows(), n = 0;
  for(int i=0; i<nr; i++) {
    if(plotted_(i, 0)) n++;
  }
  return n;
}

bool
PathwayMatrix::plotted(int i) const {
  return plotted_(i, 0);
}

void
PathwayMatrix::applyKCToItems() {
  int i=0;
  applyKCToItems(items, i);
}

void
PathwayMatrix::applyKCToItems(const QList<PathwayCanvasItem> &items, int &i) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *p;
  while((p = it.current())) {
    if(isMolecule(p)) {
      MoleculeItem *q = (MoleculeItem*)p;
      if(q->decays()) {
	q->setDecayRate(kc(i,0));
	i++;
      }
    } else if(isReaction(p)) {
      MassActionReactionItem *q = (MassActionReactionItem*)p;
      q->setKf(kc(i,0));
      i++;
      if(q->reversible()) {      
	q->setKr(kc(i,0));
	i++;
      }
    } else if(isSubnetwork(p)) {
      SubnetworkItem *q = (SubnetworkItem*)p;
      applyKCToItems(q->rawItems(), i);
    }
    ++it;
  }
}
