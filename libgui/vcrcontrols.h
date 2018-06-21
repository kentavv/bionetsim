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

#ifndef _VCRCONTROLS_H_
#define _VCRCONTROLS_H_

#include <qwidget.h>

class QLineEdit;
class QIntValidator;
class QTimer;

class VCRButtons : public QWidget {
  Q_OBJECT
public:
  VCRButtons(QWidget *parent);

signals:
  void rewind();
  void stepback();
  void stop();
  void play();
  void stepforward();
  void end();

protected:
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);

  QPoint pos_;
  QRect limits_[6];
};

class VCRControls : public QWidget {
  Q_OBJECT
public:
  VCRControls(QWidget *parent = NULL, const char *name = NULL);
  ~VCRControls();

  void setFrame(int);
  int frame();
  int frameLimit();
  void setFrameLimit(int);
 
signals:
  //void stop();
  //void play();
  void changedFrame(int);

public slots:
  void stop();
  void play();
  void rewind();
  void stepback();
  void stepforward();
  void end();

private slots:
  void timeEntered();

private:
  QLineEdit *time_, *time_limit_; 
  QIntValidator *time_validator_;
  QTimer *timer_;
  int timer_delay_;
};

#endif
