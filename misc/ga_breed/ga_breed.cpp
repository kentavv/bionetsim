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
#include <string.h>
#include <time.h>
#include <math.h>

#include "cache.h"
#include "calculate.h"
#include "log.h"
#include "math_utils.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "pathway_save.h"
#include "evolve_params.h"
#include "random.h"
#include "version.h"

/*----------------------------------------*/

static char const * const rcsid_ga_ode_opt_cpp = "$Id: ga_breed.cpp,v 1.17 2009/03/14 23:48:49 kent Exp $";

/*------------------------------*/

static EvolveParams ep;
static Random rng;

//static PathwayMatrix pem;
//static Matrix alleles;
//static int nalleles;
//static int nloci = 0;

//static int pop_size = 100;
//static int ngen = 100;
//static double spress = 0.2;

typedef struct {
  int *gene;
  double fitness;
} org_t;

static org_t *pop = NULL;
static org_t *pop2 = NULL;

/*------------------------------*/

int
random_locus() {
  return rng.integer(0, ep.param_ranges.ncolumns()-1);
}

/*------------------------------*/

void
random_gene(org_t *o) {
  for(int j=0; j<ep.param_ranges.nrows()*2; j++) {
    o->gene[j] = random_locus();
  }
}

/*------------------------------*/

void
init_population() {
  if(pop)  delete[] pop;
  if(pop2) delete[] pop2;
  pop  = new org_t[ep.pop_size];
  pop2 = new org_t[ep.pop_size];
  
  for(int i=0; i<ep.pop_size; i++) {
    pop[i].gene = new int[ep.param_ranges.nrows()*2];
    pop2[i].gene = new int[ep.param_ranges.nrows()*2];
    random_gene(pop+i);
    pop[i].fitness = 0.0;
  }
}

/*------------------------------*/

void
destroy_population() {
  if(pop) {
    for(int i=0; i<ep.pop_size; i++) {
      delete[] pop[i].gene;
    }
    delete[] pop;
  }
  if(pop2) {
    for(int i=0; i<ep.pop_size; i++) {
      delete[] pop2[i].gene;
    }
    delete[] pop2;
  }
}

/*------------------------------*/

void
eval_org_fitness(org_t *org) {
  //const char *filename = "";
  int env = 0;
  double pheno;
  // Deactivate cache_lookup
  if(cache_lookup((const char*)ep.network_filename, env, org->gene, ep.param_ranges.nrows(), &pheno)) {
  //printf("CRes %f\n", pheno);
  } else {
    for(int i=0; i<ep.param_ranges.nrows(); i++) {
      ep.pem.kc(i, 0) = (ep.param_ranges(i, org->gene[i]) +
			 ep.param_ranges(i, org->gene[i+ep.param_ranges.nrows()])) / 2.0;
    }
    ep.pem.setCurEnvironment(env);
    if(!ep.fitness.calculate(ep.pem, pheno)) {
      pheno = -1.0;
    }
   cache_store((const char*)ep.network_filename, env, org->gene, ep.param_ranges.nrows(), pheno);
    //printf("Res: %f\n", pheno);
    }
  double noise = rng.normal(0.0, ep.noise);
  org->fitness =pheno + noise ;
}

/*------------------------------*/

void
eval_population_fitness() {
  for(int i=0; i<ep.pop_size; i++) {
    eval_org_fitness(pop+i);
  }
}

/*------------------------------*/

int
org_compare(const void *a, const void *b) {
  const org_t *o1=(const org_t *)a;
  const org_t *o2=(const org_t *)b;
  
 if (ep.minimax == -1){ 
    if(o1->fitness < o2->fitness) return -1;
    if(o1->fitness > o2->fitness) return 1;
  }else {
    if(o1->fitness < o2->fitness) return 1;
    if(o1->fitness > o2->fitness) return -1;
  }
  return 0;
}

/*------------------------------*/

void
sort_population(void) {
  qsort(pop, ep.pop_size, sizeof(org_t), org_compare);
}

/*------------------------------*/

void
cross(const org_t *p1, const org_t *p2, org_t *c) {
  for(int i=0; i<ep.param_ranges.nrows(); i++) {
    if(rng.boolean()) {
      c->gene[i] = p1->gene[i];
    } else {
      c->gene[i] = p1->gene[i+ep.param_ranges.nrows()];
    }
    if(rng.boolean()) {
      c->gene[i+ep.param_ranges.nrows()] = p2->gene[i];
    } else {
      c->gene[i+ep.param_ranges.nrows()] = p2->gene[i+ep.param_ranges.nrows()];
    }
  }
}

/*------------------------------*/

void
print_org(const org_t *org) {
  printf("%f",  org->fitness);
  for(int i=0; i<ep.param_ranges.nrows(); i++) {
    printf("  %d,%d", org->gene[i], org->gene[i+ep.param_ranges.nrows()]);
  }
  putchar('\n');
}

/*------------------------------*/

void
print_population(int gen) {
  printf("Start population %d\n", gen);
  for(int i=0; i<ep.pop_size; i++) {
    print_org(pop+i);
  }
  printf("End population %d\n", gen);
}

/*------------------------------*/

