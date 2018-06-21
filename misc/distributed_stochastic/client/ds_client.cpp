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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>

#include "calculate.h"
#include "log.h"
#include "net_utils.h"
#include "pathway_load.h"
#include "pathway_matrix.h"
#include "version.h"

#define CLIENT_SIMULATE     0x1001
#define CLIENT_ABORT        0x1002
#define CLIENT_CLOSE        0x1004

/*----------------------------------------*/

static char const * const rcsid_ds_client_cpp = "$Id: ds_client.cpp,v 1.9 2009/03/14 23:00:37 kent Exp $";

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
  
  if(QString(simulationMethodNames[pem.simulation.simulationMethod()]) != "Stochastic") {
    fprintf(stderr, "Simulation mode has not been set to Stochastic\n");
    return false;
  }

  if(pem.simulation.singleTrajectory() || pem.simulation.numReplicates() <= 1) {
    fprintf(stderr, "Only one trajectory has been requested\n");
    return false;
  }

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
exportTimeSeries(const QString &filename, const QValueList<Result> &results) {
  FILE *fp = fopen((const char*)filename, "w");
  if(!fp) return false;

  QValueList<Result>::const_iterator i;
  for(i = results.begin(); i != results.end(); ++i) {
    fprintf(fp, "%s\n", (const char*)(*i).env);
    fprintf(fp, "Time");
    for(QStringList::ConstIterator j = (*i).column_labels.begin(); j != (*i).column_labels.end(); ++j) {
      fprintf(fp, "\t%s", (const char*)*j);
    }
    fputc('\n', fp);

    (*i).m.print(fp);

    fputc('\n', fp);
    fputc('\n', fp);
  }

  fclose(fp);
 
  return true;
}

void
clientSimulate(int sockfd, const char *filename) {
  char *net_txt = NULL;
  long net_txt_len = 0;
  if(!loadFile(filename, &net_txt, &net_txt_len)) {
    fprintf(stderr, "Unable to load %s\n", filename);
    return;
  }
 
  PathwayMatrix pem;
  if(!loadNetwork(pem, net_txt, net_txt_len)) {
    delete[] net_txt;
    fprintf(stderr, "Failed to load network from buffer\n");
    return;
  }

  int net_id = -1;
  
  {
    const unsigned int command = CLIENT_SIMULATE;
    if(!send(sockfd, (void *)&command, sizeof(command)) ||
       !send(sockfd, (void *)&net_txt_len, sizeof(net_txt_len)) ||
       !send(sockfd, (void *)net_txt, net_txt_len) ||
       !receive(sockfd, (void *)&net_id, sizeof(net_id))) {
      delete[] net_txt;
      fprintf(stderr, "Failed to negotiate initial command\n");
      return;
    }
  }

  delete[] net_txt;
  net_txt = NULL;

  fd_set fds;

  int rep = 0;

  while(1) {
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    select(sockfd+1, &fds, NULL, NULL, NULL);
    if(FD_ISSET(sockfd, &fds)) {
      printf("Receiving results\n");
      int tmp_net_id, n;
      if(!receive(sockfd, (void*)&tmp_net_id, sizeof(tmp_net_id)) ||
	 !receive(sockfd, (void*)&n, sizeof(n))) {
	puts("Shutting connection down");
	close(sockfd);
	break;
      }

      Result tmp;
      QValueList<Result> results;
      for(int i=0; i<n; i++) {
	if(!receive(sockfd, tmp)) {
	  puts("Shutting connection down");
	  close(sockfd);
	  break;
	}
	results.append(tmp);
      }

      if(net_id != tmp_net_id) {
	fprintf(stderr, "net_id (%d) != tmp_net_id (%d)\n", net_id, tmp_net_id);
	continue;
      }
      
      QString fn;
      fn.sprintf("rep%04d.txt", rep);
      if(!exportTimeSeries(fn, results)) {
	fprintf(stderr, "Unable to save %s\n", (const char*)fn);
	exit(EXIT_FAILURE);
      }
      rep++;
    }
  }

  {
    const unsigned int command = CLIENT_CLOSE;
    send(sockfd, (void *)&command, sizeof(command));
  }
}

int
main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(stderr, "%s: <filename> <engine ip>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  appname = "Distributed Stochastic";
  appversion = "0.02 devel";

  // Disable debug and warning output
  setLogFP(stdout, NULL, NULL, NULL);

  int sockfd = openSocket(inet_addr(argv[2]), DS_CLIENT_PORT);

  QString ver;
  if(!receive(sockfd, ver) || ver != netformat) {
    fprintf(stderr, "Unable to match network formats (%s) != (%s)\n", (const char*)ver, (const char*)netformat);
    exit(EXIT_FAILURE);
  }

  clientSimulate(sockfd, argv[1]);
  
  return 0;
}

/*------------------------------*/
