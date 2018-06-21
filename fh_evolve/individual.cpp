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
#include <math.h>
#ifdef WIN32
#include <float.h>
#else
#include <values.h>
#endif

#include <qlist.h>
#include <qvaluevector.h>

#include "calculate.h"
#include "individual.h"
#include "log.h"
#include "matrix.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_matrix.h"
#include "pathway_save.h"
#include "pathway_load.h"
#include "random.h"

extern int nmut;
extern Random rng;
extern PathwayMatrix pm;

extern bool final;

extern QWidget *parent_widget;

extern int tid_;
extern int frm_;

Individual::Individual()
  : current_(false),
    gene_ind_(0)
{
  // Initialize reactions to contain four genes and associated protein productions and degradations
  addGene();
  addGene();  
  addGene();  
  addGene();  
}

Individual::Individual(const Individual &ind)
  : f_(ind.f_),
    t_(ind.t_),
    current_(ind.current_),
    rxns_(ind.rxns_),
    mols_(ind.mols_),
    y0_(ind.y0_),
    gene_ind_(ind.gene_ind_)
{}

Individual &
Individual::operator = (const Individual &ind) {
  if(this != &ind) {
    f_       = ind.f_;
    t_       = ind.t_;
    current_ = ind.current_;
    rxns_    = ind.rxns_;
    mols_    = ind.mols_;
    y0_      = ind.y0_;
    gene_ind_  = ind.gene_ind_;
  }
  
  return *this;
}

void
Individual::addGene() {
  if(rxns_.empty()) {
    rxns_.resize(1, 8);
  } else {
    rxns_.expand(1, 0, 0.0);
  }
  
  if(mols_.empty()) {
    mols_.resize(2, 4);
  } else {
    mols_.expand(2, 0, 0);
  }

  mols_(mols_.nrows()-2, 0) = 1;  mols_(mols_.nrows()-2, 1) = gene_ind_;
  mols_(mols_.nrows()-1, 0) = 2;  mols_(mols_.nrows()-1, 1) = gene_ind_;  gene_ind_++;
  
  rxns_(rxns_.nrows()-1, 0) = 1;
  rxns_(rxns_.nrows()-1, 1) = mols_.nrows()-2;
  rxns_(rxns_.nrows()-1, 2) = mols_.nrows()-1;
  rxns_(rxns_.nrows()-1, 3) = rng.real1(); // transcription+translation
  rxns_(rxns_.nrows()-1, 4) = rng.real1(); // degradation

  if(y0_.empty()) {
    y0_.resize(2, 1, 0.0);
  } else {
    y0_.expand(2, 0, 0.0);
  }
  
  y0_(y0_.nrows()-2, 0) = 1;
  y0_(y0_.nrows()-1, 0) = 0;
}

void
Individual::mutate() {
  for(int mut=0; mut<nmut; mut++) {
    double mut_prob = rng.real1();
    double p[5];
    // Once the topology is advance enough to qualitatively express the phenotype focus on kinetic constants
    if(toggle() != 15) {
      //double p[5] = { 0.20, 0.40, 0.60, 0.80, 1.0 };
      //double p[5] = { 0.25, 0.50, 0.75, 1.0, 1.0 };
      p[0] = 0.30; // mutate degradation constant
      p[1] = 0.60; // mutate kinetic constant (excluding degradation constants...)
      p[2] = 0.65; // add a gene
      p[3] = 0.75; // protein or protein complex binds with promoter or promoter-protein complex
      p[4] = 1.00; // posttranslational modification
    } else {
      p[0] = 0.40; // mutate degradation constant
      p[1] = 1.00; // mutate kinetic constant (excluding degradation constants...)
      p[2] = 2.00; // add a gene
      p[3] = 2.00; // protein or protein complex binds with promoter or promoter-protein complex
      p[4] = 2.00; // posttranslational modification
    }
    
    if(mut_prob <= p[0]) { // mutate degradation constant
      int nd = 0;
      for(int i=0; i<rxns_.nrows()-1; i++) {
	switch((int)rxns_(i, 0)) {
	case 0: break;
	case 1: nd += 1; break;
	case 2: break;
	case 3: nd += 1; break;
	case 4: nd += 1; break;
	case 5: nd += 1; break;
	case 6: nd += 1; break;
	case 7: nd += 1; break;
	default: abort();
	}
      }

      int sd = rng.integer(0, nd-1);

      for(int i=0; i<rxns_.nrows()-1; i++) {
	switch((int)rxns_(i, 0)) {
	case 0: break;
	case 1: if(sd < 1) { rxns_(i, 4) *= rng.real1() * 2.0; } sd -= 1; break;
	case 2: break;
	case 3: if(sd < 1) { rxns_(i, 4) *= rng.real1() * 2.0; } sd -= 1; break;
	case 4: if(sd < 1) { rxns_(i, 5) *= rng.real1() * 2.0; } sd -= 1; break;
	case 5: if(sd < 1) { rxns_(i, 4) *= rng.real1() * 2.0; } sd -= 1; break;
	case 6: if(sd < 1) { rxns_(i, 3) *= rng.real1() * 2.0; } sd -= 1; break;
	case 7: if(sd < 1) { rxns_(i, 4) *= rng.real1() * 2.0; } sd -= 1; break;	  
	default: abort();
	}
	if(sd < 0) break;
      }
    } else if(mut_prob <= p[1]) { // mutate kinetic constant (excluding degradation constants...)
      int nr = 0;
      for(int i=0; i<rxns_.nrows()-1; i++) {
	switch((int)rxns_(i, 0)) {
	case 0: break;
	case 1: nr += 1; break;
	case 2: nr += 2; break;
	case 3: nr += 1; break;
	case 4: nr += 1; break;
	case 5: break;
	case 6: break;
	case 7: break;
	default: abort();
	}
      }

      int sr = rng.integer(0, nr-1);

      for(int i=0; i<rxns_.nrows()-1; i++) {
	switch((int)rxns_(i, 0)) {
	case 0: break;
	case 1: if(sr < 1) { rxns_(i, 3+sr) *= rng.real1() * 2.0; } sr -= 1; break;
	case 2: if(sr < 2) { rxns_(i, 5+sr) *= rng.real1() * 2.0; } sr -= 2; break;
	case 3: if(sr < 1) { rxns_(i, 3+sr) *= rng.real1() * 2.0; } sr -= 1; break;
	case 4: if(sr < 1) { rxns_(i, 4+sr) *= rng.real1() * 2.0; } sr -= 1; break;
	case 5: break;
	case 6: break;
	case 7: break;
	default: abort();
	}
	if(sr < 0) break;
      }
    } else if(mut_prob <= p[2]) { // add a gene
      addGene();
    } else if(mut_prob <= p[3]) { // protein or protein complex binds with promoter or promoter-protein complex
      int ng=0, np=0;
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 1 || mols_(i, 0) == 4) ng++;
	else if(mols_(i, 0) == 2 || mols_(i, 0) == 3) np++;
      }

      //1. find a gene or a gene:protein complex
      int m1 = rng.integer(1, ng), g = -1;
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 1 || mols_(i, 0) == 4) {
	  m1--;
	  if(m1 == 0) {
	    m1 = i;
	    g = (mols_(i, 0) == 1) ? i : mols_(i, 1);
	    break;
	  }
	}
      }
       
      //2. find a protein or a protein complex
      int m2 = rng.integer(1, np);
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 2 || mols_(i, 0) == 3) {
	  m2--;
	  if(m2 == 0) {
	    m2 = i;
	    break;
	  }
	}
      }

      mols_.expand(1, 0, 0);
      mols_(mols_.nrows()-1, 0) = 4;   mols_(mols_.nrows()-1, 1) = g;
      mols_(mols_.nrows()-1, 2) = m1;  mols_(mols_.nrows()-1, 3) = m2;
  
      rxns_.expand(1, 0, 0.0);
      rxns_(rxns_.nrows()-1, 0) = 2;
      rxns_(rxns_.nrows()-1, 1) = g; // gene (+1 to get to the protein)
      rxns_(rxns_.nrows()-1, 2) = m1; // gene or gene:protein complex
      rxns_(rxns_.nrows()-1, 3) = m2; // protein or protein complex
      rxns_(rxns_.nrows()-1, 4) = mols_.nrows()-1; // product complex
      rxns_(rxns_.nrows()-1, 5) = rng.real1(); // promoter asociation
      rxns_(rxns_.nrows()-1, 6) = rng.real1(); // promoter disassociation
      rxns_(rxns_.nrows()-1, 7) = rng.real1(); // protein production w/ promoter

      y0_.expand(1, 0, 0.0);  
      y0_(y0_.nrows()-1, 0) = 0;
    } else if(mut_prob <= p[4]) { // posttranslational modification
      int np=0, nc=0;
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 2 || mols_(i, 0) == 5) np++;
	else if(mols_(i, 0) == 3) nc++;
      }

      // pick two proteins or protein complexes
      int m1 = rng.integer(1, np + nc);
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 2 || mols_(i, 0) == 3 || mols_(i, 0) == 5) {
	  if(--m1 == 0) {
	    m1 = i;
	    break;
	  }
	}
      }
      int m2 = rng.integer(1, np + nc);      
      for(int i=0; i<mols_.nrows(); i++) {
	if(mols_(i, 0) == 2 || mols_(i, 0) == 3 || mols_(i, 0) == 5) {
	  if(--m2 == 0) {
	    m2 = i;
	    break;
	  }
	}
      }

      // If there is a complex it will be listed first
      if(mols_(m2, 0) == 3) {
	int tmp = m1;
	m1 = m2;
	m2 = tmp;
      }

      //printf("a %d %d\n", m1, m2);
      
      double ptp[5] = { 0.20, // phosphorylation
			0.40, // complex formation
			0.60, // catalytic degradation
			0.80, // partial protein complex degradation (complex required)
			1.00  // partial protein complex catalytic degradation (complex required)
      };

      // If no complex selected modify mutation probabilities
      if(mols_(m1, 0) != 3) {
	ptp[0] =  .33;
	ptp[1] =  .66;
	ptp[2] = 1.00;
	//puts("mod");
      }
      
      double pt_mut_prob = rng.real1();
