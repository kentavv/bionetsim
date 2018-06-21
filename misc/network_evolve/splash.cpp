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

#include <qpixmap.h>

// Reduced exec size by moving logo.xpm includes to seperate file
#include "splash_screen.h"
#include "logo.xpm"

void
showSplashScreen(int msec) {
  SplashScreen *splash = new SplashScreen(""/*"Written by Kent Vander Velden"*/, QPixmap(logo_xpm), msec);
  splash->show();
}
