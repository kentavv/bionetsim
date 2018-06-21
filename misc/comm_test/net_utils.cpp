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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <qdatetime.h>

#include "net_utils.h"

#define TCP_PORT 5098
#define SERVER_IP "10.56.0.4"

bool
receive(int sockfd, void *ptr, int n) {
  int nb = n;
  QTime t;
  t.start();

  fd_set fds;
  timeval tv;
  
  char *p = (char*)ptr;
  do {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    int m;

    tv.tv_sec = 60; // 60 sec. timeout
    tv.tv_usec = 0;
    
    m = select(sockfd+1, &fds, NULL, NULL, &tv);
    if(m == 0) {
      puts("receive timeout");
      return false;
    }
    if(m == -1) {
      perror("select (in receive)");
      return false;
    }
    
    m = read(sockfd, p, n);
    if(m == 0) {
      puts("Empty read");
      return false;
    }
    if(m == -1) {
      perror("read");
      return false;
    }
    n -= m;
    p += m;
  } while(n > 0);

  if(nb > 1024) {
    int dur = t.elapsed();
    if(dur == 0) {
      printf("Received %d bytes in %d ms\n", nb, dur);
    } else {
      printf("Received %d bytes in %d ms (%f k/s)\n", nb, dur, nb/1024./(dur/1000.));
    }
  }

  return true;
}

bool
send(int sockfd, void *ptr, int n) {
  int nb = n;
  QTime t;
  t.start();

  char *p = (char*)ptr;
  do {
    int m = write(sockfd, p, n);
    if(m == -1) {
      perror("write");
      return false;
    }
    p += m;
    n -= m;
  } while(n > 0);

  if(nb > 1024) {
    int dur = t.elapsed();
    if(dur == 0) {
      printf("Sent %d bytes in %d ms\n", nb, dur);
    } else {
      printf("Sent %d bytes in %d ms (%f k/s)\n", nb, dur, nb/1024./(dur/1000.));
    }
  }
  
  return true;
}

bool
receive(int sockfd, Matrix &mat) {
  int dim[2]; // dim[0] = nr, dim[1] = nc;
  if(!receive(sockfd, (void*)dim, sizeof(dim))) return false;
  mat.resize(dim[0], dim[1]);
  if(dim[0] * dim[1] > 0) {
    //puts("receiving");
    double *p = new double[dim[0] * dim[1]];
    if(!receive(sockfd, (void*)p, sizeof(double)*dim[0]*dim[1])) {
      delete[] p;
      return false;
    }
    mat.setRaw(p);
    delete[] p;
  }

  return true;
}

bool
send(int sockfd, const Matrix &mat) {
  int dim[2] = { mat.nrows(), mat.ncolumns() };
  if(!send(sockfd, (void*)dim, sizeof(dim))) return false;
  if(dim[0] * dim[1] > 0) {
    //puts("sending");
    if(!send(sockfd, (void*)mat.raw(), sizeof(double)*dim[0]*dim[1])) return false;
  }

  return true;
}

int
openSlaveSocket() {
  int sockfd;
  struct sockaddr_in serv_addr;
  
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  serv_addr.sin_port = htons(TCP_PORT);

  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

int
openMasterSocket() {
  int sockfd;
  struct sockaddr_in serv_addr;

  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  {
    int on = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(TCP_PORT);

  if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if(listen(sockfd, SOMAXCONN)==-1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

int
setupSlave() {
  return openSlaveSocket();
}

int
setupMaster() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;

  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);

  return openMasterSocket();
}

