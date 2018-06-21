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

#include <qstring.h>

#include "evolve_params.h"
#include "matrix.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "random.h"

EvolveParams::EvolveParams() {
}

bool
EvolveParams::load(const QString &filename) {
  FILE *file = fopen(filename, "r");
  if(!file) return false;

  int line_num = 0;
  char line[1024];
  while(fgets(line, 1024, file)) {
    line_num++;
    QString str = line;
    int p = str.find('#');
    if(p >= 0) {
      str.truncate(p);
    }
    str.stripWhiteSpace();
    QString label, value;
    label = str.section(':', 0, 0); label = label.stripWhiteSpace();
    value = str.section(':', 1);    value = value.stripWhiteSpace();
    if(label.isEmpty()) continue;
    else if(label == "Network file") {
      network_filename = value;
      printf("loading pwy");
      if(!pathwayLoad(value, pem, fitness)) {
	fprintf(stderr, "Unable to load %s\n", (const char*)value);
	fclose(file);
	return false;
      }      
    } else if(label == "Population size") {
      pop_size = value.toInt();
    } else if(label == "Number of generations") {
      ngen = value.toInt();
    } else if(label == "Selection pressure") {
      spress = value.toDouble();
    } else if(label == "Initial allele frequency"){
      initial_af = value.toDouble();
    } else if(label == "Environment"){
      env = value.toInt();} 
    else if(label == "Maximize"){
      minimax = value.toInt();
    } else if (label == "Noise"){
      noise = value.toDouble();
    } else if (label == "Target genotype"){
      targeno = value;
    } else if(label == "Crossover type") {
      value = value.lower();
      if(value == "npt") {
	crossover = NPt;
      } else if(value == "twopt") {
	crossover = TwoPt;
      } else {
	fprintf(stderr, "Parse error on line %d\n", line_num);
	fclose(file);
	return false;
      }
    } else if(label == "Shift mutation range") {
      shift_mutation_range = value.toDouble();
    } else if(label == "Save best interval") {
      save_best_interval = value.toInt();
    } else if(label == "Save best prefix") {
      save_best_prefix = value;
    } else if(label == "Print population interval") {
      print_population_interval = value.toInt();
    } else if(label == "Print population stats interval") {
      print_population_stats_interval = value.toInt();
    } else if(label == "Population identity check interval") {
      population_identity_check_interval = value.toInt();
    } else if(label == "Population identity limit") {
      population_identity_limit = value.toDouble();
    } else if(label == "Random seed") {
      random_seed = value.toLong();
    } else if(label == "Integer parameters") {
      value = value.lower();
      if(value == "true") {
	integer_parameters = true;
      } else if(value == "false") {
	integer_parameters = false;
      } else {
	fprintf(stderr, "Parse error on line %d\n", line_num);
	fclose(file);
	return false;
      }
    } else if(label == "Parameter ranges") {
      break;
    } else {
      fprintf(stderr, "Parse error on line %d\n", line_num);
      fclose(file);
      return false;
    }
  }

  param_ranges = Matrix(pem.kc.nrows(), 2, 0.0);
  while(fgets(line, 1024, file)) {
    line_num++;
    QString str = line;
    int p = str.find('#');
    if(p >= 0) {
      str.truncate(p);
    }
    str = str.stripWhiteSpace();
    if(str.isEmpty()) continue;
    QString label, v1, v2, v3, v4;
    label = str.section('\t', 0, 0); label = label.stripWhiteSpace();
    v1    = str.section('\t', 1, 1);
    v2    = str.section('\t', 2, 2);
    v3    = str.section('\t', 3, 3);
    v4    = str.section('\t', 4, 4);

    if(!applyConstraints(label,
			 v1.toDouble(), v2.toDouble(),
			 v3.toDouble(), v4.toDouble())) {
      fprintf(stderr, "Unable to locate reaction %s\n", (const char*)label);
      fclose(file);
      return false;
    }    
  }
  
  fclose(file);

  return true;
}

