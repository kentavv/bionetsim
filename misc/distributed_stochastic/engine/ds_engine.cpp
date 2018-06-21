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
#include <list>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <qcstring.h>

#include "calculate.h"
#include "log.h"
#include "net_utils.h"
#include "pathway_load.h"
#include "pathway_matrix.h"
#include "random.h"
#include "result.h"
#include "version.h"

#define SLAVE_SIMULATE      0x0001
#define SLAVE_LOAD_NETWORK  0x0002
#define SLAVE_ABORT         0x0004
#define SLAVE_SHUTDOWN      0x0008

#define CLIENT_SIMULATE     0x1001
#define CLIENT_ABORT        0x1002
#define CLIENT_CLOSE        0x1004

using std::list;
using std::vector;

/*----------------------------------------*/

static char const * const rcsid_ds_engine_cpp = "$Id: ds_engine.cpp,v 1.12 2009/03/14 23:00:39 kent Exp $";

/*------------------------------*/

Random rng;

/*------------------------------*/

bool
loadNetwork(PathwayMatrix &pem, const char *buf, long n) {
  QByteArray qbuf;
  qbuf.assign(buf, n);
  
  if(!pathwayLoadBuffer(qbuf, pem)) {
    fprintf(stderr, "Unable to load network from memory\n");
    return false;
  }

  qbuf.resetRawData(buf, n);
  
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
    return false;
  }
  
  return true;
}

class Work {
public:
  Work();
  Work(int nrep);

  int nextUnit();
  int receivedUnit();
  bool done() const;
  
private:
  int nrequested;
  int nsubmitted;
  int nreceived;
};

Work::Work() : nrequested(0), nsubmitted(0), nreceived(0) {
}

Work::Work(int nrep) : nrequested(nrep), nsubmitted(0), nreceived(0) {
}

int
Work::nextUnit() {
  if(nsubmitted < nrequested) {
    nsubmitted++;
    return 1;
  }
  if(nreceived < nrequested) {
    puts("Resending work");
    nsubmitted++;
    return 1;
  }
  puts("all done");
  return -1;
}

int
Work::receivedUnit() {
  nreceived++;
  if(nreceived > nrequested) {
    puts("Ignoring left over");
    return -1;
  }
  printf("rep %d/%d completed\n", nreceived, nrequested);
  return nreceived-1;
}

bool
Work::done() const {
  return nreceived >= nrequested;
}

class client_t;

class slave_t {
public:
  slave_t() : fd(-1), working(false), net_id(-1), client(NULL) {}
  int fd;
  bool working;
  int net_id;
  client_t *client;
  sockaddr_in address;
};

class client_t {
public:
  client_t() : fd(-1), net_txt(NULL), net_txt_len(0), net_id(-1) {}
  ~client_t() { if(net_txt) delete[] net_txt; }
  void clearNet() {
    if(net_txt) {
      delete[] net_txt;
    }
    net_txt = NULL;
    net_txt_len = 0;
    net_id = -1;
  }
  int fd;
  char *net_txt;
  long net_txt_len;
  int net_id;
  Work work;  
  sockaddr_in address;
  list<slave_t*> slaves;
};

void
fd_set_max(int fd, fd_set *set, int &max_fd) {
  FD_SET(fd, set);
  if(fd > max_fd) max_fd = fd;
}

bool
waitForData(fd_set &fds, int client_lfd, int slave_lfd,
	    const list<client_t*> &clients, const list<slave_t*> &hosts) { 
  int max_fd;

  FD_ZERO(&fds);
  fd_set_max(slave_lfd, &fds, max_fd);
  fd_set_max(client_lfd, &fds, max_fd);
  
  for(list<client_t*>::const_iterator i = clients.begin(); i != clients.end(); ++i) {
    fd_set_max((*i)->fd, &fds, max_fd);
  }
  
  for(list<slave_t*>::const_iterator i = hosts.begin(); i != hosts.end(); ++i) {
    fd_set_max((*i)->fd, &fds, max_fd);
  }
  
  int rv = select(max_fd+1, &fds, NULL, NULL, NULL);

  if(rv == 0) {
    fprintf(stderr, "select() return 0\n");
    return false;
  } else if(rv == -1) {
    perror("select");
    exit(EXIT_FAILURE);
  }
  return true;
}

bool
sendAbortCommand(slave_t &slave) {
  printf("Sending abort command to %s\n", inet_ntoa(slave.address.sin_addr));
  const unsigned int command = SLAVE_ABORT;
  if(!send(slave.fd, (void *)&command, sizeof(command))) return false;
  slave.working = false;
  slave.net_id = -1;
  slave.client = NULL;
  return true;
}

