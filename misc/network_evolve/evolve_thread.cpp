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

#include <qapplication.h>
#include <qevent.h>
#include <qmutex.h>
#include <qthread.h>

#include "evolve_thread.h"
#include "network_evolve.h"
#include "pathway_canvas_items.h"
#include "simulation.h"

EvolveEvent::EvolveEvent()
  : QCustomEvent(QEvent::User + 100)
{}

EvolveThread::EvolveThread(QObject *r)
  : gen_(0),
    fitness_(-1.0),
    receiver_(r)
{}

void
EvolveThread::best(QList<PathwayCanvasItem> &items, Simulation &sim,
		   int &gen, double &fitness, Matrix &stats) {
  mutex_.lock();
  sim = sim_;
  items = items_;
  gen = gen_;
  fitness = fitness_;
  stats = stats_;
  mutex_.unlock();
}

void
EvolveThread::run() {
  bool do_once = true;
  while(1) {
    if(do_once) {
      setup();
      do_once = false;
      gen_ = 0;
    } else {
      evolve();
      gen_++;
    }
    mutex_.lock();
    evolve_sim(sim_);
    fitness_ = best_individual(items_);
    population_stats(stats_);
    mutex_.unlock();
    EvolveEvent *ee = new EvolveEvent();
    QApplication::postEvent(receiver_, ee);
  }
}
