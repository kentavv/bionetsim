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
#ifdef WIN32
#include <float.h>
#else
#include <values.h>
#endif
#include <ctype.h>

#include <qlist.h>
#include <qvaluevector.h>

#include "calculate.h"
#include "individual.h"
#include "log.h"
#include "matrix.h"
#include "pathway_canvas_items.h"
#include "pathway_matrix.h"
#include "pathway_save.h"
#include "random.h"
#include "version.h"

// WARNING: There are some small numerical differences between the simulation of files saved with
// save() and those saved with save2().  The difference is the result of a rearrangement
// of the reactions.  To remove this difference edit the file saved by save2() and move
// the top level reactions (the control variable reactions) ahead of the subnetworks section.

const int pop_size = 1000;
const int nworld = 100;
const int ngen = 30;
/*const*/ int nmut = 2;
//Random rng(1061249409);
//Random rng(100);
Random rng;
//Random rng(1948170051);
PathwayMatrix pm;
//#define STOCHASTIC
int tid_;
int frm_;

bool final = false;

QWidget *parent_widget = NULL;

class Motifs {
public:

protected:
  Motifs();
};

void
sort_population(Individual *pop, int n) {
  for(int i=0; i<n-1; i++) {
    for(int j=i+1; j<n; j++) {
      Individual tmp;
      if(pop[i].toggle() < pop[j].toggle() ||
	 (pop[i].toggle() == pop[j].toggle() &&
	  pop[i].fitness() > pop[j].fitness())) {
	tmp = pop[i];
	pop[i] = pop[j];
	pop[j] = tmp;
      }
    }
  }
}

int
main(int argc, char *argv[]) {
  if(argc != 3) {
    exit(EXIT_FAILURE);
  }
  tid_ = atoi(argv[1]);
  frm_ = atoi(argv[2]);
  printf("TID: %d\n", tid_);
  printf("FRM: %d\n", frm_);
  
  appname = "FH Evolve";
  appversion = "0.01 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);
  printf("Using random seed: %ld\n", rng.seed());
  
  for(int world=0; world<nworld; world++) {
    Individual *pop = new Individual[pop_size * 2];

  pm.simulation.setTimeBegin(0.0);
  pm.simulation.setTimeEnd(4000.0);
  pm.simulation.setTimeStepSize(0.1);

#ifndef STOCHASTIC
  pm.simulation.setSimulationMethod("ODE");
  pm.simulation.setSimulateToSteadyState(false);
  pm.simulation.setAbsoluteTolerance(1e-8);
  pm.simulation.setRelativeTolerance(1e-4);
  pm.simulation.setMinStep(0.0);
  pm.simulation.setMaxStep(1.0);

  pm.simulation.setSimulateToSteadyState(true);
  pm.simulation.setSteadyStateTolerance(1e-6);
#else
  pm.simulation.setSimulationMethod("Stochastic");
  pm.simulation.setSingleTrajectory(false);
  pm.simulation.setNumReplicates(20);
#endif
  
  pm.progress = NULL;
  
  sort_population(pop, pop_size*2);
  bool first_success = false;
  
  for(int gen=0; gen<=ngen; gen++) {
    if(gen > 0) {
      for(int i=0; i<pop_size; i++) {
	pop[pop_size + i] = pop[i];
	pop[pop_size + i].mutate();
      }
      sort_population(pop, pop_size*2);
    }
    for(int i=0; i<GenoDYN::min(pop_size,20); i++) {
      printf("%d:%d:%d: %d %d %f %d\n", world, gen, i, pop[i].nmols(), pop[i].nrxns(), pop[i].fitness(), pop[i].toggle());
#if 0
      if(pop[i].toggle()==3) {
	QString str;
	str.sprintf("gent%05d_%02d.net", gen, i);
	pop[i].save2(str);
	
	str.sprintf("gent%05d_%02d_orig.net", gen, i);
	pop[i].save(str);
	pop[i] = Individual();
      }
#endif
#if 0
      if(pop[i].toggle() > 0) {
	QString str;
	str.sprintf("world%05d_gen%05d_%02d.net", world, gen, i);
	pop[i].save2(str);
      }
#endif
    }
#if 1
    for(int i=0; i<GenoDYN::min(5, pop_size); i++) {
      QString str;
      str.sprintf("world%05d_gen%05d_%02d.net", world, gen, i);
      pop[i].save2(str);
      
      //      str.sprintf("gen%05d_%02d_orig.net", gen, i);
      //      pop[i].save(str);
    }
#endif
    fflush(NULL);

    // On first identification of a successful topology seed the population with it
    if(!first_success) {
      for(int i=0; i<pop_size; i++) {
	if(pop[i].toggle() == 3) {
	  for(int j=0; j<pop_size; j++) {
	    pop[j] = pop[i];
	  }
	  first_success = true;
	  break;
	}
      }
    }
  }

#if 0
  final = true;
  pop[0].fitness();

  //final = false;
  printf("prune %d %f %d\n", pop[0].rxns_.nrows(), pop[0].fitness(), pop[0].toggle());
  for(int i=pop[0].rxns_.nrows()-1; i >= 4; i--) {
    pop[0].rxns_(i, 0) = 0;
    pop[0].current_ = false;
    printf("prune %d %f %d\n", i, pop[0].fitness(), pop[0].toggle());
    QString str;
    str.sprintf("world%05d_prune%05d.net", world, i);
    pop[0].save(str);
  }
#endif
  delete[] pop;
  }
  
  return 0;
}