bool
sendShutdownCommand(slave_t &slave) {
  printf("Sending shutdown command to %s\n", inet_ntoa(slave.address.sin_addr));
  const unsigned int command = SLAVE_SHUTDOWN;
  if(!send(slave.fd, (void *)&command, sizeof(command))) return false;
  slave.working = false;
  slave.net_id = -1;
  slave.client = NULL;
  return true;
}

bool
sendLoadNetworkCommand(slave_t &slave) {
  if(slave.net_id == slave.client->net_id) return true;

  printf("Sending load network command to %s\n", inet_ntoa(slave.address.sin_addr));
  const unsigned int command = SLAVE_LOAD_NETWORK;
  if(!send(slave.fd, (void *)&command, sizeof(command)) ||
     !send(slave.fd, (void *)&slave.client->net_id, sizeof(slave.client->net_id)) ||
     !send(slave.fd, (void *)&slave.client->net_txt_len, sizeof(slave.client->net_txt_len)) ||
     !send(slave.fd, (void *)slave.client->net_txt, slave.client->net_txt_len)) return false;

  slave.net_id = slave.client->net_id;
  
  return true;
}

bool
sendSimulateCommand(slave_t &slave) {
  printf("Sending simulate network command to %s\n", inet_ntoa(slave.address.sin_addr));
  const unsigned int command = SLAVE_SIMULATE;
  if(!send(slave.fd, (void *)&command, sizeof(command)) ||
     !send(slave.fd, (void *)&slave.net_id, sizeof(slave.net_id))) return false;
  slave.working = true;

  return true;
}

void
clientAbort(client_t &client) {
  list<slave_t *>::iterator i;
  for(i = client.slaves.begin(); i != client.slaves.end(); ++i) {
    slave_t *slave = *i;
    sendAbortCommand(*slave);
  }
  client.slaves.clear();
  client.work = Work(0);
}

bool
processClient(client_t &client) {
  printf("Receiving client command from %s\n", inet_ntoa(client.address.sin_addr));
  unsigned int command;
  if(!receive(client.fd, (void*)&command, sizeof(command))) {
    return false;
  }

  printf("Client command: %d\n", command);
  
  if(command == CLIENT_SIMULATE) {
    clientAbort(client);
    
    client.clearNet();
    if(!receive(client.fd, (void *)&client.net_txt_len, sizeof(client.net_txt_len))) return false;
    client.net_txt = new char[client.net_txt_len];
    if(!receive(client.fd, (void *)client.net_txt, client.net_txt_len)) return false;

    PathwayMatrix pem;
    if(!loadNetwork(pem, client.net_txt, client.net_txt_len)) return false;


    int n = 1;
    if(strcmp(simulationMethodNames[pem.simulation.simulationMethod()], "ODE") != 0 &&
       !pem.simulation.singleTrajectory()) {
      n = pem.simulation.numReplicates();
    }
    
    client.work = Work(n);

    client.net_id = rng.int31();
    if(!send(client.fd, (void *)&client.net_id, sizeof(client.net_id))) return false;
  }

  if(command == CLIENT_ABORT) {
    clientAbort(client);
  }

  if(command == CLIENT_CLOSE) {
    return false;
  }
  
  return true;
}

void
processClients(fd_set &fds, list<client_t*> &clients) {
  list<client_t *>::iterator i;
  for(i = clients.begin(); i != clients.end();) {
    client_t *client = *i;
    if(FD_ISSET(client->fd, &fds)) {
      if(!processClient(*client)) {
	puts("Shutting client connection down");
	clientAbort(*client);
	close(client->fd);
	i = clients.erase(i);
	continue;
      }
    }
    ++i;
  }
}

void
processNewClient(int fd, list<client_t*> &clients) {
  struct sockaddr_in cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int sfd = accept(fd, (struct sockaddr*)&cli_addr, &cli_len);
  if(sfd == -1) {
    perror("accept");
    return;
  }
  
  printf("New client connection from %s\n", inet_ntoa(cli_addr.sin_addr));
  
  client_t *client = new client_t;
  client->fd = sfd;
  client->address = cli_addr;

  if(send(client->fd, netformat)) {
    clients.push_back(client);
  }
}

