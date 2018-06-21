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
#include <time.h>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qpixmap.h>

#include "network_editor_mw.h"
#include "splash_screen.h"
#include "version.h"

#include "logo.xpm"
#include "icon.xpm"

#ifdef WIN32
#define srandom srand
#endif

int
main(int argc, char *argv[]) {
  qApp = new QApplication(argc, argv);

  appname = "GenoDYN";
  appversion = "1.0";
  app_key = "/GenoDYN/";
  
  bool show_splash = true;
  {
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, registry);
    show_splash = settings.readBoolEntry(app_key + "ShowSplash", true);
  }

  NetworkEditorApp nea;

  qApp->setMainWidget(&nea);
  nea.show();

  nea.setCaption(appname + " " + appversion + " " __DATE__);
  nea.setIcon(QPixmap(icon_xpm));
  
  if(argc >= 2) {
    qApp->processEvents();
    for(int i=1; i<argc; i++) {
      if(!nea.open(argv[i])) {
	// Do not display splash screen if an error occurs
	show_splash = false;
      }
    }
    if(argc > 2) {
      nea.tile();
    }
  } else {
    nea.newDoc();
  }
  
  QMimeSourceFactory::defaultFactory()->setPixmap("SplashScreen", QPixmap(logo_xpm));
  if(show_splash) {
    QPixmap pix = QPixmap::fromMimeSource("SplashScreen");
    SplashScreen *splash = new SplashScreen("", pix, 5000);
    splash->show();
  }

  qApp->connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  srandom(time(NULL));
  
  return qApp->exec();
}
