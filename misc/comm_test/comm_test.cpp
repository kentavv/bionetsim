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
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

#include "net_utils.h"
#include "random.h"

#define DO_SIMULATE                0x0001
#define DO_LOAD_NETWORK            0x0040
#define DO_LOAD_PARAMS             0x0080
#define DO_SHUTDOWN                0x0100

/*----------------------------------------*/

static char const * const rcsid_comm_test_cpp = "$Id: comm_test.cpp,v 1.2 2009/03/14 23:00:26 kent Exp $";

/*------------------------------*/

Random rng;
static int sockfd = -1;

/*------------------------------*/

template<class T> T min(T a, T b) { return a <= b ? a : b; }

struct host_t {
  host_t() : fd(-1), working(false) {}
  int fd;
  bool working;
  sockaddr_in address;
};

void
masterSimulate() {
  const unsigned int command = DO_SIMULATE;
  QValueList<host_t> hosts;
  QValueList<host_t>::iterator si;
  fd_set fds;
  int env = 0;
  
  while(1) {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    for(si = hosts.begin(); si != hosts.end(); ++si) {
      FD_SET((*si).fd, &fds);
    }

    select(1024, &fds, NULL, NULL, NULL);

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
	
	if(!send(host.fd, (void *)&command, sizeof(command))) abort();
	if(!send(host.fd, (void *)&env, sizeof(env))) abort();	
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
      
      if(!send(sfd, (void *)&command, sizeof(command))) abort();
      if(!send(sfd, (void *)&env, sizeof(env))) abort();
      host.working = true;
      
      hosts.push_back(host);
    }
    fflush(NULL);
    if(env == -1) break;
  }

  for(si = hosts.begin(); si != hosts.end(); ++si) {
    close((*si).fd);
  }
}

void
slaveCalculate() {
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
      if(!receive(sockfd, (void*)&env, sizeof(env))) abort();
      printf("env: %d\n", env);
      //puts("Slave data received");
      
      if(command & DO_SHUTDOWN) {
	puts("shutting down");
	exit(EXIT_SUCCESS);
      }
      
      if(command & DO_SIMULATE) {
	Matrix results(101, 13);
	int delay = rng.integer(0, 20);
	printf("Delaying %ds\n", delay);
	sleep(delay);
	printf("Slave returning matrix of size %d x %d\n", results.nrows(), results.ncolumns());
	if(!send(sockfd, (void*)&env, sizeof(env))) abort();
	puts("Sending matrix");
	if(!send(sockfd, results)) abort();
      }
      puts("Done returning data");
      fflush(NULL);
    }
  }
}

int
main(int argc, char *argv[]) {
  rng.setSeed(-1);
  if(argc == 2) {
    sockfd = setupMaster();
    masterSimulate();
  } else {
    sockfd = setupSlave();
    slaveCalculate();
  }
  
  return 0;
}

/*------------------------------*/