void
print_population_average_qformat() {
  Matrix sums(ep.param_ranges.nrows()+1, 1);
  int i, j;
  
  for(j=0; j<ep.param_ranges.nrows()+1; j++) {
    sums(j, 0) = 0.0;
  }
  
  for(i=0; i<ep.pop_size; i++) {
    sums(0, 0) += pop[i].fitness;
    for(j=0; j<ep.param_ranges.nrows()*2; j++) {
      if(pop[i].gene[j] == 0) {
	sums((j%ep.param_ranges.nrows())+1, 0) += 1.0;
      }
    }
  }

  printf("%f", sums(0, 0)/ep.pop_size);
  for(j=0; j<ep.param_ranges.nrows(); j++) {
    printf("\t%.2f", sums(j+1, 0)/(2*ep.pop_size));
  }
  putchar('\n');
}

/*------------------------------*/

void
print_population_average() {
  Matrix sums(ep.param_ranges.nrows()+1, 1);
  int i, j;
  
  for(j=0; j<ep.param_ranges.nrows()+1; j++) {
    sums(j, 0) = 0.0;
  }
  
  for(i=0; i<ep.pop_size; i++) {
    sums(0, 0) += pop[i].fitness;
    for(j=0; j<ep.param_ranges.nrows()*2; j++) {
      if(pop[i].gene[j] == 0) {
	sums((j%ep.param_ranges.nrows())+1, 0) += 1.0;
      }
    }
  }

  printf("%f", sums(0, 0)/ep.pop_size);
  for(j=0; j<ep.param_ranges.nrows(); j++) {
    printf("\t%.2f", sums(j+1, 0)/(2*ep.pop_size));
  }
  putchar('\n');
}

/*------------------------------*/

void
print_population_stats() {
  double sum = 0.0;
  double min = 1000;
  double max = -1000;
  for(int i=0; i<ep.pop_size; i++) {
    sum += pop[i].fitness;
    if(min > pop[i].fitness) min = pop[i].fitness;
    if(max < pop[i].fitness) max = pop[i].fitness;
  }
  double avg = sum / ep.pop_size;
  printf("Fitness Min:%f\tAvg:%f\tMax:%f\n", min, avg, max);
}

/*------------------------------*/

bool
save_best(int gen) {
  int i;
  int env = 0;
  org_t *org = pop+0;
  for(i=0; i<ep.param_ranges.nrows(); i++) {
    ep.pem.kc(i, 0) = (ep.param_ranges(i, org->gene[i]) +
		       ep.param_ranges(i, org->gene[i+ep.param_ranges.nrows()])) / 2.0;
  }
  ep.pem.applyKCToItems();
  ep.pem.setCurEnvironment(env);
  ep.pem.simulation.setCurEnvironment(env);

  char filename[1024];
  sprintf(filename, (const char*)(ep.save_best_prefix + "_%04d.net"), gen);
  return pathwaySave(filename, appname, appversion,
		     ep.pem.editor, ep.fitness,
		     ep.pem.simulation, ep.pem.items);
}

/*------------------------------*/

int
main(int argc, char *argv[]) {
  int i;

  if(argc != 2) {
    fprintf(stderr, "%s: <parameter file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  if(!ep.load(filename)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  //ep.print();
  fflush(NULL);

  rng.setSeed(ep.random_seed);

  //nloci = pem.kc.nrows();
  // nalleles = alleles.ncolumns();

  if(ep.param_ranges.ncolumns() != 2) {
    fprintf(stderr, "Numder of alleles != 2\n");
    abort();
  }
  
  setLogFP(stdout, NULL, NULL, NULL);
  init_population();

  eval_population_fitness();

  for(int gen=0; gen<=ep.ngen; gen++) {
    sort_population();

    printf("%d\t", gen); print_population_average_qformat();
    
    if(ep.print_population_interval > 0 && (gen % ep.print_population_interval) == 0) {
      print_population(gen);
    }
    if(ep.print_population_stats_interval > 0 && (gen % ep.print_population_stats_interval) == 0) {
      printf("Best:\t%d\t", gen); print_org(pop+0);
      printf("Avg:\t%d\t", gen); print_population_average();
      printf("Worst:\t%d\t", gen); print_org(pop+ep.pop_size-1);
      printf("%03d/%03d\t", gen, ep.ngen); print_population_stats();
    }
    if(ep.save_best_interval > 0 && (gen % ep.save_best_interval) == 0) {
      save_best(gen);
    }

    if(gen == ep.ngen) break;

    for(i=0; i<ep.pop_size; i++) {
      int pind1 = rng.integer(0, PNE::roundToInt(ep.spress*(ep.pop_size-1)));
      int pind2 = rng.integer(0, PNE::roundToInt(ep.spress*(ep.pop_size-1)));
      org_t *p1=pop+pind1, *p2=pop+pind2, *c=pop2+i;

      cross(p1, p2, c);
      eval_org_fitness(c);
    }

    org_t *tmp = pop;
    pop = pop2;
    pop2 = tmp;

    fflush(stdout);
  }

  destroy_population();
  
  cache_all_dump(ep.param_ranges.nrows());

  return 0;
}

/*------------------------------*/

