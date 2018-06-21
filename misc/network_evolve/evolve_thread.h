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

#ifndef _EVOLVE_THREAD_H_
#define _EVOLVE_THREAD_H_

#include <qevent.h>
#include <qlist.h>
#include <qmutex.h>
#include <qthread.h>

#include "matrix.h"
#include "simulation.h"

class PathwayCanvasItem;

class EvolveEvent : public QCustomEvent {
public:
  EvolveEvent();
};

class EvolveThread : public QThread {
public:
  EvolveThread(QObject *r);

  void best(QList<PathwayCanvasItem> &items, Simulation &sim, int &gen, double &fitness, Matrix &stats);
  
protected:
  void run();
  int gen_;
  double fitness_;
  Matrix stats_;
  Simulation sim_;
  QList<PathwayCanvasItem> items_;
  QMutex mutex_;
  QObject *receiver_;
};

#endif
