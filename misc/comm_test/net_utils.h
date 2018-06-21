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

#include "matrix.h"

bool receive(int sockfd, void *ptr, int n);
bool send(int sockfd, void *ptr, int n);

bool receive(int sockfd, Matrix &mat);
bool send(int sockfd, const Matrix &mat);

int openSlaveSocket();
int openMasterSocket();

int setupSlave();
int setupMaster();

#endif