#if 0
      for(int j=0; j<5; j++) {
	printf("%d %f %f %d\n", j, pt_mut_prob, ptp[j], pt_mut_prob < ptp[j]);
      }
#endif
      
      if(pt_mut_prob < ptp[0]) {
	//puts("m1");
	// phosphorylation
	mols_.expand(1, 0, 0);
	mols_(mols_.nrows()-1, 0) = 5;   mols_(mols_.nrows()-1, 1) = m1;
  
	rxns_.expand(1, 0, 0.0);
	rxns_(rxns_.nrows()-1, 0) = 3;
	rxns_(rxns_.nrows()-1, 1) = m1; // protein or protein complex
	rxns_(rxns_.nrows()-1, 2) = mols_.nrows()-1; // phosphorylated protein or protein complex
	rxns_(rxns_.nrows()-1, 3) = rng.real1(); // posttranslational modification
	rxns_(rxns_.nrows()-1, 4) = rng.real1(); // degradation of modified protein or protein complex

	y0_.expand(1, 0, 0.0);  
	y0_(y0_.nrows()-1, 0) = 0;
      } else if(pt_mut_prob < ptp[1]) {
	//puts("m2");
	// complex formation
	mols_.expand(1, 0, 0);
	mols_(mols_.nrows()-1, 0) = 3;   mols_(mols_.nrows()-1, 1) = m1; mols_(mols_.nrows()-1, 2) = m2;
  
	rxns_.expand(1, 0, 0.0);
	rxns_(rxns_.nrows()-1, 0) = 4;
	rxns_(rxns_.nrows()-1, 1) = m1; // protein or protein complex
	rxns_(rxns_.nrows()-1, 2) = m2; // protein or protein complex
	rxns_(rxns_.nrows()-1, 3) = mols_.nrows()-1; // product complex
	rxns_(rxns_.nrows()-1, 4) = rng.real1(); // complex formation
	rxns_(rxns_.nrows()-1, 5) = rng.real1(); // degradation

	y0_.expand(1, 0, 0.0);
	y0_(y0_.nrows()-1, 0) = 0;
      } else if(pt_mut_prob < ptp[2]) {
	//puts("m3");
	// catalytic degradation	
	rxns_.expand(1, 0, 0.0);
	rxns_(rxns_.nrows()-1, 0) = 5;
	rxns_(rxns_.nrows()-1, 1) = m1; // protein or protein complex
	rxns_(rxns_.nrows()-1, 2) = m2; // protein or protein complex
	rxns_(rxns_.nrows()-1, 3) = rng.boolean() ? m1 : m2; // remaining protein or protein complex
	rxns_(rxns_.nrows()-1, 4) = rng.real1(); // complex degradation
      } else if(pt_mut_prob < ptp[3]) {
	//puts("m4");
	// partial protein complex degradation (complex required)
	rxns_.expand(1, 0, 0.0);
	rxns_(rxns_.nrows()-1, 0) = 6;
	rxns_(rxns_.nrows()-1, 1) = m1; // protein complex
	rxns_(rxns_.nrows()-1, 2) = rng.boolean() ? mols_(m1, 1) : mols_(m1, 2); // protein
	//mols_.print();
	//printf("%d %d %d\n", m1, mols_(m1, 1), mols_(m1, 2));
	rxns_(rxns_.nrows()-1, 3) = rng.real1(); // complex degradation
      } else if(pt_mut_prob < ptp[4]) {
	//puts("m5");
	// partial protein complex catalytic degradation (complex required)
	rxns_.expand(1, 0, 0.0);
	rxns_(rxns_.nrows()-1, 0) = 7;
	rxns_(rxns_.nrows()-1, 1) = m1; // protein complex
	rxns_(rxns_.nrows()-1, 2) = m2; // protein or protein complex
	rxns_(rxns_.nrows()-1, 3) = rng.boolean() ? mols_(m1, 1) : mols_(m1, 2); // choose a protein from the complex
	rxns_(rxns_.nrows()-1, 4) = rng.real1(); // complex degradation
      }
    }
  }

  current_ = false;
}

