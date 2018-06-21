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

#ifndef _NET_UTILS_H_
#define _NET_UTILS_H_

#define GA_ODE_OPT_PORT     16000
#define DE_ODE_OPT_PORT     16001
#define RANDOM_SIMPLEX_PORT 16002
#define DS_ENGINE_OLD_PORT  16003
#define DS_ENGINE_PORT      16004
#define DS_CLIENT_PORT      16005

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <qstring.h>

#include "matrix.h"
#include "result.h"

bool resolve_hostname(const char *hostname, in_addr &addr);

bool receive(int sockfd, void *ptr, int n);
bool send(int sockfd, void *ptr, int n);

bool receive(int sockfd, Matrix &mat);
bool send(int sockfd, const Matrix &mat);

bool receive(int sockfd, QString &str);
bool send(int sockfd, const QString &str);

bool receive(int sockfd, Result &result);
bool send(int sockfd, const Result &result);

#ifndef WIN32
#include <sys/types.h>
#include <netinet/in.h>

int openSocket(in_addr_t ip, short port);
int openServerSocket(short port);
#endif

#endif
