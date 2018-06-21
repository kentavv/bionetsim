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
#include <signal.h>
#include <sys/types.h>

#ifndef WIN32
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <qdatetime.h>

static void ignoreSigPipe();

#ifdef WIN32
// Need to remove global min and max before including net_utils.h
#undef min
#undef max
#endif

#include "net_utils.h"

//#define TIMING

bool
resolve_hostname(const char *hostname, in_addr &addr) {
#ifdef WIN32
  unsigned long ip = inet_addr(hostname);
  if(ip != INADDR_NONE) {
    addr.s_addr = ip;
    return true;
  }
#else
  if(inet_aton(hostname, &addr) != 0) {
    return true;
  }
#endif

#ifdef PTHREAD
  pthread_mutex_lock(&excl_);
#endif
  struct hostent *hostptr = gethostbyname(hostname);
  
  if(hostptr == NULL) {
#ifndef WIN32
    herror(hostname);
#else
    fprintf(stderr, "gethostbyname(): Error: %d\n", WSAGetLastError());
#endif
#ifdef PTHREAD
    pthread_mutex_unlock(&excl_);
#endif
    addr.s_addr=INADDR_NONE;
    return false;
  }
  
  memcpy(&addr, hostptr->h_addr, hostptr->h_length);
#ifdef PTHREAD
  pthread_mutex_unlock(&excl_);
#endif
  
  return true;
}

bool
receive(int sockfd, void *ptr, int n) {
#ifdef TIMING
  int nb = n;
  QTime t;
  t.start();
#endif

  fd_set fds;
  timeval tv;
  
  char *p = (char*)ptr;
  do {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    int m;

    tv.tv_sec = 5; // 5 sec. timeout
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
    
    m = recv(sockfd, p, n, 0);
    if(m == 0) {
      puts("Empty recv");
      return false;
    }
    if(m == -1) {
      perror("recv");
      return false;
    }
    n -= m;
    p += m;
  } while(n > 0);

#ifdef TIMING
  int dur = t.elapsed();
  if(dur == 0) {
    printf("Received %d bytes in %d ms\n", nb, dur);
  } else {
    printf("Received %d bytes in %d ms (%f k/s)\n", nb, dur, nb/1024./(dur/1000.));
  }
#endif
  
  return true;
}

bool
send(int sockfd, void *ptr, int n) {
#ifdef TIMING
  int nb = n;
  QTime t;
  t.start();
#endif
  
  char *p = (char*)ptr;
  do {
    int m = send(sockfd, p, n, 0);
    if(m == -1) {
      perror("send");
      return false;
    }
    p += m;
    n -= m;
  } while(n > 0);

#ifdef TIMING
  int dur = t.elapsed();
  if(dur == 0) {
    printf("Sent %d bytes in %d ms\n", nb, dur);
  } else {
    printf("Sent %d bytes in %d ms (%f k/s)\n", nb, dur, nb/1024./(dur/1000.));
  }
#endif
  
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

bool
receive(int sockfd, QString &str) {
  int n = 0;
  if(!receive(sockfd, (void*)&n, sizeof(n))) return false;
  if(n > 0) {
    char *p = new char[n+1];
    //puts("receiving");
    if(!receive(sockfd, (void*)p, sizeof(char)*n)) {
      delete[] p;
      return false;
    }
    p[n] = '\0';
    //printf("receive %s\n", p);
    str = p;
    delete[] p;
  }

  return true;
}

bool
send(int sockfd, const QString &str) {
  int n = str.length();
  if(!send(sockfd, (void*)&n, sizeof(n))) return false;
  //printf("n: %d\n", n);
  if(n > 0) {
    //puts("sending");
    if(!send(sockfd, (void*)(const char*)str, sizeof(char)*n)) return false;
  }

  return true;
}

bool
receive(int sockfd, Result &result) {
  result.clear();
  
  if(!receive(sockfd, (void*)&result.success, sizeof(result.success))) return false;
  if(!receive(sockfd, result.m)) return false;
  if(!receive(sockfd, result.jacobian)) return false;
  {
    int n;
    if(!receive(sockfd, (void*)&n, sizeof(n))) return false;
    for(int i=0; i<n; i++) {
      Matrix m;
      if(!receive(sockfd, m)) return false;
      result.tel.append(new Matrix(m));
    }
  }
  if(!receive(sockfd, result.env)) return false;
  if(!receive(sockfd, result.exp)) return false;
  {
    int n;
    if(!receive(sockfd, (void*)&n, sizeof(n))) return false;
    for(int i=0; i<n; i++) {
      QString str;
      if(!receive(sockfd, str)) return false;
      result.column_labels.append(str);
    }
  }
  {
    int n;
    if(!receive(sockfd, (void*)&n, sizeof(n))) return false;
    result.cont.resize(n);
    for(int i=0; i<n; i++) {
      bool v;
      if(!receive(sockfd, (void*)&v, sizeof(v))) return false;
      result.cont[i] = v;
    }
  }

  return true;
}

bool
send(int sockfd, const Result &result) {
  if(!send(sockfd, (void*)&result.success, sizeof(result.success))) return false;
  if(!send(sockfd, result.m)) return false;
  if(!send(sockfd, result.jacobian)) return false;
  {
    int n = result.tel.count();
    if(!send(sockfd, (void*)&n, sizeof(n))) return false;
    QPtrListIterator<Matrix> j(result.tel);
    Matrix *m;
    while((m = j.current())) {
      ++j;
      if(!send(sockfd, *m)) return false;
    }
  }
  if(!send(sockfd, result.env)) return false;
  if(!send(sockfd, result.exp)) return false;
  {
    int n = result.column_labels.size();
    if(!send(sockfd, (void*)&n, sizeof(n))) return false;
    for(QStringList::ConstIterator j = result.column_labels.begin();
	j != result.column_labels.end();
	++j) {
      if(!send(sockfd, *j)) return false;
    }
  }
  {
    int n = result.cont.size();
    if(!send(sockfd, (void*)&n, sizeof(n))) return false;
    for(int i=0; i<n; i++) {
      bool v = result.cont[i];
      if(!send(sockfd, (void*)&v, sizeof(v))) return false;
    }
  }

  return true;
}

#ifndef WIN32
static void
ignoreSigPipe() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;

  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);
}

int
openSocket(in_addr_t ip, short port) {
  ignoreSigPipe();
  
  int sockfd;
  struct sockaddr_in serv_addr;
  
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = ip;
  serv_addr.sin_port = htons(port);

  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1) {
    perror("connect");
    return -1;
  }

  return sockfd;
}

int
openServerSocket(short port) {
  ignoreSigPipe();

  int sockfd;
  struct sockaddr_in serv_addr;

  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  {
    int on = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind");
    return -1;
  }

  if(listen(sockfd, SOMAXCONN)==-1) {
    perror("listen");
    return -1;
  }

  return sockfd;
}
#endif

