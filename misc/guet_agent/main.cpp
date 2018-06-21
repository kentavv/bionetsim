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
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qscrollview.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qpixmap.h>

#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#define srandom srand
#endif

template<class T> T sqr(T x) { return x * x; }

typedef enum { agent, plasmid, polymerase, lac, lambda, tetr, gfp, lac_mrna, lambda_mrna, tetr_mrna, gfp_mrna, protease, ribosome } AgentTypes;

class Agent {
public:
  Agent() : x_((random()%1000)/1000.-.5), y_((random()%1000)/1000.-.5), r_(.1), t_(0) {
  }
  virtual void draw(QPainter *) = 0;
  virtual void iterate() { t_++; }

  double x() { return x_; }
  double y() { return y_; }
  double r() { return r_; }

  void setX(double x) { x_ = x; }
  void setY(double y) { y_ = y; }

  int age() { return t_; }
  
  virtual AgentTypes rtti() { return agent; }
  
protected:
  double x_, y_;
  double r_;
  int t_;
};

class Plasmid : public Agent {
public:
  Plasmid() {
    r_ = .03;
    rot_ = 0;
    op_[0] = -1;
    op_[1] = -1;
    op_[2] = -1;
    op_[3] = -1;
  }
  void draw(QPainter *p) {
    p->save();
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->setPen("orange");   // lac
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 0+rot_, 30);
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 30+rot_, 470);
    p->setPen(Qt::blue);  // lambda
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 500+rot_, 30);
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 530+rot_, 470);
    p->setPen(Qt::yellow); // tet
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 1000+rot_, 30);
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 1030+rot_, 470);
    p->setPen(Qt::green); // gfp
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 1500+rot_, 30);
    p->drawArc((x_-r_)*400+200+50, (y_-r_)*400.+200+50, r_*1000, r_*1000, 1530+rot_, 470);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    if(op_[0] >= 0) op_[0]++;
    if(op_[1] >= 0) op_[1]++;
    if(op_[2] >= 0) op_[2]++;
    if(op_[3] >= 0) op_[3]++;
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;

    rot_ += ((random()%201) - 100);
  }
  bool operatorFilled(AgentTypes t) {
    if(t == lac) return op_[0] >= 0;
    if(t == lambda) return op_[1] >= 0;
    if(t == tetr) return op_[2] >= 0;
    if(t == gfp) return op_[3] >= 0;
  }
  AgentTypes rtti() { return plasmid; }
  int op_[4];
protected:
  int rot_;
};

class Polymerase : public Agent {
public:
  Polymerase() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return polymerase; }
};

class Protein : public Agent {
public:
  Protein() {}
};

class mRNA : public Agent {
public:
  mRNA() {}
};

class Lac : public Protein {
public:
  Lac() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen("orange");
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return lac; }
};

class TetR : public Protein {
public:
  TetR() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::yellow);
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return tetr; }
};

class Lambda : public Protein {
public:
  Lambda() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::blue);
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return lambda; }
};

class GFP : public Protein {
public:
  GFP() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::green);
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return gfp; }
};






class Lac_mRNA : public mRNA {
public:
  Lac_mRNA() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen("orange");
    p->drawLine((x_-r_)*400+200+50, (y_-r_)*400+200+50, (x_+r_)*400+200+50, (y_+r_)*400+200+50);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return lac_mrna; }
};

class TetR_mRNA : public mRNA {
public:
  TetR_mRNA() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::yellow);
    p->drawLine((x_-r_)*400+200+50, (y_-r_)*400+200+50, (x_+r_)*400+200+50, (y_+r_)*400+200+50);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return tetr_mrna; }
};

class Lambda_mRNA : public mRNA {
public:
  Lambda_mRNA() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::blue);
    p->drawLine((x_-r_)*400+200+50, (y_-r_)*400+200+50, (x_+r_)*400+200+50, (y_+r_)*400+200+50);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return lambda_mrna; }
};

class GFP_mRNA : public mRNA {
public:
  GFP_mRNA() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::green);
    p->drawLine((x_-r_)*400+200+50, (y_-r_)*400+200+50, (x_+r_)*400+200+50, (y_+r_)*400+200+50);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return gfp_mrna; }
};


class Protease : public Agent {
public:
  Protease() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen(Qt::red);
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return protease; }
};

