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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <qstring.h>

#include "evolve_params.h"
#include "matrix.h"
#include "pathway_canvas_utils.h"
#include "pathway_load.h"
#include "random.h"

EvolveParams::EvolveParams() 
  : time_limit_type(Iteration),
    time_limit(1000),
    cs_mode(false),
    i_am_master(false)
{}

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
      if(!pathwayLoad(value, pem, fitness)) {
	fprintf(stderr, "Unable to load %s\n", (const char*)value);
	fclose(file);
	return false;
      }      
    } else if(label == "Maximize fitness function") {
      maximize = (value.lower() == "true");
    } else if(label == "Elitism") {
      elitism = (value.lower() == "true");
    } else if(label == "Replace failures") {
      replace_failures = (value.lower() == "true");
    } else if(label == "Simplex size") {
      simplex_size = value.toInt();
    } else if(label == "Time limit type") {
      value = value.lower();
      if(value == "none") {
	time_limit_type = NoTimeLimit;
      } else if(value == "timer") {
	time_limit_type = Timer;
      } else if(value == "iteration") {
	time_limit_type = Iteration;
      }
    } else if(label == "Time limit") {
       time_limit = value.toInt();
    } else if(label == "Population size") {
      pop_size = value.toInt();
    } else if(label == "Client-server mode") {
      //cs_mode = (value.lower() == "true");
    } else if(label == "Work-unit size") {
      wu_size = value.toInt();
    } else if(label == "Master Address") {
      //master_ip.s_addr = inet_addr((const char*)value);
    } else if(label == "Number of generations") {
      ngen = value.toInt();
    } else if(label == "Selection pressure") {
      spress = value.toDouble();
    } else if(label == "Crossover type") {
      value = value.lower();
      if(value == "none") {
	crossover = None;
      } else if(value == "npt") {
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

  int nparams = 0;
  kc2param_map = IMatrix(pem.kc.nrows(), 1, -1);
  while(fgets(line, 1024, file)) {
    line_num++;
    QString str = line;
    {
      str = str.stripWhiteSpace();
      if(str == "# Parameters") {
	break;
      }
    }
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

    int a = v1.toInt(), b = v2.toInt();

    if(!buildMap(label, a, b)) {
      fprintf(stderr, "Unable to locate reaction %s\n", (const char*)label);
      fclose(file);
      return false;
    }

    if(nparams <= a) nparams = a+1;
    if(nparams <= b) nparams = b+1;
  }
  //print(stdout);
  //abort();
  param_ranges = Matrix(nparams, 2, 0.0);
  int kc_ind = 0, param_ind = 0;
  kc2param_map.print();
  //abort();
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

    bool var;
    double a = v1.toDouble();
    double b = v2.toDouble(&var);

    if(var) {
      param_ranges(param_ind, 0) = a;
      param_ranges(param_ind, 1) = b;
      param_ind++;
    } else {
      //kc2param_map(kc_ind, 0) = -1;
      for(int i=0; i<kc2param_map.nrows(); i++) {
	if(kc2param_map(i, 0) == kc_ind) {
	  kc2param_map(i, 0) = -1;
	  pem.kc(i, 0) = a;
	}
      }
    }
    kc_ind++;
  }
  puts("=======");
  kc2param_map.print();
  puts("-------");
  //abort();

  if(param_ind < param_ranges.nrows()) {
    printf("Resizing param_ranges %d rows to %d rows\n", param_ranges.nrows(), param_ind);
    Matrix pr2 = Matrix(param_ind, 2, 0.0);
    for(int i=0; i<param_ind; i++) {
      pr2(i, 0) = param_ranges(i, 0);
      pr2(i, 1) = param_ranges(i, 1);
    }
    for(int i=0, j=0; j<kc2param_map.nrows(); j++) {
      if(kc2param_map(j, 0) != -1) {
	kc2param_map(j, 0) -= (j - i);
	printf("%d %d %d\n", kc2param_map(j, 0), j, i);
	i++;
      }
    }
    param_ranges = pr2;
  }

  printf("pr.nrows(): %d\n", param_ranges.nrows());
  param_ranges.print();
  
  fclose(file);

  if(simplex_size > pop_size) {
    simplex_size = pop_size;
  }
  
  return true;
}

