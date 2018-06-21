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

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qlabel.h>
#include <qtimer.h>

#include "vcrcontrols.h"
#include "controls.xpm"

VCRButtons::VCRButtons(QWidget *parent)
  : QWidget(parent)
{
  QPixmap pix(controls_xpm);
  setBackgroundPixmap(pix);
  setFixedSize(pix.size());
  limits_[0] = QRect(5,   2,  25-  5, 20-2); 
  limits_[1] = QRect(43,  2,  60- 43, 20-2); 
  limits_[2] = QRect(78,  2,  98- 78, 20-2); 
  limits_[3] = QRect(114, 2, 132-114, 20-2); 
  limits_[4] = QRect(145, 2, 164-145, 20-2); 
  limits_[5] = QRect(180, 2, 200-180, 20-2);
}

void
VCRButtons::mousePressEvent(QMouseEvent *e) {
  pos_ = e->pos();
}

void
VCRButtons::mouseReleaseEvent(QMouseEvent *e) {
  for(int i=0; i<6; i++) {
    if(limits_[i].contains(pos_) && limits_[i].contains(e->pos())) {
      switch(i) {
      case 0: emit play();        break;
      case 1: emit stop();        break;
      case 2: emit rewind();      break;
      case 3: emit stepback();    break;
      case 4: emit stepforward(); break;
      case 5: emit end();         break;
      }
      break;
    }
  }
}

VCRControls::VCRControls(QWidget *parent, const char *name)
  : QWidget(parent, name),
    timer_delay_(100)
{
  QHBoxLayout *box = new QHBoxLayout(this);
  box->setAutoAdd(true);

  VCRButtons *vb_ = new VCRButtons(this);
  time_ = new QLineEdit("1", this);
  time_validator_ = new QIntValidator(1, 1, this);
  time_->setValidator(time_validator_);
  QLabel *label = new QLabel(" of ", this);
  time_limit_ = new QLineEdit("1", this);
  
  time_->setFixedSize(time_->sizeHint());
  label->setFixedSize(label->sizeHint());
  time_limit_->setFixedSize(time_->sizeHint());
  time_limit_->setReadOnly(true);

  timer_ = new QTimer(this);
  connect(timer_, SIGNAL(timeout()), SLOT(stepforward()));

  connect(vb_, SIGNAL(rewind()),      SLOT(rewind()));
  connect(vb_, SIGNAL(stepback()),    SLOT(stepback()));
  //connect(vb_, SIGNAL(stop()),        SIGNAL(stop()));
  //connect(vb_, SIGNAL(play()),        SIGNAL(play()));
  connect(vb_, SIGNAL(stop()),        SLOT(stop()));
  connect(vb_, SIGNAL(play()),        SLOT(play()));
  connect(vb_, SIGNAL(stepforward()), SLOT(stepforward()));
  connect(vb_, SIGNAL(end()),         SLOT(end()));
  connect(time_, SIGNAL(textChanged(const QString &)), SLOT(timeEntered()));
}

VCRControls::~VCRControls() {
}

void
VCRControls::setFrame(int t) {
  QString tmp;
  tmp.setNum(t);
  time_->setText(tmp);
}

int
VCRControls::frame() {
  return time_->text().toInt();
}

int
VCRControls::frameLimit() {
  return time_limit_->text().toInt();
}

void
VCRControls::setFrameLimit(int n) {
  QString str;
  str.setNum(n);
  time_limit_->setText(str);
  time_validator_->setRange(1, n);
  if(frame() > n) setFrame(n);
}

void
VCRControls::timeEntered() {
  int n = frame();
  emit changedFrame(n);
}

void
VCRControls::rewind() {
  if(frameLimit() >= 1) {
    setFrame(1);
    emit changedFrame(1);
  }
}

void
VCRControls::stepback() {
  int n = frame();
  if(n > 1) {
    setFrame(n-1);
    emit changedFrame(n-1);
  }
}

void
VCRControls::stepforward() {
  int n = frame();
  if(n < frameLimit()) {
    setFrame(n+1);
    emit changedFrame(n+1);
  } else {
    timer_->stop();
  }
}

void
VCRControls::end() {
  int n = frameLimit();
  setFrame(n);
  emit changedFrame(n);
}

void
VCRControls::stop() {
  timer_->stop();
}

void
VCRControls::play() {
  timer_->start(timer_delay_);
}
