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

#include <qdatetime.h>
#include <qcolor.h>
#include <qimage.h>
#include <qstringlist.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "evolve_params.h"
#include "individual.h"
#include "math_utils.h"
#include "matrix.h"
#include "pathway_loader.h"
#include "population.h"
#include "random.h"
#include "version.h"

/*----------------------------------------*/

static char const * const rcsid_ga_ode_opt_cpp = "$Id: ga_ode_opt.cpp,v 1.2 2009/03/14 23:00:48 kent Exp $";

/*------------------------------*/

EvolveParams ep;
Random rng;
static int gene_len;
static int eval_time = 0;
static int neval = 0;

//#define COMBO

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

  ep.print();
  fflush(NULL);
  
  rng.setSeed(ep.random_seed);

#if 0
  double x1 = -3.75, x2 = 3.75;
  double y1 = -3.75, y2 = 3.75;
#endif

#if 0
  double x1 = -500.0, x2 = 500.0;
  double y1 = -500.0, y2 = 500.0;
#endif

#if 1
  double x1 = -5.12, x2 = 5.12;
  double y1 = -5.12, y2 = 5.12;
#endif

  gene_len = 2;
  ep.param_ranges.resize(gene_len, 2);
  ep.param_ranges( 0, 0) = x1;   ep.param_ranges( 0, 1) =  x2;
  ep.param_ranges( 1, 0) = y1;   ep.param_ranges( 1, 1) =  y2;


  int w=600, h=600;
  int x, y;
  double sx = (x2 - x1) / w;
  double sy = (y2 - y1) / h;

  Matrix m(w, h);
  double min = DBL_MIN, max = -DBL_MAX;

  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      extern double func(double, double);
      double v = func(x * sx + x1, y * sy + y1);
      m(x, y) = v;
      if(v < min) min = v;
      if(v > max) max = v;
    }
  }

  QImage image_orig(w, h, 32);
  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      double v = (m(x, y) - min) / (max - min);
      image_orig.setPixel(x, y, QColor((int)((1.0-v)*240+0.5), 255, 255, QColor::Hsv).rgb());
    }
  }
  image_orig.save("func.png", "PNG");

#if 0
  Individual in(gene_len);
  QString str;
  int n = 0;
  while(1) {
    in.random();
    printf("Before %06d:\t", n); in.print();
    str.sprintf("before_%06d.net", n);
    in.save(str);
    in.localAreaOptimization();
    printf("After  %06d:\t", n); in.print();
    str.sprintf("after_%06d.net", n);
    in.save(str);
    fflush(NULL);
    n++;
  }
#endif
  
  Population pop(ep.pop_size, gene_len);
  pop.sort();
  
  Individual prev_best, best;
  
  for(int gen=0; gen<=ep.ngen || ep.ngen==-1; gen++) {
    if(ep.population_identity_check_interval > 0 && (gen % ep.population_identity_check_interval) == 0) {
      pop.prune();
      pop.sort();
    }

    if(ep.print_population_interval > 0 && (gen % ep.print_population_interval) == 0) {
      pop.print();
    }
    if(ep.print_population_stats_interval > 0 && (gen % ep.print_population_stats_interval) == 0) {
      printf("Best:\t%d\t", gen);        pop(0).print();
      printf("Avg:\t%d\t", gen);         pop.printAverage();
      printf("Worst:\t%d\t", gen);       pop(ep.pop_size-1).print();
      printf("%3d/%3d\t", gen, ep.ngen); pop.printStats();
    }

    if(ep.save_best_interval > 0 && (gen % ep.save_best_interval) == 0) {
      if(pop(0) != best) {
	char filename[1024];
	sprintf(filename, (const char*)(ep.save_best_prefix + "_%04d.net"), gen);
	pop(0).save(filename);
	best = pop(0);
      }
    }

    {
      char filename[1024];
      sprintf(filename, (const char*)("%04d.png"), gen);
      QImage image = image_orig.copy();
      for(i=0; i<ep.pop_size; i++) {
	int x = (pop(i)(0) - x1) / (x2 - x1) * w + 1;
	int y = (pop(i)(1) - y1) / (y2 - y1) * h + 1;
#define cl(x) (x < 0 ? 0 : x > 599 ? 599 : x)
	image.setPixel(cl(x-1), cl(y-1), QColor("black").rgb());
	image.setPixel(cl(x-1), cl(y),   QColor("black").rgb());
	image.setPixel(cl(x-1), cl(y+1), QColor("black").rgb());
	image.setPixel(cl(x),   cl(y-1), QColor("black").rgb());
	image.setPixel(cl(x),   cl(y),   QColor("black").rgb());
	image.setPixel(cl(x),   cl(y+1), QColor("black").rgb());
	image.setPixel(cl(x+1), cl(y-1), QColor("black").rgb());
	image.setPixel(cl(x+1), cl(y),   QColor("black").rgb());
	image.setPixel(cl(x+1), cl(y+1), QColor("black").rgb());
      }
      image.save(filename, "PNG");
    }
    
    //printf("Calculation rate: %d %d %.2f evals/sec\n", neval, eval_time, neval / (eval_time / 1000.0));

    if(gen == ep.ngen) break;
   
    if(ep.elitism) {
      prev_best = pop(0);
    }

    pop.crossover();
    
#if 1
    for(i=0; i<ep.pop_size; i++) {
      pop(i).helpfulMutate();
    }

    for(i=0; i<ep.pop_size; i++) {
      pop(i).helpfulShiftMutate();
    }
#endif
    
    if(ep.replace_failures) {
      for(i=0; i<ep.pop_size; i++) {
	int j=0;
	while(pop(i).fitness() == DBL_MAX && j < 50) {
	  pop(i).random();
	  printf("%d %d\n", i, j);
	  j++;
	}
      }
    }

    pop.sort();
    if(ep.elitism) {
      if(pop(0) < prev_best) {
	pop(ep.pop_size-1) = prev_best;
	pop.sort();
      }
    }

    if(ep.simplex_size > 0) {
      for(int i=0; i<ep.simplex_size; i++) {
	Individual before = pop(i);
	double f1 = pop(i).fitness();
	pop(i).localAreaOptimization();
	double f2 = pop(i).fitness();
	printf("pop(%d) before simplex:\t%d\t", i, gen); before.print();
	printf("pop(%d) after simplex:\t%d\t", i, gen); pop(i).print();
      }
      if(ep.simplex_size > 1) {
	pop.sort();
      }
    }
  }

  return 0;
}

/*------------------------------*/
