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

#ifndef _SPLASH_SCREEN_H_
#define _SPLASH_SCREEN_H_

#include <qdialog.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qstring.h>

class QPainter;

class SplashScreen: public QDialog {
Q_OBJECT
public:
  SplashScreen(const char *msg, const QPixmap &pixmap, int msec=5000);

protected:
  void mousePressEvent(QMouseEvent*);
 
private slots:
  void finish();

private:
  QLabel *message_;
};

class SplashLabel: public QLabel {
Q_OBJECT
public:
  SplashLabel(const char *text, const QPixmap &pixmap, QWidget *parent=0);

signals:
  void clicked();
  
protected:
  void drawContents(QPainter *p);
  void mousePressEvent(QMouseEvent*);
  
private:
  QString text_;
};

#endif
