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

#include <cfloat>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qapplication.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "network_evolve.h"
#include "pathway_load.h"
#include "random.h"
#include "version.h"

/*----------------------------------------*/

static char const * const rcsid_ga_ode_opt_cpp = "$Id: network_evolve.cpp,v 1.10 2009/03/14 23:01:03 kent Exp $";

/*------------------------------*/

static PathwayMatrix pem;
static Random rng;

static int pop_size = 10;
static int nrxn = 10;
static int ncvs = 2;
static int nmol = 8;
static int ngen = 100;
static double spress = 0.2;

typedef struct {
  double *kc;
  int *rxns;
  double fitness;
  double e[4];
} org_t;

static org_t *pop = NULL;
static org_t *pop2 = NULL;

static double min_kc = 0.75;
static double max_kc = 2000;

/*------------------------------*/

static void
random_gene(org_t *o) {
  for(int i=0, j; i<nrxn; i++) {
    o->kc[i] = rng.real(min_kc, max_kc);
    o->rxns[i*4+0] = rng.integer(0, nmol-1);
    o->rxns[i*4+1] = rng.integer(0, nmol-1);
    j = rng.integer(-1, nmol-ncvs-1);
    if(j == -1) {
      o->rxns[i*4+2] = -1;
    } else {
      o->rxns[i*4+2] = j + ncvs;
    }
    j = rng.integer(-1, nmol-ncvs-1);
    if(j == -1) {
      o->rxns[i*4+3] = -1;
    } else {
      o->rxns[i*4+3] = j + ncvs;
    }
  }
}

/*------------------------------*/

static void
init_population() {
  if(pop) delete[] pop;
  pop = new org_t[pop_size];
  if(pop2) delete[] pop2;
  pop2 = new org_t[pop_size];
  
  for(int i=0; i<pop_size; i++) {
    pop[i].kc = new double[nrxn];
    pop2[i].kc = new double[nrxn];
    pop[i].rxns = new int[nrxn*4];
    pop2[i].rxns = new int[nrxn*4];
    random_gene(pop+i);
    pop[i].fitness = 0.0;
  }
}

/*------------------------------*/

static void
destroy_population() {
  if(pop) {
    for(int i=0; i<pop_size; i++) {
      delete[] pop[i].kc;
      delete[] pop[i].rxns;
    }
    delete[] pop;
  }
  if(pop2) {
    for(int i=0; i<pop_size; i++) {
      delete[] pop2[i].kc;
      delete[] pop2[i].rxns;
    }
    delete[] pop2;
  }
}

/*------------------------------*/

static void
setupPEM() {
  pem.cvs.resize(ncvs);
  SquareWaveControlVariableItem *cv;
  
  cv = new SquareWaveControlVariableItem(NULL);
  cv->setInitialized(true);
  cv->setLabel(QObject::tr("CV%1").arg(1));
  cv->setDuration(950, 950);
  cv->setTransitionTime(100, 100);
  cv->setValues(0, 1);
  pem.cvs[0] = cv;
  
  cv = new SquareWaveControlVariableItem(NULL);
  cv->setInitialized(true);
  cv->setLabel(QObject::tr("CV%1").arg(2));
  cv->setDuration(425, 425);
  cv->setTransitionTime(100, 100);
  cv->setValues(0, 1);
  pem.cvs[1] = cv;
  
  pem.kc = Matrix(nrxn, 1, 0.0);
  pem.pro = IMatrix(nmol, nrxn, 0);
  pem.re = IMatrix(nmol, nrxn, 0);
  pem.y0 = Matrix(nmol, 1, 0.0);
  pem.atol = Matrix(nmol, 1, 1e-8);
  pem.simulation.setSimulationMethod(0);
  pem.simulation.setAbsoluteTolerance(1e-8);
  pem.simulation.setRelativeTolerance(0.0001);
  pem.simulation.setTimeBegin(0.0);
  pem.simulation.setTimeEnd(2000);
  pem.simulation.setTimeStepSize(0.1);
  pem.simulation.setMaxStep(0.1);
  pem.simulation.setMinStep(0.0);
  pem.simulation.setFitnessFunction(0);
  pem.setCurEnvironment(0);
}
  
