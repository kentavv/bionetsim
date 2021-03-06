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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <qdatetime.h>
#include <qstringlist.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "evolve_params.h"
#include "individual.h"
#include "log.h"
#include "math_utils.h"
#include "net_utils.h"
#include "pathway_load.h"
#include "population.h"
#include "random.h"
#include "version.h"

#define DO_MUTATION                0x0001
#define DO_HELPFUL_MUTATION        0x0002
#define DO_SHIFT_MUTATION          0x0004
#define DO_HELPFUL_SHIFT_MUTATION  0x0008
#define DO_LOCAL_AREA_OPTIMIZATION 0x0010
#define DO_REPLACE_FAILURES        0x0020
#define DO_LOAD_NETWORK            0x0040
#define DO_LOAD_PARAMS             0x0080
#define DO_SHUTDOWN                0x0100

/*----------------------------------------*/

static char const * const rcsid_ga_ode_opt_cpp = "$Id: ga_ode_opt.cpp,v 1.39 2009/03/14 23:48:27 kent Exp $";

/*------------------------------*/

EvolveParams ep;
Random rng;
static int gene_len;
static int sockfd = -1;

/*------------------------------*/

void
setupEnvironment(int argc, char *argv[]) {
  const char *filename = argv[1];
  if(!ep.load(filename)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    exit(EXIT_FAILURE);
  }

  ep.cs_mode = argc == 3;
  if(ep.cs_mode) {
    ep.i_am_master = strcmp(argv[2], "master") == 0;
    ep.master_ip.s_addr = inet_addr(argv[2]);
  }
  
  rng.setSeed(ep.random_seed);

#if 0
  {
    QStringList unique_rxn_names = ep.pem.rxn_names;
    unique_rxn_names.sort();
    QStringList::Iterator it, it1, it2;
    for(it1 = unique_rxn_names.begin(), it2 = unique_rxn_names.begin(), ++it2;
	it2 != unique_rxn_names.end(); ++it1,  ++it2) {
      if(*it2 == *it1) {
	unique_rxn_names.erase(it2);
      }
    }
    for(it = unique_rxn_names.begin(); it != unique_rxn_names.end(); ++it) {
      printf("%s\n", (const char*)*it);
    }
  }
#endif

  gene_len = ep.param_ranges.nrows();

  ep.print();
  printf("gene_len: %d\n", gene_len);
  fflush(NULL);
}

struct host_t {
  host_t() : fd(-1), offset(0), working(false) {}
  int fd;
  int offset;
  bool working;
  sockaddr_in address;
};

bool
sendPopulationSegment(host_t &host, int command, int &cur, const Population &pop) {
  int n = GenoDYN::min(ep.pop_size, cur+ep.wu_size);
  int n2 = n - cur;

  printf("Sending work unit (size: %d/%d  pos: %d/%d) to %s\n", n2, ep.wu_size, cur, ep.pop_size, inet_ntoa(host.address.sin_addr));

  if(!send(host.fd, (void *)&command, sizeof(command))) {
    puts("send1 failed");
    return false;
  }
  if(!send(host.fd, (void *)&n2, sizeof(n2))) {
    puts("send2 failed");
    return false;
  }
	  
  for(int j=cur; j<n; j++) {
    if(!pop(j).send(host.fd)) {
      puts("send3 failed");
      return false;
    }
  }
  
  host.offset = cur;
  cur = n;

  //printf("new: offset:%d cur:%d\n", host.offset, cur);
  
  return true;
}

bool
receivePopulationSegment(const host_t &host, Population &pop) {
  printf("Receiving results from %s (inserting at position %d)\n", inet_ntoa(host.address.sin_addr), host.offset);

  int n;
  if(!receive(host.fd, (void*)&n, sizeof(n))) {
    puts("receive1 failed");
    return false;
  }
  
  //printf("receiving: offset:%d n:%d\n", host.offset, n);

  bool failed = false;
  for(int j=0; j<n; j++) {
    if(!pop(j+host.offset).receive(host.fd)) {
      puts("receive2 failed");
      failed = true;
      break;
    }
  }
  if(failed) return false;

  return true;
}

static QValueList<host_t> hosts;