void
EvolveParams::print(FILE *fp) {
  fprintf(fp, "Network file: %s\n", (const char*)network_filename);
  fprintf(fp, "Population size: %d\n", pop_size);
  fprintf(fp, "Number of generations: %d\n", ngen);
  fprintf(fp, "Selection pressure: %f\n", spress);
  fprintf(fp, "Initial allele frequency: %f\n", initial_af);
  fprintf(fp, "Environment: %d\n", env);
  fprintf(fp, "Target genotype: %s\n", (const char*)targeno);
  if(crossover == NPt) {
    fprintf(fp, "Crossover type: NPt\n");
  } else if(crossover == TwoPt) {
    fprintf(fp, "Crossover type: TwoPt\n");
  } else {
    fprintf(fp, "Crossover type: ???");
  }
  fprintf(fp, "Shift mutation range: %f\n", shift_mutation_range);
  fprintf(fp, "Save best interval: %d\n", save_best_interval);
  fprintf(fp, "Save best prefix: %s\n", (const char*)save_best_prefix);
  fprintf(fp, "Print population interval: %d\n", print_population_interval);
  fprintf(fp, "Print population stats interval: %d\n", print_population_stats_interval);
  fprintf(fp, "Population identity check interval: %d\n", population_identity_check_interval);
  fprintf(fp, "Population identity limit: %f\n", population_identity_limit);
  fprintf(fp, "Random seed: %lu\n", random_seed);
  if(integer_parameters) {
    fprintf(fp, "Integer parameters: true\n");
  } else {
    fprintf(fp, "Integer parameters: false\n");
  }
  fprintf(fp, "Parameter ranges:\n");

  int nrxn = 0;
  PathwayCanvasItem *item;
  for(item=pem.items.first(); item; item=pem.items.next()) {
    if(isMolecule(item)) {
      MoleculeItem *mol = (MoleculeItem*)item;
      if(mol->decays()) {
	fprintf(fp, "%s\t%f\t%f\t\t\t# %-60s\t%f\n",
		(const char*)mol->label(), param_ranges(nrxn, 0), param_ranges(nrxn, 1),
		(const char*)(mol->label() + " --> 0"), mol->decayRate());
	nrxn++;
      }
    } else if(isReaction(item)) {
      MassActionReactionItem *rxn = (MassActionReactionItem*)item;
      fprintf(fp, "%s\t%f\t%f\t", (const char*)rxn->label(), param_ranges(nrxn, 0), param_ranges(nrxn, 1));
      nrxn++;
      if(rxn->reversible()) {
	fprintf(fp, "%f\t%f\t", param_ranges(nrxn, 0), param_ranges(nrxn, 1));
	nrxn++;
      } else {
	fprintf(fp, "\t\t");
      }
      fprintf(fp, "# %-60s\tKf:%f", (const char*)rxn->equation(), rxn->Kf());
      if(rxn->reversible()) {
	fprintf(fp, "\tKr:%f", rxn->Kr());
      }
      fputc('\n', fp);
    }
  }
}

bool
EvolveParams::applyConstraints(const QString &label,
			       double v1, double v2, double v3, double v4) {
  int nrxn = 0;
  return applyConstraints(pem.items, label, v1, v2, v3, v4, "", nrxn);
}

bool
EvolveParams::applyConstraints(const QList<PathwayCanvasItem> &items,
			       const QString &label,
			       double v1, double v2, double v3, double v4,
			       const QString &path, int &nrxn) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *item;
  while((item = it.current())) {
    if(isMolecule(item)) {
      MoleculeItem *mol = (MoleculeItem*)item;
      //printf("%s    %s\n", (const char*)(path + mol->label()), (const char*)label);
      if(mol->decays()) {
	if(path + mol->label() == label) {
	  param_ranges(nrxn, 0) = v1;
	  param_ranges(nrxn, 1) = v2;
	  break;
	}
	nrxn++;
      }
    } else if(isReaction(item)) {
      ReactionItem *rxn = (ReactionItem*)item;
      //printf("%s    %s\n", (const char*)(path + rxn->label()), (const char*)label);
      if(path + rxn->label() == label) {
	param_ranges(nrxn, 0) = v1;
	param_ranges(nrxn, 1) = v2;
	if(rxn->reversible()) {
	  param_ranges(nrxn+1, 0) = v3;
	  param_ranges(nrxn+1, 1) = v4;
	}
	break;  // This break messes things up if there are multiple reactions with the same label
      }
      nrxn++;
      if(((ReactionItem*)item)->reversible()) {
	nrxn++;
      }
    } else if(isSubnetwork(item)) {
      SubnetworkItem *sn = (SubnetworkItem*)item;
      if(applyConstraints(sn->rawItems(), label, v1, v2, v3, v4,
			  path + sn->label() + "::", nrxn)) break;
    }
    ++it;
  }

  return item != NULL;
}