/*------------------------------*/

static void
configurePEM(const org_t *org) {
  pem.pro = IMatrix(nmol, nrxn, 0);
  pem.re = IMatrix(nmol, nrxn, 0);

  int i;
  for(i=0; i<nrxn; i++) {
    pem.kc(i, 0) = org->kc[i];
  }
  for(i=0; i<nrxn; i++) {
    if(org->rxns[i*4+0] >= 0) pem.re(org->rxns[i*4+0], i)++;
    if(org->rxns[i*4+1] >= 0) pem.re(org->rxns[i*4+1], i)++;
    if(org->rxns[i*4+2] >= 0) pem.pro(org->rxns[i*4+2], i)++;
    if(org->rxns[i*4+3] >= 0) pem.pro(org->rxns[i*4+3], i)++;
  }
  pem.setupSparseMatrices();
}

/*------------------------------*/

static void
eval_org_fitness(org_t *org) {
  configurePEM(org);
  org->fitness = DBL_MAX;
  int i, j;
  for(j=0; j<4; j++) {
    org->e[j] = DBL_MAX;
  }
  Matrix results;
  if(!run_simulation(pem, results)) {
    return;
  }
  double target[4] = {0.0, 0.0, 1.0, 0.0};
  double min_fitness=DBL_MAX;
  for(i=ncvs-1; i<nmol; i++) {
    double sum = 0.0;
    double max=-DBL_MAX, min=DBL_MAX;
    for(j=0; j<4; j++) {
      double v = results(((j+1)*425+j*100-5)*10, i+1);
      if(v < min) min = v;
      if(v > max) max = v;
    }
    if(max == min) {
      max = min + 1.0;
    }
    for(j=0; j<4; j++) {
      double v = results(((j+1)*425+j*100-5)*10, i+1);
      double d = target[j] - v;
      //double d = target[j] - ((v-min)/(max-min));
      sum += d*d;      
    }
    if(min_fitness > sum) {
      min_fitness = sum;
      for(j=0; j<4; j++) {
	double v = results(((j+1)*425+j*100-5)*10, i+1);
	double d = target[j] - v;
	org->e[j] = d;
      }
    }
#if 0
    printf("%d %f", i+1, sum);
    for(j=0; j<4; j++) {
      double v = results(((j+1)*425+j*100-5)*10, i+1);
      printf("\t%f", v);
    }
    putchar('\n');
#endif
  }
  org->fitness = min_fitness;
}

/*------------------------------*/

static void
eval_population_fitness() {
  for(int i=0; i<pop_size; i++) {
    eval_org_fitness(pop+i);
  }
}

/*------------------------------*/

static int
org_compare(const void *a, const void *b) {
  const org_t *o1=(const org_t *)a;
  const org_t *o2=(const org_t *)b;

  if(o1->fitness > o2->fitness) return 1;
  if(o1->fitness < o2->fitness) return -1;
  return 0;
}

/*------------------------------*/

static void
sort_population(void) {
  qsort(pop, pop_size, sizeof(org_t), org_compare);
}

/*------------------------------*/

static void
cross(const org_t *p1, const org_t *p2, org_t *c) {
  int i;
  for(i=0; i<nrxn; i++) {
    if(rng.boolean()) {
      c->kc[i] = p1->kc[i];
    } else {
      c->kc[i] = p2->kc[i];
    }
  }
  for(i=0; i<nrxn*4; i++) {
    if(rng.boolean()) {
      c->rxns[i] = p1->rxns[i];
    } else {
      c->rxns[i] = p2->rxns[i];
    }
  }
}

/*------------------------------*/

static void
mutate(org_t *c) {
  int i=rng.integer(0, nrxn-1);

  c->kc[i] = rng.real(min_kc, max_kc);

  int j = rng.integer(0, 3);
  if(j < 2) {
    c->rxns[i*4 + j] = rng.integer(0, nmol-1);
  } else {
    // Do not want control variables as products
    c->rxns[i*4 + j] = rng.integer(ncvs, nmol-1);
  }
}