bool
Individual::convertToPM() {
#if 0
  int nc = 2; // number of control variables
  int nr = 0;
  int nm = mols_.nrows() + nc;
  for(int i=0; i<rxns_.nrows(); i++) {
    switch((int)rxns_(i, 0)) {
    case 0: break;
    case 1:
      nr += 2; // one reaction for translation and one for degradation
      break;
    case 2:
      nr += 3; // association, disassociation, and production
      break;
    case 3:
      nr += 2; // posttranslational modification and degradation
      break;
    case 4:
      nr += 2; // complex formation and degradation
      break;
    case 5:
      nr += 1; // catalytic degradation
      break;
    case 6:
      nr += 1; // partial protein complex degradation
      break;
    case 7:
      nr += 1; // partial protein complex catalytic degradation
      break;
    default:
      fprintf(stderr, "Unknown reaction type %d\n", (int)rxns_(i, 0));
      break;
    }
  }

  nr += nc;
  
  // Fill out PathwayMatrix      
  
  pm.re = IMatrix(nm, nr, 0);
  pm.pro = IMatrix(nm, nr, 0);
  pm.kc = Matrix(nr, 1, 0.0);
  
  pm.atol = Matrix(nm, 1, pm.simulation.absoluteTolerance());
  pm.plotted_.resize(nm, 1);
  pm.plotted_.fill(true);

  //pm.y0 = y0_;
  pm.y0 = Matrix(y0_.nrows()+nc, 1);
  pm.y0(0, 0) = 0;
  pm.y0(1, 0) = 0;
  for(int i=0; i<y0_.nrows(); i++) {
    pm.y0(i+nc, 0) = y0_(i, 0);
  }
  
  pm.kc(0, 0) = 10;
  pm.re(0, 0) = 1;
  pm.pro(5, 0) = 1;
  pm.kc(1, 0) = 10;
  pm.re(1, 1) = 1;
  pm.pro(3, 1) = 1;

  pm.cvs.resize(2);
  SquareWaveControlVariableItem *cv;
  cv = new SquareWaveControlVariableItem(NULL);
  cv->setLabel("CV1");
  cv->setValues(0, 1);
  cv->setDuration(2000, 1);
  cv->setTransitionTime(0, 0);
  cv->setPhaseShift(-1000);
  cv->setInitialized(true);
  pm.cvs[0] = cv;
  
  cv = new SquareWaveControlVariableItem(NULL);
  cv->setLabel("CV2");
  cv->setValues(0, 1);
  cv->setDuration(2000, 1);
  cv->setTransitionTime(0, 0);
  cv->setPhaseShift(0);
  cv->setInitialized(true);
  pm.cvs[1] = cv;
  
  // When filling out the re and pro matrices use ++ instead of =1 since
  // the same matrix element may be referenced multiple times by the
  // same reaction
  for(int i=0, r=nc; i<rxns_.nrows(); i++) {
    switch((int)rxns_(i, 0)) {
    case 0: break;
    case 1:
      // translation
      pm.kc(r, 0) = rxns_(i, 3);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.pro((int)rxns_(i, 1)+nc, r)++;
      pm.pro((int)rxns_(i, 2)+nc, r)++;
      r++;
      
      // degradation
      pm.kc(r, 0) = rxns_(i, 4);
      pm.re((int)rxns_(i, 2)+nc, r)++;
      r++;
      break;
    case 2:
      // association
      // decrement gene or gene:protein complex
      pm.kc(r, 0) = rxns_(i, 5);
      pm.re((int)rxns_(i, 2)+nc, r)++;
      pm.re((int)rxns_(i, 3)+nc, r)++;
      pm.pro((int)rxns_(i, 4)+nc, r)++;
      r++;
      
      // disassociation
      pm.kc(r, 0) = rxns_(i, 6);
      pm.re((int)rxns_(i, 4)+nc, r)++;
      pm.pro((int)rxns_(i, 2)+nc, r)++;
      pm.pro((int)rxns_(i, 3)+nc, r)++;
      r++;
      
      // protein production
      pm.kc(r, 0) = rxns_(i, 7);
      pm.re((int)rxns_(i, 4)+nc, r)++;
      pm.pro((int)rxns_(i, 4)+nc, r)++;
      pm.pro((int)rxns_(i, 1)+1+nc, r)++;
      r++;
      break;
    case 3:
      // posttranslational modification
      pm.kc(r, 0) = rxns_(i, 3);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.pro((int)rxns_(i, 2)+nc, r)++;
      r++;
      
      // degradation
      pm.kc(r, 0) = rxns_(i, 4);
      pm.re((int)rxns_(i, 2)+nc, r)++;
      r++;
      break;
    case 4:
      // complex formation
      pm.kc(r, 0) = rxns_(i, 4);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.re((int)rxns_(i, 2)+nc, r)++;
      pm.pro((int)rxns_(i, 3)+nc, r)++;
      r++;
      
      // degradation
      pm.kc(r, 0) = rxns_(i, 5);
      pm.re((int)rxns_(i, 3)+nc, r)++;
      r++;
      break;
    case 5:
      // catalytic degradation
      pm.kc(r, 0) = rxns_(i, 4);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.re((int)rxns_(i, 2)+nc, r)++;
      pm.pro((int)rxns_(i, 3)+nc, r)++;
      r++;
      break;	
    case 6:
      // partial protein complex degradation
      pm.kc(r, 0) = rxns_(i, 3);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.pro((int)rxns_(i, 2)+nc, r)++;
      r++;
      break;
    case 7:
      // partial protein complex catalytic degradation
      pm.kc(r, 0) = rxns_(i, 4);
      pm.re((int)rxns_(i, 1)+nc, r)++;
      pm.re((int)rxns_(i, 2)+nc, r)++;
      pm.pro((int)rxns_(i, 3)+nc, r)++;
      r++;
      break;
    default:
      fprintf(stderr, "Unknown reaction type %d\n", (int)rxns_(i, 0));
      abort();
    }
  }

  {
    for(int i=0; i<nm; i++) {
      QString tmp;
      tmp.sprintf("Mol %d", i+1);
      pm.plotted_mol_names_.append(tmp);
    }
  }

  {
    QList<MoleculeItem> mol;
    pm.setupPlottedMoleculesList(mol);
  }
  pm.setupSparseMatrices();
#endif
  
  //if(final) pm.print();

  return true;
}