bool
processSlave(slave_t &slave) {
  printf("Receiving results from %s\n", inet_ntoa(slave.address.sin_addr));

  int net_id;
  if(!receive(slave.fd, (void*)&net_id, sizeof(net_id))) return false;

  int n;
  if(!receive(slave.fd, (void*)&n, sizeof(n))) return false;

  QValueVector<Result> results(n);
  for(int i=0; i<n; i++) {
    if(!receive(slave.fd, results[i])) return false;
  }

  if(slave.client) {
    bool disconnect = false;
    if(net_id == slave.client->net_id) {
      puts("Forwarding results to client");
      if(!send(slave.client->fd, (void*)&net_id, sizeof(net_id)) ||
	 !send(slave.client->fd, results[0])) {
	fprintf(stderr, "Unable to send results to client\n");
	//disconnect = true;
	clientAbort(*slave.client);
	return true;
      } else {
	(void)slave.client->work.receivedUnit();
      }
    } else {
      fprintf(stderr, "Discarding old results\n");
      disconnect = true;
    }

    if(disconnect) {
      slave.client->slaves.remove(&slave);
      slave.client = NULL;
      slave.working = false;
    }
    
    if(slave.client) {
      if(slave.client->work.nextUnit() != -1) {
	if(!sendSimulateCommand(slave)) return false;
      } else {
	slave.working = false;
      }
    }
  } else {
    fprintf(stderr, "Results discarded - client is no longer connected\n");
  }

  return true;
}

void
processSlaves(fd_set &fds, list<slave_t*> &slaves) {
  list<slave_t*>::iterator i;
  for(i = slaves.begin(); i != slaves.end();) {
    slave_t *slave = *i;
    if(FD_ISSET(slave->fd, &fds)) {
      if(!processSlave(*slave)) {
	puts("Shutting slave connection down");
	sendShutdownCommand(*slave);
	close(slave->fd);
	if(slave->client) {
	  slave->client->slaves.remove(slave);
	}
	i = slaves.erase(i);
	continue;
      }
    }
    ++i;
  }
}

void
processNewSlave(int fd, list<slave_t*> &slaves) {
  struct sockaddr_in cli_addr;
  socklen_t cli_len=sizeof(cli_addr);
  int sfd=accept(fd, (struct sockaddr*)&cli_addr, &cli_len);
  if(sfd==-1) {
    perror("accept");
    return;
  }
  
  printf("New connection (%d total) from %s\n", slaves.size()+1, inet_ntoa(cli_addr.sin_addr));
  
  slave_t *slave = new slave_t;
  slave->fd = sfd;
  slave->address = cli_addr;

  if(send(slave->fd, netformat)) {
    slaves.push_back(slave);
  }
}

void
masterSimulate() {
  int slave_lfd  = openServerSocket(DS_ENGINE_PORT);
  int client_lfd = openServerSocket(DS_CLIENT_PORT);

  if(slave_lfd == -1 || client_lfd == -1) {
    fprintf(stderr, "Unable to open listening sockets\n");
    return;
  }
  
  list<client_t*> clients;
  list<slave_t*> slaves;
  fd_set fds;

  while(1) {
	  puts("ready waitfordata");
    if(!waitForData(fds, client_lfd, slave_lfd, clients, slaves)) continue;
	  puts("done waitfordata");
puts("a");
    processClients(fds, clients);

puts("b");
    if(FD_ISSET(client_lfd, &fds)) processNewClient(client_lfd, clients);

puts("c");
    processSlaves(fds, slaves);

puts("d");
    if(FD_ISSET(slave_lfd, &fds)) processNewSlave(slave_lfd, slaves); 

puts("e");
    for(list<slave_t*>::iterator i = slaves.begin(); i != slaves.end();) {
      slave_t *slave = *i;
      if(!slave->working) {
	if(!slave->client) {
	  list<client_t*>::iterator j;
	  for(j = clients.begin(); j != clients.end(); j++) {
	    if(!(*j)->work.done()) {
	      slave->client = *j;
	      slave->client->slaves.push_back(slave);
	    }
	  }
	  if(slave->client) continue;
	} else {
	  if(!sendLoadNetworkCommand(*slave)) {
	    sendShutdownCommand(*slave);
	    close(slave->fd);
	    if(slave->client) {
	      slave->client->slaves.remove(slave);
	    }
	    i = slaves.erase(i);
	    continue;
	  }

	  if(slave->client->work.nextUnit() != -1) {
	    if(!sendSimulateCommand(*slave)) {
	      sendShutdownCommand(*slave);
	      close(slave->fd);
	      if(slave->client) {
		slave->client->slaves.remove(slave);
	      }
	      i = slaves.erase(i);
	      continue;
	    }
	  }
	}
      }
      ++i;
    }
  }

  for(list<client_t*>::iterator i = clients.begin(); i != clients.end(); ++i) {
    close((*i)->fd);
  }
  for(list<slave_t*>::iterator i = slaves.begin(); i != slaves.end(); ++i) {
    sendShutdownCommand(*(*i));
    close((*i)->fd);
  }
}

