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

QString param_filename;

/*----------------------------------------*/

static char const * const rcsid_random_simplex_cpp = "$Id: random_simplex.cpp,v 1.7 2009/03/14 23:01:09 kent Exp $";

/*------------------------------*/

EvolveParams ep;
Random rng;
static int gene_len;
static int sockfd = -1;

/*------------------------------*/

void
setupEnvironment(int argc, char *argv[]) {
  const char *filename = argv[1];
  param_filename = filename;
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

void
masterLoop() {
  Population pop(5, gene_len);
  Individual a(gene_len), b(gene_len);
  int sockets[256];
  sockaddr_in addresses[256];
  int cur = 0;
  int client_sock = sockfd;
  int nsockets = 0;
  fd_set fds;
  int nreceived = 0;
  for(int i=0; i<256; i++) {
    sockets[i] = -1;
  }
  while(1) {
    FD_ZERO(&fds);
    FD_SET(client_sock, &fds);
    for(int i=0; i<nsockets; i++) {
       if(sockets[i] != -1) {
        FD_SET(sockets[i], &fds);
      }
    }
    select(1024, &fds, NULL, NULL, NULL);
    if(FD_ISSET(client_sock, &fds)) {
      struct sockaddr_in cli_addr;
      socklen_t cli_len=sizeof(cli_addr);
      int sfd=accept(client_sock, (struct sockaddr*)&cli_addr, &cli_len);
      if(sfd==-1) {
	perror("accept");
	continue;
      }
      sockets[nsockets] = sfd;
      addresses[nsockets++] = cli_addr;
    }
    for(int i=0; i<nsockets; i++) {
      if(sockets[i] != -1 && FD_ISSET(sockets[i], &fds)) {
	int sfd = sockets[i];
	
	if(!a.receive(sfd)) abort();
	if(!b.receive(sfd)) abort();
	//printf("Before (%d): ", sfd); a.print();
	//printf("After  (%d): ", sfd); b.print();
	nreceived++;
	if(nreceived <= 5) {
	  pop(nreceived-1) = b;
	  if(nreceived == 5) pop.sort();
	} else {
	  if(b < pop(4)) {
	    pop(4) = b;
	  }
	  pop.sort();
	}
      }
    }
    system("clear");
    if(nreceived > 5) {
      pop.print();
    }
    printf("%d received  %d slaves\n", nreceived, nsockets);
  }
}

void
slaveLoop() {
  Individual a(gene_len), b(gene_len);
  while(1) {
    a.random();
    b = a;
    b.localAreaOptimization();
    a.fitness();
    b.fitness();
    if(!a.send(sockfd)) abort();
    if(!b.send(sockfd)) abort();
  }
}

extern int ff;

void
evolve() {
  Individual a(gene_len), b(gene_len);  
  while(1) {
    a.random();
    b = a;
    b.localAreaOptimization();
    printf("Before: "); a.print();
    printf("After: "); b.print();
  }
}

int
main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "%s: <parameter file> [<master IP address> | \"master\"]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  //appname = "Random Simplex";
  //appversion = "0.01 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);
  
  setupEnvironment(argc, argv);

  if(ep.cs_mode) {
    if(ep.i_am_master) {
      sockfd = openServerSocket(GA_ODE_OPT_PORT);
      masterLoop();
    } else {
      sockfd = openSocket(ep.master_ip.s_addr, GA_ODE_OPT_PORT);
      slaveLoop();
    }
  } else {
    evolve();
  }
  
  return 0;
}

/*------------------------------*/
