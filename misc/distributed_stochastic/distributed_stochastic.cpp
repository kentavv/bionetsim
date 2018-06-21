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

#include <qcstring.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

#include "calculate.h"
#include "fitness_functions.h"
#include "log.h"
#include "math_utils.h"
#include "net_utils.h"
#include "pathway_load.h"
#include "pathway_matrix.h"
#include "random.h"
#include "version.h"

#define DO_SIMULATE                0x0001
#define DO_LOAD_NETWORK            0x0040
#define DO_LOAD_PARAMS             0x0080
#define DO_SHUTDOWN                0x0100

/*----------------------------------------*/

static char const * const rcsid_distributed_stochastic_cpp = "$Id: distributed_stochastic.cpp,v 1.8 2009/03/14 23:00:36 kent Exp $";

/*------------------------------*/

Random rng;
static int sockfd = -1;

/*------------------------------*/

template<class T> T min(T a, T b) { return a <= b ? a : b; }

bool
loadNetwork(PathwayMatrix &pem, const char *buf, long n) {
  QByteArray qbuf;
  qbuf.assign(buf, n);
  
  if(!pathwayLoadBuffer(qbuf, pem)) {
    fprintf(stderr, "Unable to load network from memory\n");
    return false;
  }

  qbuf.resetRawData(buf, n);
  
  if(QString(simulationMethodNames[pem.simulation.simulationMethod()]) != "Stochastic") {
    fprintf(stderr, "Simulation mode has not been set to Stochastic\n");
    return false;
  }

  if(pem.simulation.singleTrajectory() || pem.simulation.numReplicates() <= 1) {
    fprintf(stderr, "Only one trajectory has been requested\n");
    return false;
  }

  //rng.setSeed(random_seed);

  return true;
}

bool
loadFile(const char *filename, char **buf, long *n) {
  FILE *file = fopen(filename, "r");
  if(!file) return false;
    
  fseek(file, 0, SEEK_END);
  *n = ftell(file);
  
  *buf = new char[*n];
    
  fseek(file, 0, SEEK_SET);
  
  long nr = fread(*buf, sizeof(char), *n, file);
  
  if(*n != nr) {
    delete[] *buf;
    *buf = NULL;
  }
  
  return true;
}

bool
exportTimeSeries(const QString &filename, const Matrix &results, const QStringList &results_labels) {
  //printf("Exporting matrix of size %d x %d\n", results.nrows(), results.ncolumns());
  FILE *fp = fopen((const char*)filename, "w");
  if(!fp) return false;
 
  fprintf(fp, "Time");
  for(QStringList::ConstIterator it = results_labels.begin(); it != results_labels.end(); ++it) {
    fprintf(fp, "\t%s", (const char*)*it);
  }
  fputc('\n', fp);
 
  results.print(fp);
   
  fclose(fp);
 
  return true;
}

struct env_work_t {
  env_work_t() : nrequested(0), nsubmitted(0), nreceived(0) {}
  env_work_t(int nrep) : nrequested(nrep), nsubmitted(0), nreceived(0) {}
  int nrequested;
  int nsubmitted;
  int nreceived;
};

class Work {
public:
  Work(int nenv, int nrep);

  int nextEnv();
  int receivedEnv(int env);

private:
  QValueVector<env_work_t> work;  
};

Work::Work(int nenv, int nrep) {
  work.resize(nenv, env_work_t(nrep));
}

int
Work::nextEnv() {
  unsigned int i;
  for(i=0; i<work.size(); i++) {
    if(work[i].nsubmitted < work[i].nrequested) {
      work[i].nsubmitted++;
      return i;
    }
  }
  for(i=0; i<work.size(); i++) {
    if(work[i].nreceived < work[i].nrequested) {
      printf("Resending work from env %d\n", i);
      work[i].nsubmitted++;
      return i;
    }
  }
  puts("all done");
  return -1;
}

int
Work::receivedEnv(int env) {
  work[env].nreceived++;
  if(work[env].nreceived > work[env].nrequested) {
    printf("Ignoring left over from env%d\n", env);
    return -1;
  }
  printf("env %d/%d  rep %d/%d completed\n", env+1, work.size(), work[env].nreceived, work[env].nrequested);
  return work[env].nreceived-1;;
}

struct host_t {
  host_t() : fd(-1), working(false) {}
  int fd;
  bool working;
  sockaddr_in address;
};

