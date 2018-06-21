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

#include <stdlib.h>

#include <qapplication.h>
#include <qmessagebox.h>

#include "network_evolve_app.h"
#include "version.h"
#include "splash.h"

//QApplication *qApp = NULL;

int
main(int argc, char *argv[]) {
  qApp = new QApplication(argc, argv);

  showSplashScreen();

  NetworkEvolveApp nea;

  qApp->setMainWidget(&nea);
  nea.show();

  nea.setCaption(appname " " appversion " " __DATE__);
  
  qApp->connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  return qApp->exec();
}