static PathwayMatrix pem;
static QValueList<Result> results;

void *
thread_calculate(void *v) {
  int fd = (int)v;

  bool rv = run_simulation(pem, results);

  send(fd, (void*)&rv, sizeof(rv));

  pthread_exit((void*)rv);
}

void
slaveCalculate(const char *addr) {
  int fd = openSocket(inet_addr(addr), DS_ENGINE_PORT);
  if(fd == -1) {
    fprintf(stderr, "Unable to open connection\n");
    return;
  }

  {
    QString ver;
    if(!receive(fd, ver) || ver != netformat) {
      fprintf(stderr, "Unable to match network formats (%s) != (%s)\n", (const char*)ver, (const char*)netformat);
      return;
    }
  }
  
  bool idle = true;
  pthread_t thread;
  
  int sock[2];
  if(socketpair(AF_UNIX, SOCK_STREAM, 0, sock) == -1) {
    perror("socketpair");
    return;
  }
  
  int net_id = -1;

  fd_set fds;
  while(1) {
    unsigned int command = 0;
    int fd_max=0;
    FD_ZERO(&fds);
    fd_set_max(fd, &fds, fd_max);
    fd_set_max(sock[0], &fds, fd_max);

    select(fd_max+1, &fds, NULL, NULL, NULL);

    if(FD_ISSET(sock[0], &fds)) {
      bool rv;
      receive(sock[0], (void*)&rv, sizeof(rv));
      pthread_join(thread, (void **)&rv);

      if(rv) {
	int n = results.size();
	printf("Slave returning Result object with %d environments\n", n);
	if(!send(fd, (void*)&net_id, sizeof(net_id)) ||
	   !send(fd, (void*)&n, sizeof(n))) {
	  fprintf(stderr, "Unable to send results header\n");
	  return;
	}
	for(QValueList<Result>::ConstIterator i=results.begin();
	    i!=results.end();
	    ++i) {
	  if(!send(fd, *i)) {
	    fprintf(stderr, "Unable to send results\n");
	    return;
	  }
	}
      }

      idle = true;
    }

    if(FD_ISSET(fd, &fds)) {
      if(!receive(fd, (void*)&command, sizeof(command))) {
	fprintf(stderr, "Failed to receive command\n");
	return;
      }
      printf("Command: %d\n", command);

      if(command == SLAVE_LOAD_NETWORK) {
	int tmp_net_id;
	long net_txt_len;
	char *net_txt;
	if(!receive(fd, (void *)&tmp_net_id, sizeof(tmp_net_id)) ||
	   !receive(fd, (void *)&net_txt_len, sizeof(net_txt_len))) {
	  fprintf(stderr, "Unable to receive load network command\n");
	  return;
	}
	net_txt = new char[net_txt_len];
	if(!receive(fd, (void *)net_txt, net_txt_len)) {
	  fprintf(stderr, "Unable to receive load network command\n");
	  return;
	}

	if(!idle) {
	  delete[] net_txt;
	  fprintf(stderr, "Calculation thread currently executing\n");
	  continue;
	}

	if(!loadNetwork(pem, net_txt, net_txt_len)) {
	  delete[] net_txt;
	  net_id = -1;
	  continue;
	}

	pem.simulation.setSingleTrajectory(true);
	
	net_id = tmp_net_id;
	delete[] net_txt;
      }

      if(command == SLAVE_SIMULATE) {
	pem.quit = false;
	
	int tmp_net_id;
	if(!receive(fd, (void *)&tmp_net_id, sizeof(tmp_net_id))) {
	  fprintf(stderr, "Unable to receive simulate command\n");
	  return;
	}
	if(net_id != tmp_net_id) {
	  fprintf(stderr, "Requested net_id (%d) != current net_id (%d)\n", tmp_net_id, net_id);
	  continue;
	}

	if(!idle) {
	  fprintf(stderr, "Calculation thread already executing\n");
	  continue;
	}

	pthread_create(&thread, NULL, thread_calculate, (void*)sock[1]);

	idle = false;
      }

      if(command == SLAVE_ABORT) {
	pem.quit = true;
      }

      if(command == SLAVE_SHUTDOWN) {
	puts("shutting down");
	return;
      }
    }
  }
}

int
main(int argc, char *argv[]) {
  if(argc > 2) {
    fprintf(stderr, "%s: [|<master ip>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  appname = "Distributed Stochastic";
  appversion = "0.02 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);

  if(argc == 1) {
    masterSimulate();
  } else {
    slaveCalculate(argv[1]);
  }
  
  return 0;
}

/*------------------------------*/