void
distributePopulation(const unsigned int command, Population &pop) {
  //puts("distributing");
  QValueList<host_t>::iterator si;
  int cur = 0;
  int client_sock = sockfd;
  fd_set fds;
  int nworking = 0;

  for(si = hosts.begin(); si != hosts.end(); ++si) {
    (*si).offset = 0;
    (*si).working = false;
    if(cur < ep.pop_size) {
      if(!sendPopulationSegment(*si, command, cur, pop)) {
	puts("Shutting connection down3a");
	close((*si).fd);
	si = hosts.erase(si);
	continue;
      }
      (*si).working = true;
      nworking++;
    }
  }
  
  while(1) {
    FD_ZERO(&fds);
    FD_SET(client_sock, &fds);
    for(si = hosts.begin(); si != hosts.end(); ++si) {
      FD_SET((*si).fd, &fds);
    }

    select(1024, &fds, NULL, NULL, NULL);
    
    for(si = hosts.begin(); si != hosts.end();) {
      host_t &host = *si;

      if(FD_ISSET(host.fd, &fds)) {
	host.working = false;
	nworking--;

	if(!receivePopulationSegment(host, pop)) {
	  puts("Shutting connection down");
          close(host.fd);
          si = hosts.erase(si);
          continue;
	}
	
	if(cur < ep.pop_size) {
	  if(!sendPopulationSegment(host, command, cur, pop)) {
	    puts("Shutting connection down3b");
	    close(host.fd);
	    si = hosts.erase(si);
	    continue;
	  }
	  host.working = true;
	  nworking++;	    	  
	}
      }

      ++si;
    }

    if(FD_ISSET(client_sock, &fds)) {
      struct sockaddr_in cli_addr;
      socklen_t cli_len=sizeof(cli_addr);
      int sfd=accept(client_sock, (struct sockaddr*)&cli_addr, &cli_len);
      if(sfd==-1) {
        perror("accept");
        continue;
      }

      printf("New connection from %s\n", inet_ntoa(cli_addr.sin_addr));

      host_t host;
      host.fd = sfd;
      host.address = cli_addr;

      if(cur < ep.pop_size) {
	if(!sendPopulationSegment(host, command, cur, pop)) {
	  puts("Shutting connection down3c");
	  close(host.fd);
	  continue;
	}
	host.working = true;
	nworking++;
      }

      hosts.push_back(host);      
    }
    if(nworking <= 0) {
      break;
    }
  }
}

void
shutdownSlaves() {
  QValueList<host_t>::iterator si;

  for(si = hosts.begin(); si != hosts.end(); ++si) {
    printf("Sending shutdown signal to %s\n", inet_ntoa((*si).address.sin_addr));

    unsigned int command = DO_SHUTDOWN, n = 0;
    send((*si).fd, (void *)&command, sizeof(command));
    send((*si).fd, (void *)&n, sizeof(n));
  }

  hosts.clear();
}

void
slaveCalculate() {
  Population pop(ep.wu_size, gene_len);
  fd_set fds;
  int i;
  while(1) {
    unsigned int command = 0;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    select(sockfd+1, &fds, NULL, NULL, NULL);

    if(FD_ISSET(sockfd, &fds)) {
      int n;
      if(!receive(sockfd, (void*)&command, sizeof(command)) ||
	 !receive(sockfd, (void*)&n, sizeof(n))) abort();
      if(command & DO_SHUTDOWN) return;
      pop.resize(n, gene_len);
      printf("command:%d\n", command);
      printf("n:%d\n", n);
      for(i=0; i<n; i++) {
	if(!pop(i).receive(sockfd)) abort();
      }
    }

    puts("Slave data received");
    ep.timer.start();
    
    if(command & DO_HELPFUL_MUTATION) {
      for(i=0; i<pop.size(); i++) {
	pop(i).helpfulMutate();
      }
    }

    if(command & DO_HELPFUL_SHIFT_MUTATION) {
      for(i=0; i<pop.size(); i++) {
	pop(i).helpfulShiftMutate();
      }
    }

    if(command & DO_REPLACE_FAILURES) {
      if(ep.replace_failures) {
	for(i=0; i<pop.size(); i++) {
	  int j=0;
	  while(pop(i).fitness() == DBL_MAX && j < 50) {
	    pop(i).random();
	    j++;
	    printf("Replacing failing individual %d (attempt %d)\n", i, j);
	  }
	}
      }
    }

    pop.sort();

    if(command & DO_LOCAL_AREA_OPTIMIZATION) {
      if(ep.simplex_size > 0) {
	printf("%d secs (before simplex)\n", ep.timer.elapsed()/1000);

	int nopt = 0, i;
	for(i=0; i<GenoDYN::min(ep.simplex_size, pop.size()); i++) {
	  Individual before = pop(i);
	  pop(i).localAreaOptimization();
	  printf("pop(%d) before simplex:\t", i); before.print();
	  printf("pop(%d) after simplex:\t", i); pop(i).print();
	  nopt++;
	}
	if(ep.time_limit_type == EvolveParams::Timer) {
	  for(; i<pop.size() && ep.timer.elapsed() < ep.time_limit*1000; i++) {
	    Individual before = pop(i);
	    pop(i).localAreaOptimization();
	    printf("pop(%d) before simplex:\t", i); before.print();
	    printf("pop(%d) after simplex:\t", i); pop(i).print();
	    nopt++;   
	  }
	}
	printf("Performed simplex on %d individuals\n", nopt);
	if(nopt > 1) {
	  pop.sort();
	}
      }
    }

    printf("%d secs\n", ep.timer.elapsed()/1000);
    
    puts("Slave returning data");
    int n = pop.size();
    send(sockfd, (void*)&n, sizeof(n));
    for(i=0; i<GenoDYN::min(ep.wu_size, pop.size()); i++) {
      if(!pop(i).send(sockfd)) abort();
    }
    puts("Done returning data");
    fflush(NULL);
  }
}

extern int ff;