void
masterSimulate(const char *filename) {
  char *net_txt = NULL;
  long net_txt_len = 0;
  if(!loadFile(filename, &net_txt, &net_txt_len)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    return;
  }

  PathwayMatrix pem;
  if(!loadNetwork(pem, net_txt, net_txt_len)) {
    return;
  }

  QValueList<host_t> hosts;
  QValueList<host_t>::iterator si;
  fd_set fds;

  QStringList labels = pem.plottedMoleculeNames();

  Work work(pem.simulation.numEnvironment(),
	    pem.simulation.numReplicates());
  int env = 0;
  
  while(1) {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    for(si = hosts.begin(); si != hosts.end(); ++si) {
      FD_SET((*si).fd, &fds);
    }

    //puts("In select...");
    select(1024, &fds, NULL, NULL, NULL);
    //puts("Out of select...");    

    for(si = hosts.begin(); si != hosts.end();) {
      host_t host = *si;
      if(FD_ISSET(host.fd, &fds)) {
	printf("Receiving results from %s\n", inet_ntoa(host.address.sin_addr));
	Matrix results;
	int senv;
	if(!receive(host.fd, (void*)&senv, sizeof(senv))) {
	  puts("Shutting connection down");
	  close(host.fd);
	  si = hosts.erase(si);
	  continue;
	}
	
	if(!receive(host.fd, results)) {
	  puts("Shutting connection down2");
	  close(host.fd);
	  si = hosts.erase(si);
	  continue;
	}
	
	int rep = work.receivedEnv(senv);
	
	if((env = work.nextEnv()) != -1) {
	  //pem.setCurEnvironment(env);
	  const unsigned int command = DO_SIMULATE;
	  if(!send(host.fd, (void *)&command, sizeof(command))) abort();
	  if(!send(host.fd, (void *)&env, sizeof(env))) abort();
	  //host.working = true;
	} else {
	  host.working = false;
	}
	
	if(rep >= 0) {	  
	  QString fn;
	  fn.sprintf("env%02d_rep%04d.txt", senv, rep);
	  if(!exportTimeSeries(fn, results, labels)) {
	    fprintf(stderr, "Unable to save %s\n", (const char*)fn);
	    exit(EXIT_FAILURE);
	  }
	}
      }
      ++si;
    }

    if(FD_ISSET(sockfd, &fds)) {
      struct sockaddr_in cli_addr;
      socklen_t cli_len=sizeof(cli_addr);
      int sfd=accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
      if(sfd==-1) {
	perror("accept");
	continue;
      }
      
      printf("New connection from %s\n", inet_ntoa(cli_addr.sin_addr));

      host_t host;
      host.fd = sfd;
      host.address = cli_addr;

      {
	const unsigned int command = DO_LOAD_NETWORK;
	if(!send(sfd, (void *)&command, sizeof(command))) abort();
	if(!send(sfd, (void *)&net_txt_len, sizeof(net_txt_len))) abort();
	if(!send(sfd, (void *)net_txt, net_txt_len)) abort();
      }

      if((env = work.nextEnv()) != -1) {
	const unsigned int command = DO_SIMULATE;
	if(!send(sfd, (void *)&command, sizeof(command))) abort();
	if(!send(sfd, (void *)&env, sizeof(env))) abort();
	host.working = true;
      }
      
      hosts.push_back(host);
    }
    fflush(NULL);
    if(env == -1) break;
  }

  for(si = hosts.begin(); si != hosts.end(); ++si) {
    close((*si).fd);
  }

  delete[] net_txt;
}

void
slaveCalculate() {
  bool network_loaded = false;
  PathwayMatrix pem;
  
  fd_set fds;
  while(1) {
    unsigned int command = 0;
    int env;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    //puts("In slave select");
    select(sockfd+1, &fds, NULL, NULL, NULL);
    //puts("Out of slave select");
    if(FD_ISSET(sockfd, &fds)) {
      if(!receive(sockfd, (void*)&command, sizeof(command))) abort();
      printf("Command: %d\n", command);
      if(command == DO_LOAD_NETWORK) {
	network_loaded = false;
	
	long net_txt_len;
	char *net_txt;
	if(!receive(sockfd, (void *)&net_txt_len, sizeof(net_txt_len))) abort();
	net_txt = new char[net_txt_len];
	if(!receive(sockfd, (void *)net_txt, net_txt_len)) abort();
	fflush(NULL);
	if(!loadNetwork(pem, net_txt, net_txt_len)) {
	  continue;
	}

	delete[] net_txt;
	
	network_loaded = true;
      }

      if(command == DO_SHUTDOWN) {
	puts("shutting down");
	exit(EXIT_SUCCESS);
      }
      
      if(command == DO_SIMULATE) {
	if(!network_loaded) abort();
	
	if(!receive(sockfd, (void*)&env, sizeof(env))) abort();
	printf("env: %d\n", env);
	//puts("Slave data received");
      
	pem.setCurEnvironment(env);

	Result result;
	while(1) {
	  if(!run_simulation(pem, result)) {
	    fprintf(stderr, "Simulation failed\n");
	    continue;
	  }
	  break;
	}
	printf("Slave returning matrix of size %d x %d\n", result.m.nrows(), result.m.ncolumns());
	if(!send(sockfd, (void*)&env, sizeof(env))) abort();
	puts("Sending matrix");
	if(!send(sockfd, result.m)) abort();
      }
      puts("Done returning data");
      fflush(NULL);
    }
  }
}

int
main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "%s: [<filename> master]|[<master ip>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  appname = "Distributed Stochastic";
  appversion = "0.01 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);

  if(argc == 3) {
    sockfd = openServerSocket(DS_ENGINE_OLD_PORT);
    masterSimulate(argv[1]);
  } else {
    sockfd = openSocket(inet_addr(argv[1]), DS_ENGINE_OLD_PORT);
    slaveCalculate();
  }
  
  return 0;
}

/*------------------------------*/