/*------------------------------*/

static void
shift_mutate(org_t *c) {
  int i=rng.integer(0, nrxn-1);
  c->kc[i] *= (1.0 + rng.real1() * 0.10 - 0.05);
  if(c->kc[i] < min_kc) c->kc[i] = min_kc;
  if(c->kc[i] > max_kc) c->kc[i] = max_kc;
}

/*------------------------------*/

static void
helper(org_t *c, void (*func)(org_t*)) {
  org_t torg;

  torg.kc = new double[nrxn];
  torg.rxns = new int[nrxn*4];
  memcpy(torg.kc, c->kc, sizeof(double)*nrxn);
  memcpy(torg.rxns, c->rxns, sizeof(int)*nrxn*4);
  
  func(&torg);
  eval_org_fitness(&torg);

  if(torg.fitness >= c->fitness) {
    double *d = c->kc;
    int *i = c->rxns;
    *c = torg;
    torg.kc = d;
    torg.rxns = i;
  }

  delete[] torg.kc;
  delete[] torg.rxns;
}

/*------------------------------*/

static void
print_org(const org_t *org) {
  printf("%f", org->fitness);
  for(int i=0; i<nrxn; i++) {
    printf("\t%d+%d->%d+%d:%.2f", org->rxns[i*4+0], org->rxns[i*4+1], org->rxns[i*4+2], org->rxns[i*4+3], org->kc[i]);
  }
  putchar('\n');
}

/*------------------------------*/

static void
print_population() {
  for(int i=0; i<pop_size; i++) {
    print_org(pop+i);
  }
}

/*------------------------------*/

static void
print_population_stats() {
  double sum = 0.0;
  double min = DBL_MAX;
  double max = -DBL_MAX;
  for(int i=0; i<pop_size; i++) {
    sum += pop[i].fitness;
    if(min > pop[i].fitness) min = pop[i].fitness;
    if(max < pop[i].fitness) max = pop[i].fitness;
  }
  double avg = sum / pop_size;
  printf("Fitness Avg:%f\tMin:%f\tMax:%f\n", avg, min, max);
}

/*------------------------------*/

static void
prune_population() {
  int i, j, n;
  IMatrix identical(pop_size, 1, 0);
  for(n=0, i=0, j=1; j<pop_size; j++) {
    if(memcmp(pop[i].kc, pop[j].kc, sizeof(double)*nrxn) == 0 &&
       memcmp(pop[i].rxns, pop[j].rxns, sizeof(int)*nrxn*4) == 0) {
      //printf("%d %d identical\n", i, j);
      identical(j, 0) = 1;
      n++;
    } else {
      i=j;
    }
  }
  double per = n / (double) pop_size;
  printf("%f of pop identical\n", per);
  if(per > 0.50) {
    puts("Genetic explosion");
    for(i=0; i<pop_size; i++) {
      if(identical(i, 0)) {
	random_gene(pop+i);
	eval_org_fitness(pop+i);
      }
    }
  }
}

/*------------------------------*/

void
setup() {
  rng.setSeed(-1);
  printf("Random seed: %lu\n", rng.seed());

  setupPEM();
  
  init_population();
  eval_population_fitness();
  sort_population();
}

/*------------------------------*/

void
evolve() {
  int i;

  //print_population();
  //prune_population();
  //sort_population();
  
  for(i=0; i<pop_size; i++) {
    int pind1 = (int)(rng.real1()*spress*pop_size+0.5);
    int pind2 = (int)(rng.real1()*spress*pop_size+0.5);
    org_t *p1=pop+pind1, *p2=pop+pind2, *c=pop2+i;
    
    cross(p1, p2, c);
    eval_org_fitness(c);
  }
  org_t *tmp = pop;
  pop = pop2;
  pop2 = tmp;
  
  for(i=0; i<pop_size; i++) {
    helper(pop+i, mutate);
  }
  
  for(i=0; i<pop_size; i++) {
    helper(pop+i, shift_mutate);
  }

  sort_population();
}