int
toggle(int nc, const Matrix &r) {
  return ((((r(1000*10-1, 1+1+nc) - r(2000*10-1, 1+1+nc) > 1.0) &&
            (r(2000*10-1, 1+1+nc) - r(3000*10-1, 1+1+nc) < 1.0) &&
            (r(3000*10-1, 1+1+nc) - r(4000*10-1, 1+1+nc) > 1.0)) ? 1 : 0) +
          (((r(1000*10-1, 3+1+nc) - r(2000*10-1, 3+1+nc) < 1.0) &&
            (r(2000*10-1, 3+1+nc) - r(3000*10-1, 3+1+nc) > 1.0) &&
            (r(3000*10-1, 3+1+nc) - r(4000*10-1, 3+1+nc) < 1.0)) ? 2 : 0));
}

void
applyIntialConcentrations(const Result &result, QList<PathwayCanvasItem> &items) {
  int nr = result.m.nrows();
  QListIterator<PathwayCanvasItem> it(items);
  for(it.toFirst(); it.current(); ++it) {
    if(isMolecule(it.current())) {
      MoleculeItem *item = (MoleculeItem*)it.current();
      QString l = item->label();
      int ind = result.column_labels.findIndex(l);
      if(ind == -1) {
	fprintf(stderr, "Unable to find %s\n", (const char*)l);
	exit(EXIT_FAILURE);
      } else {
	item->setInitialConcentration(result.m(nr-1, ind+1));
      }
    }
  }
}

double
Individual::fitness() {
  if(!current_ || final) {
    f_ = DBL_MAX;
    t_ = 0;
    current_ = true;  // set current here so if we have to leave on error it is already set

#if 0
    convertToPM();
#else
    QList<PathwayCanvasItem> items;
    if(!convertToItems(items)) return false;
    Editor editor;
    Fitness fitness;

    //if(!pathwayLoad("in.net", items)) abort();
    
    pm.setup(editor, pm.simulation, items);
#endif

    SquareWaveControlVariableItem *cv1=NULL, *cv2=NULL;
    
    QListIterator<PathwayCanvasItem> it(items);
    for(; it.current(); ++it) {
      if(isControlVariable(it.current())) {
	SquareWaveControlVariableItem *cv = (SquareWaveControlVariableItem*)it.current();
	if(cv->label() == "CV1") {
	  cv1 = cv;
	}
	if(cv->label() == "CV2") {
	  cv2 = cv;
	}
      }
    }

    if(!cv1 || !cv2) {
      fprintf(stderr, "Could not find cv1 or cv2\n");
      exit(EXIT_FAILURE);
    }
    
    MoleculeItem *p1=NULL, *p2=NULL, *p3=NULL, *p4=NULL;

    for(it.toFirst(); it.current(); ++it) {
      if(isMolecule(it.current())) {
        MoleculeItem *item = (MoleculeItem*)it.current();
        if(item->label() == "P1") {
          p1 = item;
        } else if(item->label() == "P2") {
          p2 = item;
        } else if(item->label() == "P3") {
          p3 = item;
        } else if(item->label() == "P4") {
          p4 = item;
        }
      }
    }

    if(!p1 || !p2 || !p3 || !p4) {
      fprintf(stderr, "Could not find p1 or p2 or p3 or p4\n");
      exit(EXIT_FAILURE);
    }

    Result r;
    double res[4] = { 0 };

    cv1->setValues(0, 0);
    cv2->setValues(0, 0);
    {
      if(!run_simulation(pm, r)) {
	return f_;
      }
      applyIntialConcentrations(r, items);
      res[0] = p4->initialConcentration();
    }
    
    cv1->setValues(1, 1);
    cv2->setValues(0, 0);
    {
      if(!run_simulation(pm, r)) {
	return f_;
      }
      applyIntialConcentrations(r, items);
      res[1] = p4->initialConcentration();
    }

    cv1->setValues(0, 0);
    cv2->setValues(1, 1);
    {
      if(!run_simulation(pm, r)) {
	return f_;
      }
      applyIntialConcentrations(r, items);
      res[2] = p4->initialConcentration();
    }

    cv1->setValues(1, 1);
    cv2->setValues(1, 1);
    {
      if(!run_simulation(pm, r)) {
	return f_;
      }
      applyIntialConcentrations(r, items);
      res[3] = p4->initialConcentration();
    }

    f_ = 0.0;
    //int nc = pm.cvs.size();

#if 0
    const char *names[30] = { "d012",
			      "d016",
			      "d018",
			      "d019",
			      "d028",
			      "d032",
			      "d038",
			      "d052",
			      "d066",
			      "d078",
			      "d090",
			      "d101",
			      "d104",
			      "d113",
			      "d114",
			      "d117",
			      "d123",
			      "d135",
			      "d143",
			      "d180",
			      "d250",
			      "d253",
			      "c024",
			      "c101",
			      "c103",
			      "c113",
			      "c144",
			      "c195",
			      "c242" };
			      
    const double target[30][4] = { {  9799.553198,    135.4443276,   384.7535858,   207.8154647 },
				   { 28855.18953,    1058.355852,    769.8561048,   633.872162  },
				   {  5746.875748,   4954.328763,   6977.720684,   6682.387208  },
				   {   331.4371422,   529.6586769, 10787.62164,     168.670177  },
				   {   571.3947982,   658.9516266,   284.0914529,   255.0539699 },
				   { 48101.94215,    1047.139287,  48895.47552,     737.5004978 },
				   {  427.4511138,    550.8609027, 13733.83285,     235.2039094 },
				   { 13557.12703,     467.7863026,   435.0814279,   686.2104438 },
				   {  7853.719169,   1621.621812,    410.1682259,   435.9175599 },
				   {   102.9218462,   146.2081607, 48589.23576,     407.9522322 },
				   {   203.5580303,   162.6739245,    92.84084969,  125.2698354 },
				   { 38861.4653,      591.6230832,  4654.478095,    885.1750024 },
				   {   215.8780725,   210.1671723,   234.7659844,   343.558898  },
				   { 35163.87507,    1836.717209,  38297.84936,    1125.719529  },
				   {   218.8725898,   276.0236019,   446.3960292,   416.1288741 },
				   { 45398.87137,   46253.62782,   51533.71546,   52351.76007   },
				   {   653.3381131,   866.5852087,   402.4610459,   282.9938732 },
				   { 43474.81651,   44461.51841,   47795.18899,   53051.27844   },
				   {  6088.122013,   5654.585624,   5820.304567,   5765.541927  },
				   { 14218.98748,   14124.02605,     568.3894219,   451.3978103 },
				   { 47496.43355,   48890.79477,     791.2395752,   885.5921033 },
				   {   795.731815,    296.9889901,   354.3590805,   182.9878827 },
				   {   885.4586605,   799.1516185,   325.6498251,   204.3227276 },
				   { 13922.27517,     439.0845826,   538.8380623,   345.9891967 },
				   {   176.7035176,  9121.305779,    297.036003,    226.8860286 },
				   { 11305.01212,    2440.715485,    818.0481013,   653.8954314 },
				   { 13454.03927,    1798.283955,    447.3287101,   530.8327948 },
				   { 31623.441,      4857.924882,    534.196326,    656.0131487 },
				   { 46271.42491,   18231.92675,   68665.36567,   56802.93089   },
				   { 17403.90257,     715.8412109,   567.656246,    442.333952  } };

    int tid = tid_;
    
    f_ += sqr(log(res[0]) - log(target[tid][0]));
    f_ += sqr(log(res[1]) - log(target[tid][0]));
    f_ += sqr(log(res[2]) - log(target[tid][0]));
    f_ += sqr(log(res[3]) - log(target[tid][0]));

    f_ = sqrt(f_);
    
    //t_ = ::toggle(2, r.m);

#if 1
    switch(tid) {
    case 0:
      t_ = ((res[1]*6 < res[0] ? 1 : 0) +
	    (res[2]*6 < res[0] ? 2 : 0) +
	    (res[3]*6 < res[0] ? 4 : 0));
      break;
    case 6:
      t_ = ((res[0]*3 < res[2] ? 1 : 0) +
	    (res[1]*2 < res[2] ? 2 : 0) +
	    (res[3]*6 < res[2] ? 4 : 0));
      break;
    default:
      abort();
    }
#else
    double v[3] = { (res[1]/res[0]) / (target[tid][1]/target[tid][0]),
		    (res[2]/res[0]) / (target[tid][2]/target[tid][0]),
		    (res[3]/res[0]) / (target[tid][3]/target[tid][0]) };
    t_ = (((0.6 <= v[0] && v[0] <= 1.4) ? 1 : 0) +
	  ((0.6 <= v[1] && v[1] <= 1.4) ? 2 : 0) +
	  ((0.6 <= v[2] && v[2] <= 1.4) ? 4 : 0));
#endif
    
    //printf("%.2f %.2f    %f %d    %f %f %f %f   %f %f %f %f\n", fabs(res[0][0]*3-res[1][0]-res[2][0]-res[3][0]), fabs(res[0][1]*3-res[1][1]-res[2][1]-res[3][1]), f_, t_, res[0][0], res[1][0], res[2][0], res[3][0], res[0][1], res[1][1], res[2][1], res[3][1]);
#else
    int tid=tid_;

    double avg = (res[0] + res[1] + res[2] + res[3]) / 4.0;

    f_ = sqrt( ((tid & 0x01) ? GenoDYN::sqr(res[3] - 100) : GenoDYN::sqr(res[3] - 1)) +
	       ((tid & 0x02) ? GenoDYN::sqr(res[2] - 100) : GenoDYN::sqr(res[2] - 1)) +
	       ((tid & 0x04) ? GenoDYN::sqr(res[1] - 100) : GenoDYN::sqr(res[1] - 1)) +
	       ((tid & 0x08) ? GenoDYN::sqr(res[0] - 100) : GenoDYN::sqr(res[0] - 1)) );

if(tid_ > 0) {
    t_ = (((((tid & 0x01) && res[3] > 1.5*avg) || ((~tid & 0x01) && res[3] < 0.5*avg)) ? 1 : 0) + 
	  ((((tid & 0x02) && res[2] > 1.5*avg) || ((~tid & 0x02) && res[2] < 0.5*avg)) ? 2 : 0) + 
	  ((((tid & 0x04) && res[1] > 1.5*avg) || ((~tid & 0x04) && res[1] < 0.5*avg)) ? 4 : 0) + 
	  ((((tid & 0x08) && res[0] > 1.5*avg) || ((~tid & 0x08) && res[0] < 0.5*avg)) ? 8 : 0));
} else {
  t_ = (((res[3] < 5) ? 1 : 0) +
        ((res[2] < 5) ? 2 : 0) +
        ((res[1] < 5) ? 4 : 0) +
        ((res[0] < 5) ? 8 : 0));
}

    /*
    t_ = ((((tid & 0x01) ^ (res[0] < 0avg)) ? 1 : 0) +
	  (((tid & 0x02) ^ (res[1] < avg)) ? 2 : 0) +
	  (((tid & 0x04) ^ (res[2] < avg)) ? 4 : 0) +
	  (((tid & 0x08) ^ (res[3] < avg)) ? 8 : 0));
    */
    
    //printf("%f  %f %f %f %f  %f  %d\n", avg, res[0], res[1], res[2], res[3], f_, t_);
#endif
    
    current_ = true;
  }
  
  return f_;
}