class Ribosome : public Agent {
public:
  Ribosome() {
    r_ = .005;
  }
  void draw(QPainter *p) {
    p->save();
    p->setPen("gray");
    p->drawEllipse((x_-r_)*400+200+50, (y_-r_)*400+200+50, r_*1000, r_*1000);
    p->restore();
  }
  void iterate() {
    Agent::iterate();
    double x = x_ + ((random()%1000)/1000.-.5) * .02;
    double y = y_ + ((random()%1000)/1000.-.5) * .02;
    double d = sqrt(sqr(x) + sqr(y));
    //printf("%f %f %f\t\t", x, y, d);
    if(d > .5 - r_) {
      x = x / d * (.5 - r_);
      y = y / d * (.5 - r_);
    }
    //printf("%f %f %f\n", x, y, d);

    x_ = x;
    y_ = y;
  }
  AgentTypes rtti() { return ribosome; }
};

class AgentView : public QScrollView {
  Q_OBJECT
public:
  AgentView(QWidget *parent=0, const char *name=0);
public slots:
  void iterate();
protected:
  void drawContents(QPainter *, int, int, int, int);
  void viewportPaintEvent(QPaintEvent *pe);
  
  QTimer *timer_;
  QPtrList<Agent> agents_;
};

AgentView::AgentView(QWidget *parent, const char *name)
  : QScrollView(parent, name)
{
  resizeContents(500, 500);
  viewport()->setBackgroundMode(NoBackground);

  timer_ = new QTimer(this);
  connect(timer_, SIGNAL(timeout()), SLOT(iterate()));
  timer_->start(50);
  agents_.append(new Plasmid);
  for(int i=0; i<50; i++) {
    agents_.append(new Polymerase);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new Lac);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new Lambda);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new TetR);
  }
  for(int i=0; i<30; i++) {
    //agents_.append(new GFP);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new Lac);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new Lac_mRNA);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new Lambda_mRNA);
  }
  for(int i=0; i<10; i++) {
    //agents_.append(new TetR_mRNA);
  }
  for(int i=0; i<30; i++) {
    //agents_.append(new GFP_mRNA);
  }
  for(int i=0; i<10; i++) {
    agents_.append(new Protease);
  }
  for(int i=0; i<10; i++) {
    agents_.append(new Ribosome);
  }
}

void
AgentView::drawContents(QPainter *p, int cx, int cy, int cw, int ch) {
  p->fillRect(cx, cy, cw, ch, black);
  p->setPen(white);
  p->drawEllipse(50, 50, 400, 400);

  for(Agent *agent = agents_.first(); agent; agent = agents_.next()) {
    agent->draw(p);
  }
}

void
AgentView::iterate() {
  int nplasmid=0;
  int nlac=0;
  int nlambda=0;
  int ntetr=0;
  int ngfp=0;
  int nlac_mrna=0;
  int nlambda_mrna=0;
  int ntetr_mrna=0;
  int ngfp_mrna=0;
  int npolymerase=0;
  int nprotease=0;
  int nribosome=0;

  QPtrListIterator<Agent> i(agents_);
  QPtrListIterator<Agent> j(agents_);
  Agent *a, *b;

  while((a = i.current()) != 0) {
    ++i;
    a->iterate();
    if(a->rtti() == plasmid) {
      printf("%d %d %d %d\t\t",
	     ((Plasmid*)a)->op_[0], ((Plasmid*)a)->op_[1],
	     ((Plasmid*)a)->op_[2], ((Plasmid*)a)->op_[3]);
    }

    if(a->rtti() == plasmid) nplasmid++;
    if(a->rtti() == lac) nlac++;
    if(a->rtti() == lambda) nlambda++;
    if(a->rtti() == tetr) ntetr++;
    if(a->rtti() == gfp) ngfp++;
    if(a->rtti() == lac_mrna) nlac_mrna++;
    if(a->rtti() == lambda_mrna) nlambda_mrna++;
    if(a->rtti() == tetr_mrna) ntetr_mrna++;
    if(a->rtti() == gfp_mrna) ngfp_mrna++;
    if(a->rtti() == polymerase) npolymerase++;
    if(a->rtti() == protease) nprotease++;
    if(a->rtti() == ribosome) nribosome++;

    if((a->rtti() == lac_mrna || a->rtti() == lambda_mrna ||
	a->rtti() == tetr_mrna || a->rtti() == gfp_mrna) &&
       a->age() > 400) {
      agents_.remove(a);
      delete a;
      continue;
    }
    
    if(a->rtti() == plasmid) {
      if(((Plasmid*)a)->op_[0] >= 0 && (random()%1000)>950) {
	((Plasmid*)a)->op_[0] = -1;
	Agent *c = new TetR;
	c->setX(a->x() + a->r() + (random()%1000)/1000.*.01);
	c->setY(a->y() + a->r() + (random()%1000)/1000.*.01);	
	agents_.prepend(c);
      }
      if(((Plasmid*)a)->op_[1] >= 0 && (random()%1000)>950) {
	((Plasmid*)a)->op_[1] = -1;
	Agent *c = new Lac;
	c->setX(a->x() + a->r() + (random()%1000)/1000.*.01);
	c->setY(a->y() + a->r() + (random()%1000)/1000.*.01);	
	agents_.prepend(c);
      }
      if(((Plasmid*)a)->op_[2] >= 0 && (random()%1000)>950) {
	((Plasmid*)a)->op_[2] = -1;
	Agent *c = new TetR;
	c->setX(a->x() + a->r() + (random()%1000)/1000.*.01);
	c->setY(a->y() + a->r() + (random()%1000)/1000.*.01);	
	agents_.prepend(c);
      }
      if(((Plasmid*)a)->op_[3] >= 0 && (random()%1000)>950) {
	((Plasmid*)a)->op_[3] = -1;
	Agent *c = new Lambda;
	c->setX(a->x() + a->r() + (random()%1000)/1000.*.01);
	c->setY(a->y() + a->r() + (random()%1000)/1000.*.01);	
	agents_.prepend(c);
      }
    }
  }

  i.toFirst();
  while((a = i.current()) != 0) {
    ++i;
    j = i;
    ++j;
    while((b = j.current()) != 0) {
      ++j;
      // with IPTG
#if 1
      if(b->rtti() == plasmid && a->rtti() == lac) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)b)->operatorFilled(lambda)) {
	    ((Plasmid*)b)->op_[1] = 0;
	    agents_.remove(a);
	    delete a;
	    break;
	  }
	}
      }
      if(a->rtti() == plasmid && b->rtti() == lac) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)a)->operatorFilled(lambda)) {
	    ((Plasmid*)a)->op_[1] = 0;
	    agents_.remove(b);
	    delete b;
	    continue;
	  }
	}
      }
