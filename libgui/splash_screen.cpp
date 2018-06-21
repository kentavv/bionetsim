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

#include <qpainter.h>
#include <qtimer.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qapplication.h>

#include "splash_screen.h"

SplashScreen::SplashScreen(const char *msg, const QPixmap &pixmap, int msec)
  : QDialog(0, "Splash", false, WStyle_Customize|WStyle_Splash) 
{
  message_ = new SplashLabel(msg, pixmap, this);
  message_->show();
  if(msec > 0) {
    QTimer::singleShot(msec, this, SLOT(finish()));
  }
  connect(message_, SIGNAL(clicked()), SLOT(finish()));
}

void
SplashScreen::finish() {
  done(0);
}  

void
SplashScreen::mousePressEvent(QMouseEvent*) {
  finish();
}

SplashLabel::SplashLabel(const char *text, const QPixmap &pixmap, QWidget *parent)
  : QLabel(parent),
    text_(text)
{
  setPixmap(pixmap);
  setFixedSize(sizeHint());
}

void
SplashLabel::drawContents(QPainter *p) {
  QLabel::drawContents(p);
  if(!text_.isEmpty()) {
    QFont defFont = QFont::defaultFont();
    QFont bolded  = QFont(defFont.family(), 14, QFont::Bold);
    p->setFont(bolded);
    p->setPen(white);
    QFontMetrics fm(bolded); 
  }
}

void
SplashLabel::mousePressEvent(QMouseEvent*) {
  emit clicked();
}