/*------------------------------*/

void
evolve_sim(Simulation &sim) {
  sim = pem.simulation;
}

double
best_individual(QList<PathwayCanvasItem> &items) {
  int i, j;
  
  org_t *org = pop+0;
  configurePEM(org);
  items.clear();
  for(i=0; i<ncvs; i++) {
    items.append(new SquareWaveControlVariableItem(*((SquareWaveControlVariableItem*)pem.cvs[i])));
  }
  for(i=ncvs; i<nmol; i++) {
    DNAMoleculeItem *p = new DNAMoleculeItem(NULL);
    p->setInitialized(true);
    p->setLabel(QObject::tr("M%1").arg(i-ncvs+1));
    p->setInitialConcentration(pem.y0(i, 0));
    items.append(p);
  }
#if 0
  printf("re:\n");
  pem.re.print();
  printf("pro:\n");
  pem.pro.print();
  printf("ire:\n");
  pem.ire.print();
  printf("ipro:\n");
  pem.ipro.print();
#endif
  for(i=0; i<nrxn; i++) {
    int nr = pem.ire(0, i), np = pem.ipro(0, i);
    //printf("%d %d %d\n", i, nr, np);

    if(pem.ipro(0, i) == 0) {
      int i2 = pem.ire(0, i);
      if(items.at(i2)->rtti() != MoleculeItem::RTTI) abort();
      MoleculeItem *p = (MoleculeItem*)items.at(i2);
      p->setDecays(true);
      p->setDecayRate(pem.kc(i, 0));
    } else {
      MassActionReactionItem *p;
      p = new MassActionReactionItem(NULL);
    
      p->setInitialized(true);
      p->setLabel(QObject::tr("R%1").arg(i+1));
      p->setKf(pem.kc(i, 0));
      items.append(p);
      
      for(j=1; j<=nr; j++) {
	int i2 = pem.ire(j, i);
	ReactionEdgeItem *q = new ReactionEdgeItem(NULL);
	q->setCoefficient(pem.re(i2, i));
	q->addPoint((MolRxnBaseItem*)items.at(i2));
	q->addPoint(p);
	items.append(q);
      }
      for(j=1; j<=np; j++) {
	int i2 = pem.ipro(j, i);
	ReactionEdgeItem *q = new ReactionEdgeItem(NULL);
	q->setCoefficient(pem.pro(i2, i));
	q->addPoint(p);
	q->addPoint((MolRxnBaseItem*)items.at(i2));
	items.append(q);
      }
    }
  }
  return org->fitness;
}

/*------------------------------*/

#define min(x, y) (x <= y ? x : y)

void
population_stats(Matrix &stats) {
  double sum = 0.0;
  int i;
  for(i=0; i<pop_size; i++) {
    sum += min(pop[i].fitness, 100);
  }
  stats.resize(1, 7);
  stats(0, 0) = min(pop[0].fitness, 100);
  stats(0, 1) = sum/pop_size;
  stats(0, 2) = min(pop[pop_size-1].fitness, 100);
  for(i=0; i<4; i++) {
    stats(0, i+3) = min(pop[0].e[i], 100);
  }
}

/*------------------------------*/

void
cleanup() {
  destroy_population();
}

/*------------------------------*/

#ifndef GUI

int
main() {
  setup();

  for(int gen=0; gen<=ngen; gen++) {
    printf("Best:\t%d\t", gen); print_org(pop+0);
    printf("Worst:\t%d\t", gen); print_org(pop+pop_size-1);
    printf("%3d/%3d\t", gen, ngen); print_population_stats();

    //print_population();

    {
      QList<PathwayCanvasItem> items;
      double fit = best_individual(items);
      char filename[1024];
      sprintf(filename, "best_%04d.net", gen);
      if(!pathwaySave(filename, appname, appversion,
		      pem.editor, pem.simulation, items)) {
        return false;
      }
    }

    if(gen == ngen) break;
    
    evolve();
  }
  
  cleanup();

  return 0;
}

#endif

/*------------------------------*/