#endif
      
      // with aTc
#if 0
      if(b->rtti() == plasmid && a->rtti() == tetr) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)b)->operatorFilled(lac) && !((Plasmid*)b)->operatorFilled(tetr)) {
	    if((random()%2) == 0) {
	      ((Plasmid*)b)->op_[0] = 0;
	      agents_.remove(a);
	      delete a;
	      break;
	    } else {
	      ((Plasmid*)b)->op_[2] = 0;
	      agents_.remove(a);
	      delete a;
	      break;
	    }	      
	  } else if(!((Plasmid*)b)->operatorFilled(lac)) {
	    ((Plasmid*)b)->op_[0] = 0;
	    agents_.remove(a);
	    delete a;
	    break;
	  } else if(!((Plasmid*)b)->operatorFilled(tetr)) {
	    ((Plasmid*)b)->op_[2] = 0;
	    agents_.remove(a);
	    delete a;
	    break;
	  }
	}
      }
      if(a->rtti() == plasmid && b->rtti() == tetr) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)a)->operatorFilled(lac) && !((Plasmid*)a)->operatorFilled(tetr)) {
	    if((random()%2) == 0) {
	      ((Plasmid*)a)->op_[0] = 0;
	      agents_.remove(b);
	      delete b;
	      continue;
	    } else {
	      ((Plasmid*)a)->op_[2] = 0;
	      agents_.remove(b);
	      delete b;
	      continue;
	    }	      
	  } else if(!((Plasmid*)a)->operatorFilled(lac)) {
	    ((Plasmid*)a)->op_[0] = 0;
	    agents_.remove(b);
	    delete b;
	    continue;
	  } else if(!((Plasmid*)a)->operatorFilled(tetr)) {
	    ((Plasmid*)a)->op_[2] = 0;
	    agents_.remove(b);
	    delete b;
	    continue;
	  }
	}
      }