void
evolve() {
  //printf("%d\n", ep.pop_size);
  Population pop(ep.pop_size, gene_len);
  if(ep.cs_mode) {
    unsigned int command = 0;
    distributePopulation(command, pop);
  }
  pop.sort();
  
  Individual prev_best, best;
     
  for(int gen=0; gen<=ep.ngen || ep.ngen==-1; gen++) {
    ep.timer.start();

    if(!ep.cs_mode && 0) {
      if(gen < ep.ngen/4*3) {
	if((random() % 10) == 0) {
	  ff = random() % 4;
	  printf("Switched fitness function to %d\n", ff);
	  pop.setChanged();
	  pop.sort();
	}
      } else if(gen == ep.ngen/4*3) {
	ff = 4;
	printf("Switched fitness function to %d\n", ff);
	pop.setChanged();
	pop.sort();
      }
    }
    
    if(ep.population_identity_check_interval > 0 && (gen % ep.population_identity_check_interval) == 0) {
      pop.prune();
      if(ep.cs_mode) {
	unsigned int command = 0;
	distributePopulation(command, pop);
      }
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
	unlink("best.net");
	if(symlink(filename, "best.net") == -1) {
          perror("symlink");
	}
      }
    }

    if(gen == ep.ngen) break;
   
    if(ep.elitism) {
      prev_best = pop(0);
    }

    pop.crossover();
    if(ep.cs_mode) {
      unsigned int command = DO_LOCAL_AREA_OPTIMIZATION | DO_REPLACE_FAILURES;
      distributePopulation(command, pop);

      pop.sort();
      printf("****************Comparing %f to %f\n", pop(0).fitness(), prev_best.fitness());
      if(ep.elitism) {
	if(pop(0) > prev_best) {
	  printf("****************Replacing %f with %f\n", pop(0).fitness(), prev_best.fitness());
	  pop(ep.pop_size-1) = prev_best;
	  pop.sort();
	}
      }
    } else {
      int i;
      for(i=0; i<ep.pop_size; i++) {
	pop(i).helpfulMutate();
      }
      
      for(i=0; i<ep.pop_size; i++) {
	pop(i).helpfulShiftMutate();
      }
      
      if(ep.replace_failures) {
	for(i=0; i<ep.pop_size; i++) {
	  int j=0;
	  while(pop(i).fitness() == DBL_MAX && j < 50) {
	    pop(i).random();
	    printf("Replacing failing individual %d (attempt %d)\n", i, j);
	    j++;
	  }
	}
      }

      pop.sort();
      if(ep.elitism) {
	if(pop(0) > prev_best) {
	  pop(ep.pop_size-1) = prev_best;
	  pop.sort();
	}
      }
      
      if(ep.simplex_size > 0) {
	//printf("%d secs (before simplex)\n", ep.timer.elapsed()/1000);
      
	for(int i=0; i<GenoDYN::min(ep.simplex_size, pop.size()); i++) {
	  Individual before = pop(i);
	  pop(i).localAreaOptimization();
	  printf("pop(%d) before simplex:\t%d\t", i, gen); before.print();
	  printf("pop(%d) after simplex:\t%d\t", i, gen); pop(i).print();
	}
	if(ep.simplex_size > 1) {
	  pop.sort();
	}
      }

      //printf("%d secs\n", ep.timer.elapsed()/1000);
    }
    fflush(NULL);
  }
  if(ep.cs_mode) {
    shutdownSlaves();
  }
}

int
main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "%s: <parameter file> [<master IP address> | \"master\"]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  appname = "GA ODE Opt";
  appversion = "0.01 devel";

  // Disable debug and warning output
  //setLogFP(stdout, NULL, stdout, NULL);
  setLogFP(stdout, stdout, stdout, stdout);
  
  setupEnvironment(argc, argv);

#if 0
  {
    Individual ind(gene_len), tmp;
    double best_s = 1000, best_sa = 1000;
    char fn[1024];
    for(int i=0; i<1000000; i++) {
      ind.random();
      printf("%06d_i: ", i); ind.print();
      tmp = ind;
      ep.timer.start();
      ep.time_limit = 10*60;
      tmp.simplex();
      sprintf(fn, "simplex_%06d.net", i);
      tmp.save(fn);
      printf("%06d_s: ", i); tmp.print();
      if(tmp.fitness() < best_s) best_s = tmp.fitness();
      //tmp = ind;
      //tmp.simulatedAnnealing();
      //sprintf(fn, "sa_%06d.net", i);
      //tmp.save(fn);
      // leave printing of intermidiate points in SA - they give indication
      // of areas of equal fitness
      //printf("%06d_a: ", i); tmp.print();
      //if(tmp.fitness() < best_sa) best_sa = tmp.fitness();
      printf("best_s: %f  best_sa: %f\n", best_s, best_sa);
      //      abort();
      fflush(NULL);
    }
    
    return 0;
  }
#endif

  if(ep.cs_mode) {
    if(ep.i_am_master) {
      sockfd = openServerSocket(GA_ODE_OPT_PORT);
      evolve();
    } else {
      sockfd = openSocket(ep.master_ip.s_addr, GA_ODE_OPT_PORT);
      slaveCalculate();
    }
  } else {
    evolve();
  }
  
  return 0;
}

/*------------------------------*/
