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

#include "fitness_landscape_app.h"
#include "version.h"

int
main(int argc, char *argv[]) {
  qApp = new QApplication(argc, argv);

  FitnessLandscapeApp app;

  qApp->setMainWidget(&app);
  app.resize(650, 400);
  app.show();
  
  if(argc > 2) {
    QMessageBox::critical(NULL, appname, QObject::tr("USAGE: %1 <filename>").arg(argv[0]));
    exit(EXIT_FAILURE);
  }

  if(argc == 2) {
    app.setDir(argv[1]);
  }

  return qApp->exec();
}