#endif

      if(b->rtti() == plasmid && a->rtti() == lambda) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)b)->operatorFilled(gfp)) {
	    ((Plasmid*)b)->op_[3] = 0;
	    agents_.remove(a);
	    delete a;
	    break;
	  }
	}
      }
      if(a->rtti() == plasmid && b->rtti() == lambda) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)a)->operatorFilled(gfp)) {
	    ((Plasmid*)a)->op_[3] = 0;
	    agents_.remove(b);
	    delete b;
	    continue;
	  }
	}
      }







      
      if(a->rtti() == polymerase && b->rtti() == plasmid) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)b)->operatorFilled(lac)) {
	    Agent *c = new Lac_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)b)->operatorFilled(lambda)) {
	    Agent *c = new Lambda_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)b)->operatorFilled(tetr)) {
	    Agent *c = new TetR_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)b)->operatorFilled(gfp)) {
	    Agent *c = new GFP_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	}
      }
      if(b->rtti() == polymerase && a->rtti() == plasmid) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(!((Plasmid*)a)->operatorFilled(lac)) {
	    Agent *c = new Lac_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)a)->operatorFilled(lambda)) {
	    Agent *c = new Lambda_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)a)->operatorFilled(tetr)) {
	    Agent *c = new TetR_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	  if(!((Plasmid*)a)->operatorFilled(gfp)) {
	    Agent *c = new GFP_mRNA;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	}
      }
      
      if((a->rtti() == lac || a->rtti() == lambda ||
	  a->rtti() == tetr || a->rtti() == gfp) &&
	 b->rtti() == protease) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  agents_.remove(a);
	  delete a;
	  break;
	}
      } else if(a->rtti() == protease &&
		(b->rtti() == lac || b->rtti() == lambda ||
		 b->rtti() == tetr || b->rtti() == gfp)) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  agents_.remove(b);
	  delete b;
	  continue;
	}
      }
      if((a->rtti() == lac_mrna || a->rtti() == lambda_mrna ||
	  a->rtti() == tetr_mrna || a->rtti() == gfp_mrna) &&
	 b->rtti() == ribosome) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(a->rtti() == lac_mrna) {
	    Agent *c = new Lac;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(a->rtti() == lambda_mrna) {
	    Agent *c = new Lambda;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(a->rtti() == tetr_mrna) {
	    Agent *c = new TetR;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(a->rtti() == gfp_mrna) {
	    Agent *c = new GFP;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	}
      } else if(a->rtti() == ribosome &&
		(b->rtti() == lac_mrna || b->rtti() == lambda_mrna ||
		 b->rtti() == tetr_mrna || b->rtti() == gfp_mrna)) {
	if(sqrt(sqr(a->x() - b->x()) + sqr(a->y() - b->y())) < (a->r() + b->r())) {
	  if(b->rtti() == lac_mrna) {
	    Agent *c = new Lac;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(b->rtti() == lambda_mrna) {
	    Agent *c = new Lambda;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(b->rtti() == tetr_mrna) {
	    Agent *c = new TetR;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  } else if(b->rtti() == gfp_mrna) {
	    Agent *c = new GFP;
	    c->setX((a->x() + b->x())/2);
	    c->setY((a->y() + b->y())/2);
	    agents_.prepend(c);
	  }
	}
      }
    }
  }
  printf("%d %d %d %d   %d %d  %d %d  %d %d  %d %d\n", nplasmid, npolymerase, nprotease, nribosome,
	 nlac, nlac_mrna, nlambda, nlambda_mrna, ntetr, ntetr_mrna, ngfp, ngfp_mrna);
  viewport()->update();
}

void
AgentView::viewportPaintEvent(QPaintEvent *pe) {
  QWidget* vp = viewport();
  //QPainter p(vp);
  QRect r = pe->rect();
                                                                                
  //printf("Updated needed: %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
                                                                                
  int ex2 = r.x();
  int ey2 = r.y();
  int ew2 = r.width();
  int eh2 = r.height();
                                                                                
  QPixmap pix(ew2, eh2);
  QPainter p2(&pix);
  p2.fillRect(0, 0, ew2, eh2, backgroundColor());
                                                                                
  r &= vp->rect();
  int ex = r.x();
  int ey = r.y();
  int ew = r.width();
  int eh = r.height();
  //printf("r&vp: %d %d %d %d\n", ex, ey, ew ,eh);
                                                                                
  //r.setX(r.x() + contentsX());
  //r.setY(r.y() + contentsY());
                                                                                
  r &= QRect(0, 0, contentsWidth(), contentsHeight());
  //printf("ra&p: %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
  if(r.isValid()) {
    ex = r.x();
    ey = r.y();
    ew = r.width();
    eh = r.height();
                                                                                
    //printf("r2&p: %d %d %d %d\n", ex, ey, ew, eh);
                                                                                
    p2.setClipRect(ex-ex2, ey-ey2, ew, eh);
    //printf("Clip: %d %d %d %d\n", ex, ey, ew, eh);
                                                                                
    p2.translate(-(ex + contentsX()), -(ey + contentsY()));
                                                                                
    drawContents(&p2, (ex + contentsX()), (ey + contentsY()), ew, eh);
                                                                                     
    p2.end();
  } else {
    p2.end();
    //puts("Invalid");
  }
  bitBlt(vp, ex2, ey2, &pix);                                                                           
}

class AgentWindow : public QMainWindow {
public:
  AgentWindow(QWidget *parent=0, const char *name=0);
protected:
  AgentView *av_;
};

AgentWindow::AgentWindow(QWidget *parent, const char *name)
  : QMainWindow(parent, name)
{
  av_ = new AgentView(this);
  setCentralWidget(av_);
}

int
main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  srandom(time(NULL));
  
  AgentWindow aw;

  app.setMainWidget(&aw);
  aw.resize(600, 500);
  aw.show();

  return app.exec();
}

#include "main.moc"