void
EvolveParams::print(FILE *fp) {
  fprintf(fp, "Network file: %s\n", (const char*)network_filename);
  fprintf(fp, "Population size: %d\n", pop_size);
  fprintf(fp, "Client-server mode: %s\n", cs_mode ? "true" : "false");
  fprintf(fp, "Work-unit size: %d\n", wu_size);
  fprintf(fp, "Master: %s\n", inet_ntoa(master_ip));
  fprintf(fp, "Number of generations: %d\n", ngen);
  fprintf(fp, "Selection pressure: %f\n", spress);
  if(crossover == None) {
    fprintf(fp, "Crossover type: None\n");
  } else if(crossover == NPt) {
    fprintf(fp, "Crossover type: NPt\n");
  } else if(crossover == TwoPt) {
    fprintf(fp, "Crossover type: TwoPt\n");
  } else {
    fprintf(fp, "Crossover type: ???\n");
  }
  fprintf(fp, "Maximize fitness function: %s\n", maximize ? "true" : "false");
  fprintf(fp, "Elitism: %s\n", elitism ? "true" : "false");
  fprintf(fp, "Replace failures: %s\n", replace_failures ? "true" : "false");
  fprintf(fp, "Simplex size: %d\n", simplex_size);
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
	int ind = kc2param_map(nrxn, 0);
	if(ind == -1) {
	  fprintf(fp, "%s\t%d\t%f\t\t\t\t# %-60s\t%f\n",
		  (const char*)mol->label(), ind, pem.kc(nrxn, 0),
		  (const char*)(mol->label() + " --> 0"), mol->decayRate());
	} else {
	  fprintf(fp, "%s\t%d\t%f\t%f\t\t\t# %-60s\t%f\n",
		  (const char*)mol->label(), ind, param_ranges(ind, 0), param_ranges(ind, 1),
		  (const char*)(mol->label() + " --> 0"), mol->decayRate());
	}
	nrxn++;
      }
    } else if(isReaction(item)) {
      MassActionReactionItem *rxn = (MassActionReactionItem*)item;
      int ind = kc2param_map(nrxn, 0);
      fprintf(fp, "%s\t%d\t", (const char*)rxn->label(), ind);
      if(ind == -1) {
	fprintf(fp, "%f\t\t", pem.kc(nrxn, 0));
      } else {
	fprintf(fp, "%f\t%f\t", param_ranges(ind, 0), param_ranges(ind, 1));
      }
      nrxn++;
      if(rxn->reversible()) {
	int ind_r = kc2param_map(nrxn, 0);
	fprintf(fp, "%d\t", ind_r);
	if(ind_r == -1) {
	  fprintf(fp, "%f\t\t", pem.kc(nrxn, 0));
	} else {
	  fprintf(fp, "%f\t%f\t", param_ranges(ind_r, 0), param_ranges(ind_r, 1));
	}
	nrxn++;
      } else {
	fprintf(fp, "\t\t\t");
      }
      fprintf(fp, "# %-60s\tKf:%f", (const char*)rxn->equation(), rxn->Kf());
      if(rxn->reversible()) {
	fprintf(fp, "\tKr:%f", rxn->Kr());
      }
      fputc('\n', fp);
    }
  }
  puts("kc2param_map:");
  kc2param_map.print();
  puts("param_ranges:");
  param_ranges.print();
  puts("pem.kc:");
  pem.kc.print();
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
      printf("%s    %s\n", (const char*)(path + mol->label()), (const char*)label);
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
      printf("%s    %s\n", (const char*)(path + rxn->label()), (const char*)label);
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

int
EvolveParams::getReactionIndex(const QString &label, bool rflag) {
  int nrxn = 0;
  return getReactionIndex(pem.items, label, rflag, "", nrxn);
}

int
EvolveParams::getReactionIndex(const QList<PathwayCanvasItem> &items,
			       const QString &label, bool rflag,
			       const QString &path, int &nrxn) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *item;
  while((item = it.current())) {
    if(isMolecule(item)) {
      MoleculeItem *mol = (MoleculeItem*)item;
      if(mol->decays()) {
	if(path + mol->label() == label) {
	  return nrxn;
	}
	nrxn++;
      }
    } else if(isReaction(item)) {
      ReactionItem *rxn = (ReactionItem*)item;
      printf("%s    %s\n", (const char*)(path + rxn->label()), (const char*)label);
      if(path + rxn->label() == label) {
	if(!rflag) {
	  return nrxn;
	}
	if(rxn->reversible() && rflag) {
	  return nrxn;
	}
	break;  // This break messes things up if there are multiple reactions with the same label
      }
      nrxn++;
      if(((ReactionItem*)item)->reversible()) {
	nrxn++;
      }
    } else if(isSubnetwork(item)) {
      SubnetworkItem *sn = (SubnetworkItem*)item;
      if(getReactionIndex(sn->rawItems(), label, rflag,
			  path + sn->label() + "::", nrxn) != -1) break;
    }
    ++it;
  }

  return -1;
}

bool
EvolveParams::buildMap(const QString &label,
		       int v1, int v2) {
  int nrxn = 0;
  return buildMap(pem.items, label, v1, v2, "", nrxn);
}

bool
EvolveParams::buildMap(const QList<PathwayCanvasItem> &items,
		       const QString &label,
		       int v1, int v2,
		       const QString &path, int &nrxn) {
  QListIterator<PathwayCanvasItem> it(items);
  PathwayCanvasItem *item;
  while((item = it.current())) {
    if(isMolecule(item)) {
      MoleculeItem *mol = (MoleculeItem*)item;
      //printf("%s    %s\n", (const char*)(path + mol->label()), (const char*)label);
      if(mol->decays()) {
	if(path + mol->label() == label) {
	  kc2param_map(nrxn, 0) = v1;
	  break;
	}
	nrxn++;
      }
    } else if(isReaction(item)) {
      ReactionItem *rxn = (ReactionItem*)item;
      //printf("%s    %s\n", (const char*)(path + rxn->label()), (const char*)label);
      if(path + rxn->label() == label) {
	kc2param_map(nrxn, 0) = v1;
	if(rxn->reversible()) {
	  kc2param_map(nrxn+1, 0) = v2;
	}
	break;  // This break messes things up if there are multiple reactions with the same label
      }
      nrxn++;
      if(((ReactionItem*)item)->reversible()) {
	nrxn++;
      }
    } else if(isSubnetwork(item)) {
      SubnetworkItem *sn = (SubnetworkItem*)item;
      if(buildMap(sn->rawItems(), label, v1, v2,
		  path + sn->label() + "::", nrxn)) break;
    }
    ++it;
  }

  return item != NULL;
}