bool
Individual::save(const char *filename) {
  return false;
#if 0
  convertToPM();

  {
    char str[1024];
    sprintf(str, "%s.mols", filename);
    FILE *file = fopen(str, "w");
    mols_.print(file);
    fclose(file);
  }

  {
    char str[1024];
    sprintf(str, "%s.rxns", filename);
    FILE *file = fopen(str, "w");
    rxns_.print(file);
    fclose(file);
  }

  {
    QList<PathwayCanvasItem> items;
    int nc = pm.cvs.size();
    QValueVector<MoleculeItem*> mols(mols_.nrows() + nc);
    for(int i=0; i<nc; i++) {
      mols[i] = (ControlVariableItem*)(pm.cvs[i]->clone());
      items.append(mols[i]);
    }
    QString str;
    for(int i=0; i<mols_.nrows(); i++) {
      switch(mols_(i, 0)) {
      case 0: break;
      case 1:
	mols[i+nc] = new DNAMoleculeItem(parent_widget);
	str.sprintf("G%03d", mols_(i, 1));
	mols[i+nc]->setLabel(str);
	mols[i+nc]->setInitialConcentration(y0_(i, 0));
	mols[i+nc]->setInitialized(true);
	items.append(mols[i+nc]);
	break;
      case 2:
	mols[i+nc] = new TranscriptionFactorMoleculeItem(parent_widget);
	str.sprintf("P%03d", mols_(i, 1));
	mols[i+nc]->setLabel(str);
	mols[i+nc]->setInitialConcentration(y0_(i, 0));
	mols[i+nc]->setInitialized(true);
	items.append(mols[i+nc]);
	break;
      case 3:
	mols[i+nc] = new ComplexMoleculeItem(parent_widget);
	str.sprintf("PC%03d_%03d", mols_(i, 1), mols_(i, 1));
	mols[i+nc]->setLabel(str);
	mols[i+nc]->setInitialConcentration(y0_(i, 0));
	mols[i+nc]->setInitialized(true);
	items.append(mols[i+nc]);
	break;
      case 4:
	mols[i+nc] = new DNAMoleculeItem(parent_widget);
	str.sprintf("G%03d:(P%03d)", mols_(mols_(i, 1), 1),mols_(mols_(i, 3), 1));
	mols[i+nc]->setLabel(str);
	mols[i+nc]->setInitialConcentration(y0_(i, 0));
	mols[i+nc]->setInitialized(true);
	items.append(mols[i+nc]);
	break;
      case 5:
	mols[i+nc] = new EnzymeMoleculeItem(parent_widget);
	str.sprintf("P%03d*", mols_(i, 2));
	mols[i+nc]->setLabel(str);
	mols[i+nc]->setInitialConcentration(y0_(i, 0));
	mols[i+nc]->setInitialized(true);
	items.append(mols[i+nc]);
	break;
      default:
	abort();
      }
    }
    
#if 0
    puts("ire");
    pm.ire.print();
    puts("ipro");
    pm.ipro.print();
#endif
    
    for(int i=0; i<pm.kc.nrows(); i++) {
      str.sprintf("R%03d", i);
      MassActionReactionItem *m = new MassActionReactionItem(parent_widget);
      m->setLabel(str);
      m->setKf(pm.kc(i, 0));
      m->setInitialized(true);
      items.append(m);
      
      for(int j=1; j<=pm.ire(0, i); j++) {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	r->appendNode(mols[pm.ire(j, i)]);
	r->appendNode(m);
	r->setCoefficient(pm.re(pm.ire(j, i), i));
	items.append(r);
      }

      for(int j=1; j<=pm.ipro(0, i); j++) {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	r->appendNode(m);
	r->appendNode(mols[pm.ipro(j, i)]);
	r->setCoefficient(pm.pro(pm.ipro(j, i), i));
	items.append(r);
      }
    }

    Editor editor;
    Fitness fitness;
    bool rv = pathwaySave(filename, editor, fitness, pm.simulation, items);

    return rv;
  }
#endif
} 

#include "pathway_reports.h"

bool
Individual::save2(const char *filename) {
  if(0) {
    QList<PathwayCanvasItem> items;
    if(!convertToItems(items, false)) return false;
    
    QString text;
    buildMoleculeReducedEquationsMathematica(items, text, 0);
    QFile file(QString(filename) + ".m");
    file.open(IO_WriteOnly);
    QTextStream stream(&file);
    stream << text << '\n';
  }

  {
    QList<PathwayCanvasItem> items;
    if(!convertToItems(items, true, true)) return false;
    
    Editor editor;
    Fitness fitness;
    Simulation sim = pm.simulation;
    sim.setSimulateToSteadyState(false);
    sim.setTimeEnd(4000);
    return pathwaySave(filename, editor, fitness, sim, items);
  }
}

bool
Individual::convertToItems(QList<PathwayCanvasItem> &items, bool includeCVs, bool easyCVs) {
  items.clear();
  items.setAutoDelete(true);

  {
    QValueVector<MoleculeItem*> mols(mols_.nrows());
    QString str;

    // Molecules can be named in the order of creation since they are created in order
    // No need to name genes and proteins first even though their names form the basis for the complexes
    for(int i=0, g=0, p=0, pc=0, pg=0, pp=0; i<mols_.nrows(); i++) {
      switch(mols_(i, 0)) {
      case 0: break;
      case 1:
	mols[i] = new DNAMoleculeItem(parent_widget);
	str.sprintf("G%d", ++g /*mols_(i, 1)*/);
	mols[i]->setLabel(str);
	mols[i]->setInitialConcentration(y0_(i, 0));
	mols[i]->setInitialized(true);
	items.append(mols[i]);
	break;
      case 2:
	mols[i] = new TranscriptionFactorMoleculeItem(parent_widget);
	str.sprintf("P%d", ++p /*mols_(i, 1)*/);
	mols[i]->setLabel(str);
	mols[i]->setInitialConcentration(y0_(i, 0));
	mols[i]->setInitialized(true);
	items.append(mols[i]);
	break;
      case 3:
	mols[i] = new ComplexMoleculeItem(parent_widget);
	str.sprintf("PC%d ((%s):(%s))", ++pc, (const char*)mols[mols_(i, 1)]->label(), (const char*)mols[mols_(i, 2)]->label());
	mols[i]->setLabel(str);
	mols[i]->setInitialConcentration(y0_(i, 0));
	mols[i]->setInitialized(true);
	items.append(mols[i]);
	break;
      case 4:
	mols[i] = new DNAMoleculeItem(parent_widget);
	str.sprintf("PG%d ((%s):(%s))", ++pg, (const char*)mols[mols_(i, 2)]->label(), (const char*)mols[mols_(i, 3)]->label());
	mols[i]->setLabel(str);
	mols[i]->setInitialConcentration(y0_(i, 0));
	mols[i]->setInitialized(true);
	items.append(mols[i]);
	break;
      case 5:
	mols[i] = new EnzymeMoleculeItem(parent_widget);
	str.sprintf("PP%d (%s)*", ++pp, (const char*)mols[mols_(i, 1)]->label());
	mols[i]->setLabel(str);
	mols[i]->setInitialConcentration(y0_(i, 0));
	mols[i]->setInitialized(true);
	items.append(mols[i]);
	break;
      default:
	abort();
      }
    }




  // Ideally the motifs would be loaded from external subnetwork files.
  // Could imagine including information on acceptiable mutations in the external files.
  
    int ct[8] = { 0 };
  for(int i=0; i<rxns_.nrows(); i++) {
    int rxn_id = (int)rxns_(i, 0);
    switch(rxn_id) {
    case 0: break;
    case 1:
      // Protein production
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);
	  
	  DNAMoleculeItem *gene = new DNAMoleculeItem(parent_widget);
	  gene->setLabel("Gene");
	  gene->setExported(true);
	  gene->moveTo(64, 64);
	  gene->setInitialized(true);
	  sn_items.append(gene);
	  
	  TranscriptionFactorMoleculeItem *protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein->setLabel("Protein");
	  protein->setExported(true);
	  protein->moveTo(288, 64);
	  protein->setInitialized(true);
	  sn_items.append(protein);
	  
	  // translation
	  {
	    MassActionReactionItem *ma_trans = new MassActionReactionItem(parent_widget);
	    ma_trans->setLabel("Translation");
	    ma_trans->setInitialized(true);
	    ma_trans->setKf(rxns_(i, 3));
	    ma_trans->moveTo(176, 64);
	    sn_items.append(ma_trans);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(gene);
	    e1->appendNode(ma_trans);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(ma_trans);
	    e2->appendNode(gene);
	    sn_items.append(e2);
	    
	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma_trans);
	    e3->appendNode(protein);
	    sn_items.append(e3);
	  }
	  
	  // degradation
	  {
	    MassActionReactionItem *ma_degrad = new MassActionReactionItem(parent_widget);
	    ma_degrad->setLabel("Degradation");
	    ma_degrad->setInitialized(true);
	    ma_degrad->setKf(rxns_(i, 4));
	    ma_degrad->moveTo(400, 64);
	    sn_items.append(ma_degrad);
	    
	    ReactionEdgeItem *e4 = new ReactionEdgeItem(parent_widget);
	    e4->appendNode(protein);
	    e4->appendNode(ma_degrad);
	    sn_items.append(e4);
	  }
	  
	  QString str;
	  str.sprintf("Trans/Degrad %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);
	
	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Gene");
	e1->appendNode(mols[(int)rxns_(i, 1)]);
	items.append(e1);
	
	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Protein");
	e2->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e2);
      }      
      break;
    case 2:
      // protein production with a bound promoter
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);
	  
	  DNAMoleculeItem *gene = new DNAMoleculeItem(parent_widget);
	  gene->setLabel("Gene");
	  gene->setExported(true);
	  gene->moveTo(208, 96);
	  gene->setInitialized(true);
	  sn_items.append(gene);
	  
	  TranscriptionFactorMoleculeItem *tf = new TranscriptionFactorMoleculeItem(parent_widget);
	  tf->setLabel("Trans. Factor");
	  tf->setExported(true);
	  tf->moveTo(64, 96);
	  tf->setInitialized(true);
	  sn_items.append(tf);

	  TranscriptionFactorMoleculeItem *gtf_complex = new TranscriptionFactorMoleculeItem(parent_widget);
	  gtf_complex->setLabel("Gene:TF");
	  gtf_complex->setExported(true);
	  gtf_complex->moveTo(352, 96);
	  gtf_complex->setInitialized(true);
	  sn_items.append(gtf_complex);

	  TranscriptionFactorMoleculeItem *protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein->setLabel("Protein");
	  protein->setExported(true);
	  protein->moveTo(576, 96);
	  protein->setInitialized(true);
	  sn_items.append(protein);

	  // association
	  // decrement gene or gene:protein complex
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Complex Formation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 5));
	    ma->moveTo(208, 160);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(gene);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(tf);
	    e2->appendNode(ma);
	    sn_items.append(e2);
	    
	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(gtf_complex);
	    sn_items.append(e3);
	  }

	  // disassociation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Complex Disassociation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 6));
	    ma->moveTo(208, 32);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(gtf_complex);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(ma);
	    e2->appendNode(gene);
	    sn_items.append(e2);
	    
	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(tf);
	    sn_items.append(e3);
	  }
	  
	  // protein production
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Translation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 7));
	    ma->moveTo(464, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(gtf_complex);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(ma);
	    e2->appendNode(gtf_complex);
	    sn_items.append(e2);
	    
	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(protein);
	    sn_items.append(e3);
	  }
	  
	  QString str;
	  str.sprintf("Promoted Trans %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);
	
	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Gene");
	e1->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e1);
	
	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Trans. Factor");
	e2->appendNode(mols[(int)rxns_(i, 3)]);
	items.append(e2);

      	SubnetworkEdgeItem *e3 = new SubnetworkEdgeItem(parent_widget);
	e3->appendNode(sn, "Gene:TF");
	e3->appendNode(mols[(int)rxns_(i, 4)]);
	items.append(e3);

      	SubnetworkEdgeItem *e4 = new SubnetworkEdgeItem(parent_widget);
	e4->appendNode(sn, "Protein");
	e4->appendNode(mols[(int)rxns_(i, 1) + 1]);
	items.append(e4);
      }      
      break;
    case 3:
      // phosphorylation
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);
	  
	  TranscriptionFactorMoleculeItem *protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein->setLabel("Protein");
	  protein->setExported(true);
	  protein->moveTo(68, 64);
	  protein->setInitialized(true);
	  sn_items.append(protein);
	  
	  TranscriptionFactorMoleculeItem *p_protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  p_protein->setLabel("Protein*");
	  p_protein->setExported(true);
	  p_protein->moveTo(292, 64);
	  p_protein->setInitialized(true);
	  sn_items.append(p_protein);
	  
	  // phosphorylation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Phosphorylation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 3));
	    ma->moveTo(180, 64);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(protein);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(ma);
	    e2->appendNode(p_protein);
	    sn_items.append(e2);
	  }
	  
	  // degradation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Degradation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 4));
	    ma->moveTo(404, 64);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(p_protein);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	  }
	  
	  QString str;
	  str.sprintf("Phosphorylation %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);
	
	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Protein");
	e1->appendNode(mols[(int)rxns_(i, 1)]);
	items.append(e1);

      	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Protein*");
	e2->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e2);
      }      
      break;
    case 4:
      // complex formation
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);
	  
	  TranscriptionFactorMoleculeItem *protein1 = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein1->setLabel("Protein A");
	  protein1->setExported(true);
	  protein1->moveTo(100, 32);
	  protein1->setInitialized(true);
	  sn_items.append(protein1);
	  
	  TranscriptionFactorMoleculeItem *protein2 = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein2->setLabel("Protein B");
	  protein2->setExported(true);
	  protein2->moveTo(100, 160);
	  protein2->setInitialized(true);
	  sn_items.append(protein2);

	  ComplexMoleculeItem *ab_protein = new ComplexMoleculeItem(parent_widget);
	  ab_protein->setLabel("A:B");
	  ab_protein->setExported(true);
	  ab_protein->moveTo(244, 96);
	  ab_protein->setInitialized(true);
	  sn_items.append(ab_protein);

	  // complex formation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Complex Formation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 4));
	    ma->moveTo(100, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(protein1);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(protein2);
	    e2->appendNode(ma);
	    sn_items.append(e2);

	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(ab_protein);
	    sn_items.append(e3);
	  }
	  
	  // degradation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Degradation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 5));
	    ma->moveTo(356, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(ab_protein);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	  }
	  
	  QString str;
	  str.sprintf("Complex Formation %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);
	
	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Protein A");
	e1->appendNode(mols[(int)rxns_(i, 1)]);
	items.append(e1);

      	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Protein B");
	e2->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e2);

	SubnetworkEdgeItem *e3 = new SubnetworkEdgeItem(parent_widget);
	e3->appendNode(sn, "A:B");
	e3->appendNode(mols[(int)rxns_(i, 3)]);
	items.append(e3);
      }      
      break;
    case 5:
      // catalytic degradation
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);
	  
	  TranscriptionFactorMoleculeItem *protein1 = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein1->setLabel("Protein A");
	  protein1->setExported(true);
	  protein1->moveTo(100, 32);
	  protein1->setInitialized(true);
	  sn_items.append(protein1);
	  
	  TranscriptionFactorMoleculeItem *protein2 = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein2->setLabel("Protein B");
	  protein2->setExported(true);
	  protein2->moveTo(100, 160);
	  protein2->setInitialized(true);
	  sn_items.append(protein2);

	  // catalytic degradation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Catalytic Degradation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 4));
	    ma->moveTo(100, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(protein1);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(protein2);
	    e2->appendNode(ma);
	    sn_items.append(e2);

	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(protein1);
	    sn_items.append(e3);
	  }
	  
	  QString str;
	  str.sprintf("Catalytic Degradation of B %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);

	if(rxns_(i, 1) == rxns_(i, 3)) {
	  SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	  e1->appendNode(sn, "Protein A");
	  e1->appendNode(mols[(int)rxns_(i, 1)]);
	  items.append(e1);
	  
	  SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	  e2->appendNode(sn, "Protein B");
	  e2->appendNode(mols[(int)rxns_(i, 2)]);
	  items.append(e2);
	} else {
	  SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	  e1->appendNode(sn, "Protein A");
	  e1->appendNode(mols[(int)rxns_(i, 2)]);
	  items.append(e1);
	  
	  SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	  e2->appendNode(sn, "Protein B");
	  e2->appendNode(mols[(int)rxns_(i, 1)]);
	  items.append(e2);
	}	
      }      
      break;
    case 6:
      // partial protein complex degradation
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;	
	  sn_items.setAutoDelete(true);
  
	  TranscriptionFactorMoleculeItem *ab_protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  ab_protein->setLabel("Protein Complex");
	  ab_protein->setExported(true);
	  ab_protein->moveTo(100, 32);
	  ab_protein->setInitialized(true);
	  sn_items.append(ab_protein);
	  
	  TranscriptionFactorMoleculeItem *protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein->setLabel("Protein A");
	  protein->setExported(true);
	  protein->moveTo(100, 160);
	  protein->setInitialized(true);
	  sn_items.append(protein);

	  // catalytic degradation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Complex Degradation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 3));
	    ma->moveTo(100, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(ab_protein);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(ma);
	    e2->appendNode(protein);
	    sn_items.append(e2);
	  }
	  
	  QString str;
	  str.sprintf("Complex Degradation %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);

	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Protein Complex");
	e1->appendNode(mols[(int)rxns_(i, 1)]);
	items.append(e1);
	
	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Protein A");
	e2->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e2);
      }      
      break;
    case 7:
      // partial protein complex catalytic degradation
      {
	SubnetworkItem *sn = new SubnetworkItem(parent_widget);
	{
	  QList<PathwayCanvasItem> sn_items;
	  sn_items.setAutoDelete(true);

	  TranscriptionFactorMoleculeItem *ab_protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  ab_protein->setLabel("Protein Complex");
	  ab_protein->setExported(true);
	  ab_protein->moveTo(100, 32);
	  ab_protein->setInitialized(true);
	  sn_items.append(ab_protein);
	  
	  TranscriptionFactorMoleculeItem *catalyst = new TranscriptionFactorMoleculeItem(parent_widget);
	  catalyst->setLabel("Catalyst");
	  catalyst->setExported(true);
	  catalyst->moveTo(100, 160);
	  catalyst->setInitialized(true);
	  sn_items.append(catalyst);

	  TranscriptionFactorMoleculeItem *protein = new TranscriptionFactorMoleculeItem(parent_widget);
	  protein->setLabel("Protein A");
	  protein->setExported(true);
	  protein->moveTo(292, 96);
	  protein->setInitialized(true);
	  sn_items.append(protein);

	  // catalytic degradation
	  {
	    MassActionReactionItem *ma = new MassActionReactionItem(parent_widget);
	    ma->setLabel("Catalytic Complex Degradation");
	    ma->setInitialized(true);
	    ma->setKf(rxns_(i, 4));
	    ma->moveTo(100, 96);
	    sn_items.append(ma);
	    
	    ReactionEdgeItem *e1 = new ReactionEdgeItem(parent_widget);
	    e1->appendNode(ab_protein);
	    e1->appendNode(ma);
	    sn_items.append(e1);
	    
	    ReactionEdgeItem *e2 = new ReactionEdgeItem(parent_widget);
	    e2->appendNode(catalyst);
	    e2->appendNode(ma);
	    sn_items.append(e2);

	    ReactionEdgeItem *e3 = new ReactionEdgeItem(parent_widget);
	    e3->appendNode(ma);
	    e3->appendNode(protein);
	    sn_items.append(e3);
	  }
	  
	  QString str;
	  str.sprintf("Catalytic Complex Degradation %d", ++ct[rxn_id]);
	  sn->setLabel(str);
	  sn->setInitialized(true);
	  sn->setItems(sn_items);
	}
	items.append(sn);

	SubnetworkEdgeItem *e1 = new SubnetworkEdgeItem(parent_widget);
	e1->appendNode(sn, "Protein Complex");
	e1->appendNode(mols[(int)rxns_(i, 1)]);
	items.append(e1);
	
	SubnetworkEdgeItem *e2 = new SubnetworkEdgeItem(parent_widget);
	e2->appendNode(sn, "Catalyst");
	e2->appendNode(mols[(int)rxns_(i, 2)]);
	items.append(e2);

      	SubnetworkEdgeItem *e3 = new SubnetworkEdgeItem(parent_widget);
	e3->appendNode(sn, "Protein A");
	e3->appendNode(mols[(int)rxns_(i, 3)]);
	items.append(e3);
      }      
      break;
    default:
      fprintf(stderr, "Unknown reaction type %d\n", (int)rxns_(i, 0));
      abort();
    }
  }
  
  //pm.setupSparseMatrices();
  







  if(includeCVs) {
    // Introduction of CV to create switch pulse
    {
      str.sprintf("R-1");
      MassActionReactionItem *m = new MassActionReactionItem(parent_widget);
      m->setLabel(str);
      m->setKf(10);
      m->setInitialized(true);
      items.append(m);
      
      SquareWaveControlVariableItem *cv = new SquareWaveControlVariableItem(parent_widget);
      cv->setLabel("CV1");
      if(easyCVs) {
	cv->setValues(0, 1);
	cv->setDuration(1000, 1000);
	cv->setTransitionTime(1, 1);
      } else {
	cv->setValues(0, 0);
	cv->setDuration(1e6, 1);
	cv->setTransitionTime(0, 0);
      }
      cv->setPhaseShift(0);
      cv->setInitialized(true);
      items.append(cv);
      {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	r->appendNode(cv);
	r->appendNode(m);
	items.append(r);
      }
      
      // By list the evolved protein first this CV acts to degrade the protein
      {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	if(frm_ == 0) {
	  r->appendNode(mols[1]);
	  r->appendNode(m);
	} else {
	  r->appendNode(m);
	  r->appendNode(mols[1]);
	}
	items.append(r);
      }
    }
    
    // Introduction of CV to create switch pulse
    {
      str.sprintf("R-2");
      MassActionReactionItem *m = new MassActionReactionItem(parent_widget);
      m->setLabel(str);
      m->setKf(10);
      m->setInitialized(true);
      items.append(m);
      
      SquareWaveControlVariableItem *cv = new SquareWaveControlVariableItem(parent_widget);
      cv->setLabel("CV2");
      if(easyCVs) {
	cv->setValues(0, 1);
	cv->setDuration(2000, 2000);
	cv->setTransitionTime(1, 1);
      } else {
	cv->setValues(0, 0);
	cv->setDuration(1e6, 1);
	cv->setTransitionTime(0, 0);
      }
      cv->setPhaseShift(0);
      cv->setInitialized(true);
      items.append(cv);
      {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	r->appendNode(cv);
	r->appendNode(m);
	items.append(r);
      }

      // By list the evolved protein first this CV acts to degrade the protein
      {
	ReactionEdgeItem *r = new ReactionEdgeItem(parent_widget);
	if(frm_ == 0) {
	  r->appendNode(mols[3]);
	  r->appendNode(m);
	} else {
	  r->appendNode(m);
	  r->appendNode(mols[3]);
	}
	items.append(r);
      }
    }
  }
  }

  return true;
}

int
Individual::nmols() const {
  return mols_.nrows();
}

int
Individual::nrxns() const {
  return rxns_.nrows();
}
