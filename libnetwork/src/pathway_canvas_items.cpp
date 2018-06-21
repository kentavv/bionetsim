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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qwidget.h>
#include <qpainter.h>

#include "log.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_save.h"
#include "random.h"
#include "simulation.h"
#include "squarewave.h"

#ifdef GUI
#include "annotation_dialog.h"
#include "interpolated_cv_dialog.h"
#include "molecule_dialog.h"
#include "reaction_dialog.h"
#include "squarewave_cv_dialog.h"
#include "subnetwork_dialog.h"

static QFont font_;

QFont pathwayCanvasItemsFont() {
  return font_;
}

void
setPathwayCanvasItemsFont(QFont f) {
  font_ = f;
  //font_.setPointSize(6);
  //font_.setWeight(1);
#if 0
  printf("family: %s\n", (const char*)font_.family());
  printf("pointsize: %d\n", font_.pointSize());
  printf("pixelsize: %d\n", font_.pixelSize());
  printf("weight: %d\n", font_.weight());
  printf("bold: %d\n", font_.bold());
  printf("string: %s\n", (const char*)font_.toString());
  printf("substitute: %s\n", (const char*)font_.substitute(font_.family()));
  printf("raw name: %s\n", (const char*)font_.rawName());
#endif
}
#endif

static Random rng;

static double sf_ = 1.0;

double pathwayCanvasItemsScalingFactor() {
  return sf_;
}

void setPathwayCanvasItemsScalingFactor(double sf) {
  sf_ = sf;
}

int MoleculeItem::RTTI = 1001;
int ReactionItem::RTTI = 1002;
int ReactionEdgeItem::RTTI = 1003;
int InternalNodeItem::RTTI = 1004;
int ControlVariableItem::sub_RTTI = 1005;
int AnnotationItem::RTTI = 1006;
int SubnetworkItem::RTTI = 1007;
int SubnetworkEdgeItem::RTTI = 1008;

PathwayCanvasItem::PathwayCanvasItem(QWidget *p)
  : parent_(p),
    pd_(NULL),
    selected_(false),
    id_(-1)
{}

PathwayCanvasItem::PathwayCanvasItem(const PathwayCanvasItem &i)
  : parent_(i.parent_),
    pd_(NULL),
    //points_(i.points_),
    selected_(i.selected_),
    id_(i.id_)
{
  points_ = i.points_.copy();
}

PathwayCanvasItem::~PathwayCanvasItem() {
  if(pd_) delete pd_;  
}

int
PathwayCanvasItem::id() const {
  return id_;
}

void
PathwayCanvasItem::setId(int v) {
  id_ = v;
}

QString
PathwayCanvasItem::tip() const {
  return "";
}

bool
PathwayCanvasItem::selected() const {
  return selected_;
}

void
PathwayCanvasItem::setSelected(bool s) {
  selected_ = s;
}

void
PathwayCanvasItem::setParent(QWidget *p) {
  parent_ = p;
}

void
PathwayCanvasItem::updatePropertiesDialog(const Simulation &) {
}

int
PathwayCanvasItem::sub_rtti() const {
  return rtti();
}

QRect
PathwayCanvasItem::boundingRect() const {
  return points_.boundingRect();
}

MolRxnBaseItem::MolRxnBaseItem(QWidget *p)
  : PathwayCanvasItem(p),
    label_("???"),
    cap_("???"),
    text_x_(0),
    text_y_(0),
    x_(0),
    y_(0),
    user_initialized_(false)
{
  snugUp();
}

MolRxnBaseItem::MolRxnBaseItem(const MolRxnBaseItem &i)
  : PathwayCanvasItem(i),
    bg_(i.bg_),
    def_bg_(i.def_bg_),
    label_(i.label_),
    cap_(i.cap_),
    text_x_(i.text_x_),
    text_y_(i.text_y_),
    x_(i.x_),
    y_(i.y_),
    //edges_(i.edges_),
    user_initialized_(i.user_initialized_),
    notes_(i.notes_)
{}

MolRxnBaseItem::~MolRxnBaseItem() {
  EdgeBaseItem *edge;
  for(edge = edges_.first(); edge; edge = edges_.next()) {
    edge->removeNode(this);
  }
}

void
MolRxnBaseItem::move(int dx, int dy) {
  moveTo(x_*sf_ + dx, y_*sf_ + dy);
}

void
MolRxnBaseItem::moveTo(int x, int y) {
  x_ = x/sf_;
  y_ = y/sf_;

  EdgeBaseItem *edge;
  QListIterator<EdgeBaseItem> it(edges_);
  while((edge = it.current())) {
    ++it;
    edge->move(this);
  }
}

QString
MolRxnBaseItem::label() const {
  return label_;
}

void
MolRxnBaseItem::setLabel(QString label) {
  label_ = label;
  snugUp();
}

QString
MolRxnBaseItem::caption() const {
  return cap_;
}

void
MolRxnBaseItem::snugUp() {
  //printf("%s snugup: %p\n", (const char*)label_, this);
#ifdef GUI
  int w=0, h=0;
  QFontMetrics fm(font_);
  int y_step = fm.height() + 2;
  QRect bb = fm.boundingRect(label_);
  w = bb.width()+8;
  h = y_step;
  text_x_ = -w/2 + 3;
  text_y_ =  h/2 - 3;
  
  points_.resize(4);
  points_.setPoint(0, -w/2, -h/2);
  points_.setPoint(1,  w/2, -h/2);
  points_.setPoint(2,  w/2,  h/2);
  points_.setPoint(3, -w/2,  h/2);
#endif
}

void
MolRxnBaseItem::addEdge(EdgeBaseItem *e) {
  edges_.append(e);
}

void
MolRxnBaseItem::removeEdge(EdgeBaseItem *e) {
  edges_.remove(e);
}

int
MolRxnBaseItem::numEdges() const {
  return edges_.count();
}

QList<EdgeBaseItem>
MolRxnBaseItem::edges() const {
  return edges_;
}

int
MolRxnBaseItem::x() const {
  return x_;
}

int
MolRxnBaseItem::y() const {
  return y_;
}

bool
MolRxnBaseItem::initialized() const {
  return user_initialized_;
}
  
void
MolRxnBaseItem::setInitialized(bool b) {
  if(user_initialized_ != b) {
    user_initialized_ = b;
#ifdef GUI
    if(parent_) parent_->update();
#endif
  }
}

void
MolRxnBaseItem::draw(QPainter *p) {
  p->save();
  p->scale(sf_, sf_);
#ifdef GUI
  p->setFont(font_);
#endif
  p->setBrush(bg_);
  p->translate(x_, y_);

  p->setPen(pen());
  p->drawPolygon(points_);
  
  p->setPen(Qt::black);
  p->drawText(text_x_, text_y_, label_);

  p->restore();

}

QPen
MolRxnBaseItem::pen() const {
  if(selected_) {
    return QPen(Qt::red, 4);
  } else if(!user_initialized_) {
    return QPen(Qt::gray, 4);
  }
  return QPen(Qt::black);
}

bool
MolRxnBaseItem::contains(QPoint p) {
  return boundingRect().contains(p / sf_);
}

bool
MolRxnBaseItem::collidesWith(const PathwayCanvasItem *p) {
  if(!isEdge(p)) {
    return boundingRect().intersects(((const MolRxnBaseItem*)p)->boundingRect());
  } else {
    return false;
  }
  return false;
}

QPoint
MolRxnBaseItem::intersection(const QPoint &a, const QPoint &b) {
  QPoint rv = b;
  if(contains(b)) {
    for(unsigned int i=0, j=1; j<points_.size()+1; i++, j++) {
      QPoint c, d;
      
      c = QPoint(points_.at(i).x() + x_, points_.at(i).y() + y_); 
      if(j == points_.size()) {
	d = QPoint(points_.at(0).x() + x_, points_.at(0).y() + y_); 
      } else {
	d = QPoint(points_.at(j).x() + x_, points_.at(j).y() + y_); 
      }
      
      double denom = (b.x() - a.x()) * (d.y() - c.y()) - (b.y() - a.y()) * (d.x() - c.x());
      double r = ((a.y() - c.y()) * (d.x() - c.x()) - (a.x() - c.x()) * (d.y() - c.y())) / denom;
      double s = ((a.y() - c.y()) * (b.x() - a.x()) - (a.x() - c.x()) * (b.y() - a.y())) / denom;
            
      if(0.0 <= r && r <= 1.0 && 0.0 <= s && s <= 1.0) {
	rv = QPoint(int(a.x() + r*(b.x() - a.x()) + 0.5),
		    int(a.y() + r*(b.y() - a.y()) + 0.5));
	break;
      }
    }
  } else {
    //puts("No containment");
  }
  
  return rv;
}

QRect
MolRxnBaseItem::boundingRect() const {
  QRect bb = points_.boundingRect();
  bb.moveCenter(QPoint(x_, y_));
  return bb;
}

QString
MolRxnBaseItem::notes() const {
  return notes_;
}

void
MolRxnBaseItem::setNotes(const QString &notes) {
  notes_ = notes;
}

void
MolRxnBaseItem::setColor(QColor c) {
  bg_ = c;
}

void
MolRxnBaseItem::defaultColor() {
  bg_ = def_bg_;
}

QColor
MolRxnBaseItem::color() const {
  return bg_;
}

MoleculeItem::MoleculeItem(QWidget *p)
  : MolRxnBaseItem(p),
    initial_con_(0.0),
    decay_rate_(0.0),
    con_(0.0),
    decays_(false),
    exported_(false),
    plotted_(true),
    tag_("Molecule")
{
  label_ = "Mol";
  cap_ = "Molecule";
}

MoleculeItem::MoleculeItem(const MoleculeItem &i)
  : MolRxnBaseItem(i),
    initial_con_(i.initial_con_),
    decay_rate_(i.decay_rate_),
    con_(i.con_),
    decays_(i.decays_),
    exported_(i.exported_),
    plotted_(i.plotted_),
    tag_(i.tag_)
{}

MoleculeItem::~MoleculeItem() {
  if(pd_) {
    delete pd_;
    pd_ = NULL;
  }
}

void
MoleculeItem::draw(QPainter *p) {
  MolRxnBaseItem::draw(p);
}

QPen
MoleculeItem::pen() const {
  if(selected_) {
    return QPen(Qt::red, 4);
  } else if(!user_initialized_) {
    return QPen(Qt::gray, 4);
  } else if(exported_) {
    return QPen(QColor(0, 0, 0), 4);
  }
  return QPen(Qt::black);
}

int
MoleculeItem::rtti() const {
  return MoleculeItem::RTTI;
}

QWidget *
MoleculeItem::propertiesDialog(QWidget *parent, const Simulation &) {
#ifdef GUI
  if(!pd_) {
    pd_ = new MoleculeDialog(parent, *this);
    pd_->show();
    return pd_;
  } else {
    ((MoleculeDialog*)pd_)->updateDialog();
    pd_->show();
  }
#endif
  return NULL;
}

void
MoleculeItem::updatePropertiesFromDialog() {
  //puts("updating...");
#ifdef GUI
  if(pd_) {
    ((MoleculeDialog*)pd_)->updateItem();
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
MoleculeItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement mol = doc.createElement(tag_);
  n.appendChild(mol);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  mol.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  mol.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  mol.appendChild(e); e.appendChild(t);

  e = doc.createElement("InitialConcentration");
  t = doc.createTextNode(QObject::tr("%1").arg(initial_con_));
  mol.appendChild(e); e.appendChild(t);

  e = doc.createElement("DecayRate");
  t = doc.createTextNode(QObject::tr("%1").arg(decay_rate_));
  mol.appendChild(e); e.appendChild(t);

  if(decays_) {
    e = doc.createElement("Decays");
    mol.appendChild(e);
  }

  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    mol.appendChild(e); e.appendChild(t);
  }

  if(exported_) {
    e = doc.createElement("Exported");
    mol.appendChild(e);
  }

  if(!plotted_) {
    e = doc.createElement("NotPlotted");
    mol.appendChild(e);
  }

  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    mol.appendChild(e);
  }
}

bool
MoleculeItem::exported() const {
  return exported_;
}

void
MoleculeItem::setExported(bool v) {
  exported_ = v;
}

double
MoleculeItem::initialConcentration() const {
  return initial_con_;
}

void
MoleculeItem::setInitialConcentration(double v) {
  initial_con_ = v;
}

bool
MoleculeItem::decays() const {
  return decays_;
}

void
MoleculeItem::setDecays(bool v) {
  decays_ = v;
}

double
MoleculeItem::decayRate() const {
  return decay_rate_;
}

void
MoleculeItem::setDecayRate(double v) {
  decay_rate_ = v;
}

bool
MoleculeItem::plotted() const {
  return plotted_;
}

void
MoleculeItem::setPlotted(bool v) {
  plotted_ = v;
}

double
MoleculeItem::concentration() const {
  return con_;
}

void
MoleculeItem::setConcentration(double v) {
  con_ = v;
}

QString
MoleculeItem::tip() const {
  QString str;
  str.sprintf("%s\n"
	      "Name: %s",
	      (const char*)cap_, (const char*)label_);
  if(decays_) {
    str += QObject::tr("\nDecay Rate: %1").arg(decay_rate_);
  }
  if(initial_con_ != 0.0) {
    str += QObject::tr("\nInitial Concentration: %1").arg(initial_con_);
  }
  if(!plotted_) str += "\nNot plotted";
  if(!user_initialized_) str += "\nNot initialized";
  if(exported_) str += "\nExported";
  
  return str;
}

DNAMoleculeItem::DNAMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor(230, 80, 80);
  cap_ = "DNA Molecule";
  tag_ = "DNAMolecule";
}

DNAMoleculeItem::DNAMoleculeItem(const DNAMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
DNAMoleculeItem::clone() const {
  return new DNAMoleculeItem(*this);
}

mRNAMoleculeItem::mRNAMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor(130, 80, 200);
  cap_ = "mRNA Molecule";
  tag_ = "mRNAMolecule";
}

mRNAMoleculeItem::mRNAMoleculeItem(const mRNAMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
mRNAMoleculeItem::clone() const {
  return new mRNAMoleculeItem(*this);
}

EnzymeMoleculeItem::EnzymeMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor("Orange");
  cap_ = "Enzyme Molecule";
  tag_ = "EnzymeMolecule";
}

EnzymeMoleculeItem::EnzymeMoleculeItem(const EnzymeMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
EnzymeMoleculeItem::clone() const {
  return new EnzymeMoleculeItem(*this);
}

MetaboliteMoleculeItem::MetaboliteMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor(50, 160, 200);
  cap_ = "Metabolite Molecule";
  tag_ = "MetaboliteMolecule";
}

MetaboliteMoleculeItem::MetaboliteMoleculeItem(const MetaboliteMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
MetaboliteMoleculeItem::clone() const {
  return new MetaboliteMoleculeItem(*this);
}

TranscriptionFactorMoleculeItem::TranscriptionFactorMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor(222, 184, 135);
  cap_ = "Transcription Factor Molecule";
  tag_ = "TranscriptionFactorMolecule";
}

TranscriptionFactorMoleculeItem::TranscriptionFactorMoleculeItem(const TranscriptionFactorMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
TranscriptionFactorMoleculeItem::clone() const {
  return new TranscriptionFactorMoleculeItem(*this);
}

ComplexMoleculeItem::ComplexMoleculeItem(QWidget *p)
  : MoleculeItem(p)
{
  bg_ = def_bg_ = QColor(255, 255, 100);  // off yellow
  cap_ = "Complex Molecule";
  tag_ = "ComplexMolecule";
}

ComplexMoleculeItem::ComplexMoleculeItem(const ComplexMoleculeItem &i)
  : MoleculeItem(i)
{}

PathwayCanvasItem *
ComplexMoleculeItem::clone() const {
  return new ComplexMoleculeItem(*this);
}

AnnotationItem::AnnotationItem(QWidget *p)
  : MolRxnBaseItem(p)
{
  bg_ = QColor(235, 235, 235);  // very light gray
  setLabel("Add your text here");
}

AnnotationItem::AnnotationItem(const AnnotationItem &i)
  : MolRxnBaseItem(i)
{}

PathwayCanvasItem *
AnnotationItem::clone() const {
  return new AnnotationItem(*this);
}

QWidget *
AnnotationItem::propertiesDialog(QWidget *parent, const Simulation &) {
#ifdef GUI
  if(!pd_) {
    pd_ = new AnnotationDialog(parent, *this);
    pd_->show();
    return pd_;
  } else {
    ((AnnotationDialog*)pd_)->updateDialog();
    pd_->show();
  }
#endif
  return NULL;
}

void
AnnotationItem::updatePropertiesFromDialog() {
#ifdef GUI
  if(pd_) {
    ((AnnotationDialog*)pd_)->updateItem();
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
AnnotationItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement ann = doc.createElement("Annotation");
  n.appendChild(ann);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  ann.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  ann.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  ann.appendChild(e); e.appendChild(t);

  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    ann.appendChild(e); e.appendChild(t);
  }
  
  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    ann.appendChild(e);
  }
}

int
AnnotationItem::rtti() const {
  return AnnotationItem::RTTI;
}

ControlVariableItem::ControlVariableItem(QWidget *p)
  : MoleculeItem(p),
    cur_env_(0),
    num_env_(0)
{
  bg_ = def_bg_ = QColor(50, 210, 50);
  cap_ = "Control Variable";
  tag_ = "ControlVariable";
}

ControlVariableItem::ControlVariableItem(const ControlVariableItem &i)
  : MoleculeItem(i),
    cur_env_(i.cur_env_),
    num_env_(i.num_env_)
{}

void
ControlVariableItem::setCurEnvironment(int e) {
  if(e < 0 || num_env_ <= e) abort();
  cur_env_ = e;
}

void
ControlVariableItem::setNumEnvironment(int e) {
  num_env_ = e;
}

void
ControlVariableItem::init() {
}

int
ControlVariableItem::sub_rtti() const {
  return ControlVariableItem::sub_RTTI;
}

SquareWaveControlVariableItem::SquareWaveControlVariableItem(QWidget *p)
  : ControlVariableItem(p),
    values_(NULL),
    c_values_(NULL),
    duration_(NULL),
    transition_(NULL),
    phase_shift_(NULL),
    noise_(NULL)
{
  cap_ = "Square Wave Control Variable";
  tag_ = "SquareWaveControlVariable";
  setNumEnvironment(1);
  values_[0]      = 0.0;  values_[1]     = 1.0;
  c_values_[0]    = 0.0;  c_values_[1]   = 1.0;
  duration_[0]    = 10.0; duration_[1]   = 10.0;
  transition_[0]  = 2.0;  transition_[1] = 2.0;
  phase_shift_[0] = 0.0;
  noise_[0]       = 0.0;
  //initial_con_ = values_[0];
}

SquareWaveControlVariableItem::SquareWaveControlVariableItem(const SquareWaveControlVariableItem &i)
  : ControlVariableItem(i)
{
  if(i.values_) {
    values_ = new double[num_env_ * 2];
    for(int j=0; j<num_env_*2; j++) {
      values_[j] = i.values_[j];
    }
  }
  if(i.c_values_) {
    c_values_ = new double[num_env_ * 2];
    for(int j=0; j<num_env_*2; j++) {
      c_values_[j] = i.c_values_[j];
    }
  }
  if(i.duration_) {
    duration_ = new double[num_env_ * 2];
    for(int j=0; j<num_env_*2; j++) {
      duration_[j] = i.duration_[j];
    }
  }
  if(i.transition_) {
    transition_ = new double[num_env_ * 2];
    for(int j=0; j<num_env_*2; j++) {
      transition_[j] = i.transition_[j];
    }
  }
  if(i.phase_shift_) {
    phase_shift_ = new double[num_env_];
    for(int j=0; j<num_env_; j++) {
      phase_shift_[j] = i.phase_shift_[j];
    }
  }
  if(i.noise_) {
    noise_ = new double[num_env_];
    for(int j=0; j<num_env_; j++) {
      noise_[j] = i.noise_[j];
    }
  }
}
    
SquareWaveControlVariableItem::~SquareWaveControlVariableItem() {
  if(pd_) {
    delete pd_;
    pd_ = NULL;
  }
  if(values_)      delete[] values_;
  if(c_values_)    delete[] c_values_;
  if(duration_)    delete[] duration_;
  if(transition_)  delete[] transition_;
  if(phase_shift_) delete[] phase_shift_;
  if(noise_)       delete[] noise_;
}

PathwayCanvasItem *
SquareWaveControlVariableItem::clone() const {
  return new SquareWaveControlVariableItem(*this);
}

QWidget *
SquareWaveControlVariableItem::propertiesDialog(QWidget *parent, const Simulation &s) {
#ifdef GUI
  if(!pd_) {
    pd_ = new SquareWaveControlVariableDialog(parent, s, *this);
    pd_->show();
    return pd_;
  } else {
    ((SquareWaveControlVariableDialog*)pd_)->updateDialog(s);
    pd_->show();
  }
#endif
  return NULL;
}

void
SquareWaveControlVariableItem::updatePropertiesDialog(const Simulation &s) {
#ifdef GUI
  if(pd_ && pd_->isVisible()) ((SquareWaveControlVariableDialog*)pd_)->updateDialog(s);
#endif
}

void
SquareWaveControlVariableItem::updatePropertiesFromDialog() {
#ifdef GUI
  if(pd_) {
    ((SquareWaveControlVariableDialog*)pd_)->updateItem();
    //initial_con_ = values_[cur_env_*2 + 0];
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
SquareWaveControlVariableItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2;
  QDomText t;

  QDomElement cv = doc.createElement("SquareWaveControlVariable");
  n.appendChild(cv);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  cv.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  cv.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  cv.appendChild(e); e.appendChild(t);

  for(int i=0; i<num_env_; i++) {
    e = doc.createElement("Parameters");
    cv.appendChild(e);

    e2 = doc.createElement("Number");
    t = doc.createTextNode(QObject::tr("%1").arg(i));
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Values");
    t = doc.createTextNode(QObject::tr("%1 %2").arg(values_[i*2]).arg(values_[i*2+1]));
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Duration");
    t = doc.createTextNode(QObject::tr("%1 %2").arg(duration_[i*2]).arg(duration_[i*2+1]));
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("TransitionTime");
    t = doc.createTextNode(QObject::tr("%1 %2").arg(transition_[i*2]).arg(transition_[i*2+1]));
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("PhaseShift");
    t = doc.createTextNode(QObject::tr("%1").arg(phase_shift_[i]));
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Noise");
    t = doc.createTextNode(QObject::tr("%1").arg(noise_[i]));
    e.appendChild(e2); e2.appendChild(t);
  }

  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    cv.appendChild(e); e.appendChild(t);
  }
  
  if(exported_) {
    e = doc.createElement("Exported");
    cv.appendChild(e);
  }

  if(!plotted_) {
    e = doc.createElement("NotPlotted");
    cv.appendChild(e);
  }

  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    cv.appendChild(e);
  }
}

void
SquareWaveControlVariableItem::setCurEnvironment(int e) {
  if(e < 0 || num_env_ <= e) abort();
  cur_env_ = e;
  //initial_con_ = values_[cur_env_*2 + 0];
}

void
SquareWaveControlVariableItem::setNumEnvironment(int e) {
  if(num_env_ == e) {
  } else if(num_env_ > e) {
    num_env_ = e;
  } else if(num_env_ < e) {
    double *t_values      = new double[e * 2];
    double *t_c_values    = new double[e * 2];
    double *t_duration    = new double[e * 2];
    double *t_transition  = new double[e * 2];
    double *t_phase_shift = new double[e];
    double *t_noise       = new double[e];

    if(num_env_ > 0) {
      int i, j;
      for(i=0; i<num_env_; i++) {
	for(j=0; j<2; j++) {
	  t_values[i*2+j]     = values_[i*2+j];
	  t_duration[i*2+j]   = duration_[i*2+j];
	  t_transition[i*2+j] = transition_[i*2+j];
	}
	t_phase_shift[i]     = phase_shift_[i];
	t_noise[i]           = noise_[i];
      }
      for(i=num_env_; i<e; i++) {
	for(j=0; j<2; j++) {
	  t_values[i*2+j]     = values_[(num_env_-1)*2+j];
	  t_duration[i*2+j]   = duration_[(num_env_-1)*2+j];
	  t_transition[i*2+j] = transition_[(num_env_-1)*2+j];
	}
	t_phase_shift[i]      = phase_shift_[num_env_-1];
	t_noise[i]            = noise_[num_env_-1];
      }
    }
    
    if(values_)      delete[] values_;
    if(c_values_)    delete[] c_values_;
    if(duration_)    delete[] duration_;
    if(transition_)  delete[] transition_;
    if(phase_shift_) delete[] phase_shift_;
    if(noise_)       delete[] noise_;

    num_env_ = e;
    
    values_      = t_values;
    c_values_    = t_c_values;
    duration_    = t_duration;
    transition_  = t_transition;
    phase_shift_ = t_phase_shift;
    noise_       = t_noise;
  }
}

double
SquareWaveControlVariableItem::concentration(double t) const {
  const double *val = c_values_   + cur_env_*2;
  const double *dur = duration_   + cur_env_*2;
  const double *trn = transition_ + cur_env_*2;
  double v = squarewave(t, val, dur, trn, phase_shift_[cur_env_]);
  if(v < 0.0) v = 0.0;
  return v;
}

void
SquareWaveControlVariableItem::setValues(double v1, double v2) {
  values_[cur_env_*2 + 0] = v1;
  values_[cur_env_*2 + 1] = v2;
  //initial_con_ = values_[cur_env_*2 + 0];
}

void
SquareWaveControlVariableItem::values(double v[2]) {
  v[0] = values_[cur_env_*2 + 0];
  v[1] = values_[cur_env_*2 + 1];
}

void
SquareWaveControlVariableItem::setDuration(double v1, double v2) {
  duration_[cur_env_*2 + 0] = v1;
  duration_[cur_env_*2 + 1] = v2;
}

void
SquareWaveControlVariableItem::duration(double v[2]) {
  v[0] = duration_[cur_env_*2 + 0];
  v[1] = duration_[cur_env_*2 + 1];
}

void
SquareWaveControlVariableItem::setTransitionTime(double v1, double v2) {
  transition_[cur_env_*2 + 0] = v1;
  transition_[cur_env_*2 + 1] = v2;
}

void
SquareWaveControlVariableItem::transitionTime(double v[2]) {
  v[0] = transition_[cur_env_*2 + 0];
  v[1] = transition_[cur_env_*2 + 1];
}

void
SquareWaveControlVariableItem::setPhaseShift(double v) {
  phase_shift_[cur_env_] = v;
}

double
SquareWaveControlVariableItem::phaseShift() {
  return phase_shift_[cur_env_];
}

void
SquareWaveControlVariableItem::setNoise(double v) {
  noise_[cur_env_] = v;
}

double
SquareWaveControlVariableItem::noise() {
  return noise_[cur_env_];
}

void
SquareWaveControlVariableItem::setParameters(const double *values,
					     const double *duration,
					     const double *transition,
					     const double *phase_shift,
					     const double *noise) {
  for(int i=0; i<num_env_; i++) {
    for(int j=0; j<2; j++) {
      values_[i*2+j]     = values[i*2+j];
      duration_[i*2+j]   = duration[i*2+j];
      transition_[i*2+j] = transition[i*2+j];
    }
    phase_shift_[i]      = phase_shift[i];
    noise_[i]            = noise[i];
  }  
}

void
SquareWaveControlVariableItem::parameters(double *values,
					  double *duration,
					  double *transition,
					  double *phase_shift,
					  double *noise) const {
  for(int i=0; i<num_env_; i++) {
    for(int j=0; j<2; j++) {
      values[i*2+j]     = values_[i*2+j];
      duration[i*2+j]   = duration_[i*2+j];
      transition[i*2+j] = transition_[i*2+j];
    }
    phase_shift[i]      = phase_shift_[i];
    noise[i]            = noise_[i];
  }  
}

void
SquareWaveControlVariableItem::init() {
  for(int i=0; i<num_env_; i++) {
    for(int j=0; j<2; j++) {
      c_values_[i*2+j] = values_[i*2+j];
      if(noise_[i] > 0.0) {
	c_values_[i*2+j] += rng.normal(0.0, noise_[i]);
	if(c_values_[i*2+j] < 0.0) c_values_[i*2+j] = 0.0;
      }
    }
  }
}

QString
SquareWaveControlVariableItem::tip() const {
  const double *val = values_     + cur_env_*2;
  const double *dur = duration_   + cur_env_*2;
  const double *trn = transition_ + cur_env_*2;
  double period = dur[0] + trn[0] + dur[1] + trn[1];

  QString str;
  str.sprintf("%s\n"
	      "Name: %s\n"
	      //"Initial Concentration: %f\n"
	      "Current Environment: %d (of %d)\n"
	      "Values: %f %f\n"
	      "Duration: %f %f\n"
	      "Transition Time: %f %f\n"
	      "Phase Shift: %f\n"
	      "Gaussian Noise (std. dev.): %f\n"
	      "Period: %f",
	      (const char*)cap_, (const char*)label_, //initial_con_,
	      cur_env_+1, num_env_,
	      val[0], val[1],
	      dur[0], dur[1],
	      trn[0], trn[1],
	      phase_shift_[cur_env_],
	      noise_[cur_env_],
	      period);
  if(!plotted_) str += "\nNot plotted";
  if(!user_initialized_) str += "\nNot initialized";
  if(exported_) str += "\nExported";
  
  return str;
}

InterpolatedControlVariableItem::InterpolatedControlVariableItem(QWidget *p)
  : ControlVariableItem(p)
{
  cap_ = "Interpolated Control Variable";
  tag_ = "InterpolatedControlVariable";
  setNumEnvironment(1);
  //initial_con_ = values_[0];
}

InterpolatedControlVariableItem::InterpolatedControlVariableItem(const InterpolatedControlVariableItem &i)
  : ControlVariableItem(i),
    values_(i.values_)
{
}
    
InterpolatedControlVariableItem::~InterpolatedControlVariableItem() {
  if(pd_) {
    delete pd_;
    pd_ = NULL;
  }
}

PathwayCanvasItem *
InterpolatedControlVariableItem::clone() const {
  return new InterpolatedControlVariableItem(*this);
}

QWidget *
InterpolatedControlVariableItem::propertiesDialog(QWidget *parent, const Simulation &s) {
#ifdef GUI
  if(!pd_) {
    pd_ = new InterpolatedControlVariableDialog(parent, s, *this);
    pd_->show();
    return pd_;
  } else {
    ((InterpolatedControlVariableDialog*)pd_)->updateDialog(s);
    pd_->show();
  }
#endif
  return NULL;
}

void
InterpolatedControlVariableItem::updatePropertiesDialog(const Simulation &s) {
#ifdef GUI
  if(pd_ && pd_->isVisible()) ((InterpolatedControlVariableDialog*)pd_)->updateDialog(s);
#endif
}

void
InterpolatedControlVariableItem::updatePropertiesFromDialog() {
#ifdef GUI
  if(pd_) {
    ((InterpolatedControlVariableDialog*)pd_)->updateItem();
    //initial_con_ = values_[cur_env_*2 + 0];
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
InterpolatedControlVariableItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2, e2a, e3, e4;
  QDomText t;

  QDomElement cv = doc.createElement("InterpolatedControlVariable");
  n.appendChild(cv);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  cv.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  cv.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  cv.appendChild(e); e.appendChild(t);

  for(int i=0; i<num_env_; i++) {
    e = doc.createElement("Parameters");
    cv.appendChild(e);

    e2 = doc.createElement("Values");

    values_.save(doc, e2);
    
    e.appendChild(e2);
  }

  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    cv.appendChild(e); e.appendChild(t);
  }
  
  if(exported_) {
    e = doc.createElement("Exported");
    cv.appendChild(e);
  }

  if(!plotted_) {
    e = doc.createElement("NotPlotted");
    cv.appendChild(e);
  }

  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    cv.appendChild(e);
  }
}

void
InterpolatedControlVariableItem::setCurEnvironment(int e) {
  if(e < 0 || num_env_ <= e) abort();
  cur_env_ = e;
  //initial_con_ = values_[cur_env_*2 + 0];
}

void
InterpolatedControlVariableItem::setNumEnvironment(int e) {
  if(num_env_ == e) {
  } else if(num_env_ > e) {
    num_env_ = e;
  } else if(num_env_ < e) {      
    values_.expand(0, e - num_env_, 0.0);
    
    if(num_env_ > 0) {
      int i, j;
      for(i=num_env_; i<e; i++) {
	for(j=0; j<values_.nrows(); j++) {
	  values_(j, i+1) = values_(j, num_env_);
	}
      }
    }
    
    num_env_ = e;
  }
}

double
InterpolatedControlVariableItem::concentration(double t) const {
  if(values_.empty()) {
    return 0.0;
  }
  if(t <= values_(0, 0)) {
    return values_(0, cur_env_+1);
  }
  for(int i=1; i<values_.nrows(); i++) {
    if(values_(i-1, 0) < t && t <= values_(i, 0)) {
      double vd = values_(i, cur_env_+1) - values_(i-1, cur_env_+1);
      double p = (t - values_(i-1, 0)) / (values_(i, 0) - values_(i-1, 0));
      return values_(i-1, cur_env_+1) + vd * p;
    }
  }
  return values_(values_.nrows()-1, cur_env_+1);
}

void
InterpolatedControlVariableItem::setValues(const Matrix &v) {
  values_ = v;
  //initial_con_ = values_[cur_env_*2 + 0];
}

void
InterpolatedControlVariableItem::values(Matrix &v) {
  v = values_;
}

QString
InterpolatedControlVariableItem::tip() const {
  QString str;
  str.sprintf("%s\n"
	      "Name: %s\n"
	      //"Initial Concentration: %f\n"
	      "Current Environment: %d (of %d)\n"
	      "Values: (not implemented in tip() yet)",
	      (const char*)cap_, (const char*)label_, //initial_con_,
	      cur_env_+1, num_env_);
  if(!plotted_) str += "\nNot plotted";
  if(!user_initialized_) str += "\nNot initialized";
  if(exported_) str += "\nExported";

  return str;
}

ReactionItem::ReactionItem(QWidget *p)
  : MolRxnBaseItem(p),
    reversible_(false)
{
  bg_ = def_bg_ = QColor(211, 211, 211);
  label_ = "Rxn";
  cap_ = "Generic Reaction";
}

ReactionItem::ReactionItem(const ReactionItem &i)
  : MolRxnBaseItem(i),
    reversible_(i.reversible_)
{}

ReactionItem::~ReactionItem() {}

void
ReactionItem::equation(QString &lhs, QString &rhs, bool include_k) const {
  lhs = "Undefined equation type";
  rhs = "Undefined equation type";
}

QString
ReactionItem::equation() const {
  return "Undefined equation type";
}

bool
ReactionItem::reversible() const {
  return reversible_;
}

void
ReactionItem::setReversible(bool r) {
  reversible_ = r;
#ifdef GUI
  if(parent_) parent_->update();
#endif
}

int
ReactionItem::rtti() const {
  return ReactionItem::RTTI;
}

MassActionReactionItem::MassActionReactionItem(QWidget *p)
  : ReactionItem(p),
    kf_(1.0),
    kr_(1.0)
{
  label_ = "MA Rxn";
  cap_ = "Mass Action Reaction";
}

MassActionReactionItem::MassActionReactionItem(const MassActionReactionItem &i)
  : ReactionItem(i),
    kf_(i.kf_),
    kr_(i.kr_)
{}

MassActionReactionItem::~MassActionReactionItem() {
  if(pd_) {
    delete pd_;
    pd_ = NULL;
  }
}

PathwayCanvasItem *
MassActionReactionItem::clone() const {
  return new MassActionReactionItem(*this);
}

void
MassActionReactionItem::equation(QString &lhs, QString &rhs, bool include_k) const {
  if(include_k) {
    lhs = QString("%1").arg(Kf());
    rhs = QString("%1").arg(Kr());
  } else {
    lhs = "";
    rhs = "";
  }

  QListIterator<EdgeBaseItem> i(edges_);
  for(; i.current(); ++i) {
    if(!isReactionEdge(i.current())) continue;
    
    ReactionEdgeItem *edge = ((ReactionEdgeItem*)i.current());
    QString term;
    if(edge->molecule()) {
      QString label = QString("[%1]").arg(edge->molecule()->label());
      if(edge->coefficient() != 1) {
	term = QString("%1^%2").arg(label).arg(edge->coefficient());
      } else {
	term = label;
      }
    } else {
      term = "(Unattached)";
    }
    if(edge->moleculeIsReactant()) {
      if(lhs.length() > 0) {
	lhs += "*";
      }
      lhs += term;
    } else if(edge->moleculeIsProduct()) {
      if(rhs.length() > 0) {
	rhs += "*";
      }
      rhs += term;
    }
  }
}

QString
MassActionReactionItem::equation() const {
  QString lhs, rhs, rxn_str;

  QListIterator<EdgeBaseItem> i(edges_);
  for(; i.current(); ++i) {
    if(!isReactionEdge(i.current())) continue;
    
    ReactionEdgeItem *edge = ((ReactionEdgeItem*)i.current());
    QString term;
    if(edge->molecule()) {
      QString label = edge->molecule()->label();
      if(label.find(' ') != -1) {
	label = QString("(%1)").arg(label);
      }
      if(edge->coefficient() != 1) {
	term = QString("%1%2").arg(edge->coefficient()).arg(label);
      } else {
	term = label;
      }
    } else {
      term = "(Unattached)";
    }
    if(edge->moleculeIsReactant()) {
      if(lhs.length() > 0) {
	lhs += " + ";
      }
      lhs += term;
    } else if(edge->moleculeIsProduct()) {
      if(rhs.length() > 0) {
	rhs += " + ";
      }
      rhs += term;
    }
  }

  if(lhs.length() > 0 || rhs.length() > 0) {
    if(reversible_) {
      rxn_str = lhs + " <-> " + rhs;
    } else {
      rxn_str = lhs + " --> " + rhs;
    }
  }

  return rxn_str;
}

QWidget *
MassActionReactionItem::propertiesDialog(QWidget *parent, const Simulation &) {
#ifdef GUI
  if(!pd_) {
    pd_ = new ReactionDialog(parent, *this);
    pd_->show();
    return pd_;
  } else {
    ((ReactionDialog*)pd_)->updateDialog();
    pd_->show();
  }
#endif
  return NULL;
}

void
MassActionReactionItem::updatePropertiesDialog(const Simulation &) {
#ifdef GUI
  if(pd_ && pd_->isVisible()) ((ReactionDialog*)pd_)->updateDialog();
#endif
}

void
MassActionReactionItem::updatePropertiesFromDialog() {
#ifdef GUI
  if(pd_) {
    ((ReactionDialog*)pd_)->updateItem();
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
MassActionReactionItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement rxn = doc.createElement("MassActionReaction");
  n.appendChild(rxn);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  rxn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  rxn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  rxn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Kf");
  t = doc.createTextNode(QObject::tr("%1").arg(kf_));
  rxn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Kr");
  t = doc.createTextNode(QObject::tr("%1").arg(kr_));
  rxn.appendChild(e); e.appendChild(t);

  if(reversible_) {
    e = doc.createElement("Reversible");
    rxn.appendChild(e);
  }
  
  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    rxn.appendChild(e); e.appendChild(t);
  }

  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    rxn.appendChild(e);
  }
  
  e = doc.createElement("Edges");
  QListIterator<EdgeBaseItem> i(edges_);
  for(; i.current(); ++i) {
    i.current()->save(doc, e);
  }
  rxn.appendChild(e);
}

void
MassActionReactionItem::setKf(double v) {
  kf_ = v;
}

double
MassActionReactionItem::Kf() const {
  return kf_;
}

void
MassActionReactionItem::setKr(double v) {
  kr_ = v;
}

double
MassActionReactionItem::Kr() const {
  return kr_;
}

QString
MassActionReactionItem::tip() const {
  QString str;
  str.sprintf("%s\n"
	      "Name: %s\n"
	      "Equation: %s\n",
	      (const char*)cap_,
	      (const char*)label_,
	      (const char*)equation());

  str += QObject::tr("Kinetic Constant (Kf): %1").arg(kf_);
  if(reversible_) {
    str += QObject::tr("  (Kr): %1").arg(kr_);
  }    
  
  if(!user_initialized_) str += "\nNot initialized";
  
  return str;
}

InternalNodeItem::InternalNodeItem(QWidget *p)
  : MolRxnBaseItem(p),
    r_(2),
    edge_(NULL)
{
  points_.resize(4);
  points_.setPoint(0, -r_, -r_);
  points_.setPoint(1,  r_, -r_);
  points_.setPoint(2,  r_,  r_);
  points_.setPoint(3, -r_,  r_);
}

InternalNodeItem::InternalNodeItem(const InternalNodeItem &i)
  : MolRxnBaseItem(i),
    r_(i.r_),
    edge_(NULL)
{
}

InternalNodeItem::~InternalNodeItem() {
  //parent_->removeNode(this);
}

PathwayCanvasItem *
InternalNodeItem::clone() const {
  return new InternalNodeItem(*this);
}

QWidget *
InternalNodeItem::propertiesDialog(QWidget *, const Simulation &) {
  return NULL;
}

void
InternalNodeItem::updatePropertiesFromDialog() {
}

void
InternalNodeItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement in = doc.createElement("InternalNode");
  n.appendChild(in);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  in.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  in.appendChild(e); e.appendChild(t);
}

const EdgeBaseItem *
InternalNodeItem::edge() const {
  return edge_;
}

void
InternalNodeItem::setEdge(const EdgeBaseItem *e) {
  edge_ = e;
}

int
InternalNodeItem::rtti() const {
  return InternalNodeItem::RTTI;
}

void
InternalNodeItem::draw(QPainter *p) {
  p->save();
  p->scale(sf_, sf_);
#ifdef GUI
  p->setFont(font_);
#endif
  if(selected_) {
    p->setPen(QPen(Qt::red, 4));
  } else {
    p->setPen(Qt::black);
  }
  p->drawPie(x_-r_, y_-r_, r_*2, r_*2, 0, 5760);
  p->restore();
}
  





EdgeBaseItem::EdgeBaseItem(QWidget *p)
  : PathwayCanvasItem(p)
{}

EdgeBaseItem::EdgeBaseItem(const EdgeBaseItem &i)
  : PathwayCanvasItem(i)
{}

EdgeBaseItem::~EdgeBaseItem() {
}

QWidget *
EdgeBaseItem::propertiesDialog(QWidget *, const Simulation &) {
  return NULL;
}

void
EdgeBaseItem::updatePropertiesFromDialog() {
}

MolRxnBaseItem *
EdgeBaseItem::insertPoint(InternalNodeItem *p) {
  //puts("Add internal node");
  p->setEdge(this);
  QPoint p3(p->x(), p->y());
  MolRxnBaseItem *i, *j, *k = NULL;
  double min_d = DBL_MAX;
  for(i=ipoints_.first(), j=ipoints_.next(); j; i=j, j=ipoints_.next()) {
    if(i && j) {
      QPoint p1(i->x(), i->y()), p2(j->x(), j->y());
      double l = ((p2.x() - p1.x())*(p2.x() - p1.x()) +
		  (p2.y() - p1.y())*(p2.y() - p1.y()));
      double r = ((p3.x() - p1.x())*(p2.x() - p1.x()) +
		  (p3.y() - p1.y())*(p2.y() - p1.y())) / l;
      if(r < 0.0 || 1.0 < r) continue;
      double s = ((p1.y() - p3.y())*(p2.x() - p1.x()) -
		  (p1.x() - p3.x())*(p2.y() - p1.y())) / l;
      double d = fabs(s) * sqrt(l);
      //printf("%d,%d  %d,%d   %f %f %f %f\n", p1.x(), p1.y(), p2.x(), p2.y(), l, r, s, d);
      //if(d < 4.0) return true;
      if(d < min_d) {
	k = i;
	min_d = d;
      }
    }   
  }
  //printf("Min distance: %f\n", min_d);
  if(k) {
    int ind = ipoints_.find(k);
    if(ind != -1) {
      ipoints_.insert(ind+1, p);
    }
  }
  return p;
}

void
EdgeBaseItem::removeNode(MolRxnBaseItem *p) {
  ipoints_.remove(p);
}

bool
EdgeBaseItem::contains(QPoint p) {
  p /= sf_;
  
  MolRxnBaseItem *i, *j;
  for(i=ipoints_.first(), j=ipoints_.next(); j; i=j, j=ipoints_.next()) {
    if(i && j) {
      if(distanceToLine(QPoint(i->x(), i->y()), QPoint(j->x(), j->y()),
			p) < 4.0) return true;
    }   
  }
  return false;
}

double
EdgeBaseItem::distanceToLine(const QPoint &p1, const QPoint &p2, const QPoint &p3) {
  //printf("(%d %d)-(%d %d)  to  (%d %d)\n", p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
  double l = ((p2.x() - p1.x())*(p2.x() - p1.x()) +
	      (p2.y() - p1.y())*(p2.y() - p1.y()));
  double r = ((p3.x() - p1.x())*(p2.x() - p1.x()) +
	      (p3.y() - p1.y())*(p2.y() - p1.y())) / l;
  // Verify that r is internal to the line (p1, p2)
  if(r < 0.0 || 1.0 < r) return DBL_MAX;
  double s = ((p1.y() - p3.y())*(p2.x() - p1.x()) -
	      (p1.x() - p3.x())*(p2.y() - p1.y())) / l;
  return fabs(s) * sqrt(l);
}

bool
EdgeBaseItem::collidesWith(const PathwayCanvasItem *) {
  return false;
}

QList<MolRxnBaseItem>
EdgeBaseItem::nodes() const {
  return ipoints_;
}

void
EdgeBaseItem::move(MolRxnBaseItem *) {
#if 0
  QRect bb = item->boundingRect();
  int x = (int)(bb.x() + bb.width()/2  + 0.5);
  int y = (int)(bb.y() + bb.height()/2 + 0.5);
  if(isMolecule(item)) {
    ipoints_[molecule_index_] = QPoint(x, y);
  } else if(isReaction(item)) {
    ipoints_[reaction_index_] = QPoint(x, y);
  }
#endif
}

void
EdgeBaseItem::setSelected(bool s) {
  selected_ = s;
  MolRxnBaseItem *i;
  for(i=ipoints_.first(); i; i=ipoints_.next()) {
    if(isNode(i)) {
      i->setSelected(s);
    }
  }
}

QRect
EdgeBaseItem::boundingRect() const {
  QPointArray points(ipoints_.count());
  MolRxnBaseItem *mr;
  QListIterator<MolRxnBaseItem> it(ipoints_);
  int j=0;
  while((mr = it.current())) {
    points[j++] = QPoint(mr->x(), mr->y());
    ++it;
  }

  return points.boundingRect();
}










ReactionEdgeItem::ReactionEdgeItem(QWidget *p)
  : EdgeBaseItem(p),
    coefficient_(1)
{}

ReactionEdgeItem::ReactionEdgeItem(const ReactionEdgeItem &i)
  : EdgeBaseItem(i),
    //ipoints_(i.ipoints_),
    coefficient_(i.coefficient_)
{}

ReactionEdgeItem::~ReactionEdgeItem() {
  MolRxnBaseItem *mol = molecule();
  MolRxnBaseItem *rxn = reaction();
  
  if(mol) mol->removeEdge(this);
  if(rxn) rxn->removeEdge(this);
}

PathwayCanvasItem *
ReactionEdgeItem::clone() const {
  return new ReactionEdgeItem(*this);
}

void
ReactionEdgeItem::save(QDomDocument &doc, QDomElement &n) const {
  if(!reaction() || !molecule()) {
    pneDebug("Incomplete reaction edge ignored during save");
    return;
  }

  QDomElement e, e2;
  QDomText t;

  QDomElement re = doc.createElement("ReactionEdge");
  n.appendChild(re);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  re.appendChild(e); e.appendChild(t);

  if(moleculeIsReactant()) {
    e = doc.createElement("Reactant");
    re.appendChild(e);

    e2 = doc.createElement("Label");
    t = doc.createTextNode(molecule()->label());
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Id");
    t = doc.createTextNode(QObject::tr("%1").arg(molecule()->id()));
    e.appendChild(e2); e2.appendChild(t);
  } else if(moleculeIsProduct()) {
    e = doc.createElement("Product");
    re.appendChild(e);

    e2 = doc.createElement("Label");
    t = doc.createTextNode(molecule()->label());
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Id");
    t = doc.createTextNode(QObject::tr("%1").arg(molecule()->id()));
    e.appendChild(e2); e2.appendChild(t);
  } else {
    abort();
  }

  e = doc.createElement("Coefficient");
  t = doc.createTextNode(QObject::tr("%1").arg(coefficient_));
  re.appendChild(e); e.appendChild(t);

  QListIterator<MolRxnBaseItem> i(ipoints_);
  if(i.count() > 2) {
    e = doc.createElement("InternalNodes");
    re.appendChild(e);
    for(; i.current(); ++i) {
      if(isNode(i.current())) {
	i.current()->save(doc, e);
      }
    }
  }
}

void
ReactionEdgeItem::appendNode(PathwayCanvasItem *p) {
  if(isMolRxn(p)) {
    MolRxnBaseItem *q = (MolRxnBaseItem*)p;
    ipoints_.append(q);
    q->addEdge(this);
  } else if(isNode(p)) {
    InternalNodeItem *q = (InternalNodeItem*)p;
    ipoints_.append(q);
    q->setEdge(this);
  } else {
    abort();
  }
}

MolRxnBaseItem *
ReactionEdgeItem::addPoint(MolRxnBaseItem *p) {
  if(ipoints_.isEmpty()) {
    if(!isMolRxn(p)) {
      return NULL;
    }
    ipoints_.append(p);
    p->addEdge(this);
  } else {
    if((isMolecule(p) && reaction()) || (isReaction(p) && molecule())) {
      PathwayCanvasItem *p2 = ipoints_.last();
      ipoints_.removeLast();
      if(p2) delete p2;
      ipoints_.append(p);
      p->addEdge(this);
      return NULL;
    }
  }
  InternalNodeItem *in = new InternalNodeItem(parent_);
  in->setEdge(this);
  ipoints_.append(in);
  return in;
}

void
ReactionEdgeItem::drawArrowHead(QPainter *p,
				MolRxnBaseItem *i, MolRxnBaseItem *j) {
  if(i && j) {
    QPoint p1(j->x(), j->y()), p2(i->x(), i->y());
    if(!isNode(i)) {
      p2 = i->intersection(p1, p2);
    }
    
    if(p1 != p2) {
#ifndef WIN32
      float r = 2.0*M_PI / 360.0;
#else
      float r = 0.01745329f;
#endif
      float l = 10.0;
      float theta, alpha = 20.0 * r;
      theta = atan2(double(p2.y() - p1.y()), double(p2.x() - p1.x()));
      QPointArray tri(3);
      tri.setPoint(0, p2);
      tri.setPoint(1,
		   p2.x() - (int)(l * cos(theta - alpha) + 0.5),
		   p2.y() - (int)(l * sin(theta - alpha) + 0.5));
      tri.setPoint(2,
		   p2.x() - (int)(l * cos(theta + alpha) + 0.5),
		   p2.y() - (int)(l * sin(theta + alpha) + 0.5));
      p->drawPolygon(tri);

      MolRxnBaseItem *mol = molecule();
      if(mol && coefficient_ > 1) {
	double x1 = p1.x(), x2 = p2.x();
	double y1 = p1.y(), y2 = p2.y();
	double len = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	//printf("p1:%d %d     p2:%d %d\n", p1.x(), p1.y(), p2.x(), p2.y());
	//printf("len: %f\n", len);
	//printf("p3:%d %d\n", (p1 - p2).x(), (p1 - p2).y());
	//printf("p3:%d %d\n", ((p1 - p2) / len).x(), ((p1 - p2) / len).y());
	//printf("p3:%d %d\n", ((p1 - p2) / len * 24).x(), ((p1 - p2) / len * 24).y());
	//printf("p3:%d %d\n", ((p1 - p2) / len * 24 + p2).x(), ((p1 - p2) / len * 24 + p2).y());
	QPoint p3(int((x1 - x2) / len * 24 + x2 + 0.5),
		  int((y1 - y2) / len * 24 + y2 + 0.5));
	//printf("p3:%d %d\n", p3.x(), p3.y());

	//p->drawText(p3, "Hi");
	QString str;
	str.setNum(coefficient_);
	p->drawText(p3, str);
      }
    }
  }
}

void
ReactionEdgeItem::draw(QPainter *p) {
  p->save();
  p->scale(sf_, sf_);
#ifdef GUI
  p->setFont(font_);
#endif
  if(selected_) {
    p->setPen(QPen(Qt::red, 4));
    p->setBrush(Qt::red);
  } else {
    p->setPen(Qt::black);
    p->setBrush(Qt::black);
  }
  MolRxnBaseItem *i, *j;
  for(i=ipoints_.first(), j=ipoints_.next(); j; i=j, j=ipoints_.next()) {
    p->drawLine(i->x(), i->y(), j->x(), j->y());
  }

  if(reaction() && reaction()->reversible()) {
    i = ipoints_.first();
    j = ipoints_.next();
    drawArrowHead(p, i, j);
  }

  i = ipoints_.last();
  j = ipoints_.prev();
  drawArrowHead(p, i, j);
  
  for(i=ipoints_.first(); i; i=ipoints_.next()) {
    if(isNode(i)) {
      i->draw(p);
    }
  }

  p->restore();
}

int
ReactionEdgeItem::rtti() const {
  return ReactionEdgeItem::RTTI;
}

MoleculeItem *
ReactionEdgeItem::molecule() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toFirst();
  const PathwayCanvasItem *q = i.toLast();
  if(p && isMolecule(p)) return (MoleculeItem*)p;
  else if(q && isMolecule(q)) return (MoleculeItem*)q;
  return NULL;
}

ReactionItem *
ReactionEdgeItem::reaction() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toFirst();
  const PathwayCanvasItem *q = i.toLast();
  if(p && isReaction(p)) return (ReactionItem*)p;
  else if(q && isReaction(q)) return (ReactionItem*)q;
  return NULL;
}

bool
ReactionEdgeItem::moleculeIsReactant() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toFirst();
  return p && isMolecule(p);
}

bool
ReactionEdgeItem::moleculeIsProduct() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toLast();
  return p && isMolecule(p);
}

int
ReactionEdgeItem::coefficient() const {
  return coefficient_;
}

void
ReactionEdgeItem::setCoefficient(int coeff) {
  coefficient_ = coeff;
}



SE_t::SE_t()
  : m(NULL),
    e(NULL)
{}

SE_t::SE_t(MoleculeItem *m1, SubnetworkEdgeItem *e1)
  : m(m1),
    e(e1)
{}



SubnetworkItem::SubnetworkItem(QWidget *p)
  : MolRxnBaseItem(p)
{
  bg_ = def_bg_ = QColor(250, 200, 100);
  cap_ = "Subnetwork";
  label_ = "Subnet";
  snugUp();
  //items_.setAutoDelete(true);
  exported_.setAutoDelete(true);
}

SubnetworkItem::SubnetworkItem(const SubnetworkItem &i)
  : MolRxnBaseItem(i)
{
  //items_.setAutoDelete(true);
  items_ = i.items_;
  items_ = copy(items_);
  tpoints_ = i.tpoints_.copy();

  buildExportList(exported_);
  exported_.setAutoDelete(true);
  
  snugUp();
}

SubnetworkItem::~SubnetworkItem() {
  if(!pd_) {
    items_.setAutoDelete(true);
  }
}

PathwayCanvasItem *
SubnetworkItem::clone() const {
  return new SubnetworkItem(*this);
}

void
SubnetworkItem::snugUp() {
  updateExportList();
#ifdef GUI
  QFontMetrics fm(font_);
  int y_step = fm.height()+2;
  QRect bb = fm.boundingRect(label_);
  int w = bb.width(), h = y_step;

  QListIterator<SE_t> it(exported_);
  for(SE_t *p; it.current(); ++it) {
    p = it.current();
    QRect br = fm.boundingRect(p->m->label());
    if(w < br.width()) w = br.width();
    h += y_step;
  }

  // The creation of tpoints_ is a hack to ensure that exported molecules
  // will be inline with regular molecules.
  int h2 = h;
  int offset = 0;
  if((exported_.count() % 2) != 0) {
    h += y_step;
    offset = y_step/2;
  }
  //printf("%s %d %d\n", (const char*)label(), h, h2);
  
  w += 8;
  text_x_ = -w/2 + 3;  // text_y is computed in draw()
  points_.resize(4);
  points_.setPoint(0, -w/2, -h/2);
  points_.setPoint(1,  w/2, -h/2);
  points_.setPoint(2,  w/2,  h/2);
  points_.setPoint(3, -w/2,  h/2);

  tpoints_.resize(4);
  tpoints_.setPoint(0, -w/2, -h2/2-offset);
  tpoints_.setPoint(1,  w/2, -h2/2-offset);
  tpoints_.setPoint(2,  w/2,  h2/2-offset);
  tpoints_.setPoint(3, -w/2,  h2/2-offset);
#endif
}

bool
SubnetworkItem::addEdge(SubnetworkEdgeItem *e, MoleculeItem *m) {
  QListIterator<SE_t> it(exported_);
  for(SE_t *p; it.current(); ++it) {
    p = it.current();
    if(p->m == m) {
      /* If molecule already has an edge return false */
      if(p->e) {
	//puts("Molecule already has an edge");
	return false;
      } else {
	//puts("Success adding edge");
	p->e = e;
	edges_.append(e);
	return true;
      }
    }
  }
  //puts("Molecule not found");
  return false;
}

void
SubnetworkItem::removeEdge(SubnetworkEdgeItem *e) {
  MolRxnBaseItem::removeEdge(e);
  //puts("Removing edge");
  for(SE_t *p=exported_.first(); p; p=exported_.next()) {
    //printf("p:%p p->m:%p p->e:%p e:%p\n", p, p->m, p->e, e);
    if(p->e == e) {
      //puts("Found the edge");
      p->e = NULL;
      break;
    }
  }
}

MoleculeItem *
SubnetworkItem::exportedMolecule(QPoint pt) {
#ifdef GUI
  QFontMetrics fm(font_);
  int y_step = fm.height()+2;
#else
  int y_step = 16;
#endif
  pt /= sf_;
  
  QString str;

  int ind = (pt.y() - boundingRect().y()) / y_step - 1;
  //printf("%d %d %d\n", pt.y(), boundingRect().y(), ind);
  
  if(ind < 0 || ind >= (int)exported_.count()) return NULL;
  return exported_.at(ind)->m;
}

MoleculeItem *
SubnetworkItem::exportedMolecule(const QString &str) {
  QListIterator<SE_t> it(exported_);
  for(; it.current(); ++it) {
    if(it.current()->m->label() == str) {
      return it.current()->m;
    }
  }
  return NULL;
}

MoleculeItem *
SubnetworkItem::exportedMolecule(const SubnetworkEdgeItem *e) {
  QListIterator<SE_t> it(exported_);
  //puts("in sn::exportedMolecule()");
  for(; it.current(); ++it) {
    //printf("sn::exportedMolecule %p %p %p\n", e, it.current(), it.current()->e);
    if(it.current()->e == e) {
      return it.current()->m;
    }
  }
  return NULL;
}

int
SubnetworkItem::ex(const SubnetworkEdgeItem *) const {
  return x();
}

int
SubnetworkItem::ey(const SubnetworkEdgeItem *e) const {
#ifdef GUI
  QFontMetrics fm(font_);
  int y_step = fm.height()+2;;
#else
  int y_step = 16;
#endif

  QListIterator<SE_t> it(exported_);
  for(int i=1; it.current(); ++it, i++) {
    if(it.current()->e == e) {
      return boundingRect().y() + y_step * i + y_step/2;
    }
  }
  return y();
}

int
SubnetworkItem::numMolecules() const {
  int n=0;
  QListIterator<PathwayCanvasItem> it(items_);
  for(; it.current(); ++it) {
    if(isMolecule(it.current())) n++;
    else if(isSubnetwork(it.current())) n += ((SubnetworkItem*)it.current())->numMolecules();
  }
  return n;
}

int
SubnetworkItem::numBoundMolecules() const {
  int n=0;
  {
    QListIterator<SE_t> it(exported_);
    for(; it.current(); ++it) {
      if(it.current()->e) n++;
    }
  }
  {
    QListIterator<PathwayCanvasItem> it(items_);
    for(; it.current(); ++it) {
      if(isSubnetwork(it.current())) n += ((SubnetworkItem*)it.current())->numBoundMolecules();
    }
  }
  return n;
}

int
SubnetworkItem::numReactions() const {
  int n=0;
  QListIterator<PathwayCanvasItem> it(items_);
  for(; it.current(); ++it) {
    if(isReaction(it.current())) n++;
    else if(isSubnetwork(it.current())) n += ((SubnetworkItem*)it.current())->numReactions();
  }
  return n;
}

QWidget *
SubnetworkItem::propertiesDialog(QWidget *parent, const Simulation &) {
#ifdef GUI
  if(!pd_) {
    pd_ = new SubnetworkDialog(parent, *this);
    pd_->show();
    return pd_;
  } else {
    ((SubnetworkDialog*)pd_)->updateDialog();
    pd_->show();
  }
#endif
  return NULL;
}

void
SubnetworkItem::updatePropertiesFromDialog() {
#ifdef GUI
  if(pd_) {
    ((SubnetworkDialog*)pd_)->updateItem();
    user_initialized_ = true;
    snugUp();
    if(parent_) parent_->update();
  }
#endif
}

void
SubnetworkItem::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e;
  QDomText t;

  QDomElement sn = doc.createElement("Subnetwork");
  n.appendChild(sn);

  e = doc.createElement("Label");
  t = doc.createTextNode(label_);
  sn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  sn.appendChild(e); e.appendChild(t);

  e = doc.createElement("Pos");
  t = doc.createTextNode(QObject::tr("%1 %2").arg(x_).arg(y_));
  sn.appendChild(e); e.appendChild(t);

  if(!notes_.isEmpty()) {
    e = doc.createElement("Notes");
    t = doc.createTextNode(notes_);
    sn.appendChild(e); e.appendChild(t);
  }

  if(!user_initialized_) {
    e = doc.createElement("NotInitialized");
    sn.appendChild(e);
  }

  e = doc.createElement("Network");
  sn.appendChild(e);

  pathwaySaveItems(doc, e, const_cast<SubnetworkItem*>(this)->items_);

  e = doc.createElement("Edges");
  sn.appendChild(e);
  
  QListIterator<EdgeBaseItem> i(edges_);
  for(; i.current(); ++i) {
    i.current()->save(doc, e);
  }
}

QString
SubnetworkItem::tip() const {
  QString str;
  str.sprintf("%s\n"
	      "Name: %s\n"
	      "Equation: %s\n",
	      (const char*)cap_,
	      (const char*)label_,
	      (const char*)equation());

  if(!user_initialized_) str += "\nNot initialized";
  
  return str;
}

QString
SubnetworkItem::equation() const {
  return "Empty";
}
 
QList<PathwayCanvasItem>
SubnetworkItem::items() const {
  return copy(const_cast<SubnetworkItem*>(this)->items_);
}

QList<PathwayCanvasItem>
SubnetworkItem::rawItems() const {
  return const_cast<SubnetworkItem*>(this)->items_;
}

void
SubnetworkItem::setItems(QList<PathwayCanvasItem> &items) {
  items_ = copy(items);
  snugUp();
}

int
SubnetworkItem::rtti() const {
  return SubnetworkItem::RTTI;
}

void
SubnetworkItem::draw(QPainter *p) {
  p->save();
  p->scale(sf_, sf_);
#ifdef GUI
  p->setFont(font_);
#endif
  p->setBrush(bg_);
  p->translate(x_, y_);

  p->setPen(pen());
  p->drawPolygon(tpoints_);
  //p->drawPolygon(points_);

  p->setPen(Qt::black);

  QRect bb = boundingRect();

#ifdef GUI
  QFontMetrics fm(font_);
  int y_step = fm.height()+2;;
#else
  int y_step = 16;
#endif
  
  int text_y_ = -y_ + bb.y() + y_step; // remove translation to y
#ifdef WIN32
  int txo = 2;
#else
  int txo = 0;
#endif
  
  p->drawText(text_x_, text_y_-y_step+txo, bb.width()-6, y_step, QObject::AlignHCenter, label_);

  QListIterator<SE_t> it(exported_);
  for(MoleculeItem *q; it.current(); ++it) {
    q = it.current()->m;
    p->setBrush(q->color());
    p->setPen(pen());
    p->drawRect(-bb.width()/2, text_y_, bb.width(), y_step+1);
    text_y_ += y_step;
    p->setPen(Qt::black);
    p->drawText(text_x_, text_y_-y_step+txo, bb.width()-6, y_step, QObject::AlignHCenter, q->label());
  }
  
  p->restore();
}

bool
SubnetworkItem::contains(QPoint p) {
  p /= sf_;
 
#ifdef GUI 
  QFontMetrics fm(font_);
  int y_step = fm.height()+2;;
#else
  int y_step = 16;
#endif

  QRect bb = tightBoundingRect();
  if((exported_.count() % 2) != 0) {
    bb.moveBy(0, -y_step/2);
  }
  return bb.contains(p);
}

QRect
SubnetworkItem::tightBoundingRect() const {
  QRect bb = tpoints_.boundingRect();
  bb.moveCenter(QPoint(x_, y_));
  return bb;
}

void
SubnetworkItem::buildExportList(QList<SE_t> &exported) {
  exported.clear();
  
  QListIterator<PathwayCanvasItem> it(items_);
  for(PathwayCanvasItem *p; it.current(); ++it) {
    p = it.current();
    if(isMolecule(p) && ((MoleculeItem*)p)->exported()) {
      exported.append(new SE_t((MoleculeItem*)p, NULL));
    }
  }

  {
    // Sort exported molecule list by molecule label
    QListIterator<SE_t> i(exported), j(exported);
    for(SE_t *p, *q; i.current(); ++i) {
      p = i.current();
      j = i;
      
      for(++j; j.current(); ++j) {
	q = j.current();
	if(p->m && q->m) {
	  if(QString::localeAwareCompare(p->m->label(), q->m->label()) > 0) {
	    SE_t t = *p;
	    *p = *q;
	    *q = t;
	  }
	}
      }
    }
  }
}

void
SubnetworkItem::updateExportList() {
  //fprintf(stderr, "Updating export list\n");

  // Build list of exported molecules
  QList<SE_t> exported;
  buildExportList(exported);

  SE_t *i, *j;
  
  // Remove edges who's exported molecule no longer exists
  for(j=exported_.first(); j; j=exported_.next()) {
    for(i=exported.first(); i; i=exported.next()) {
      if(i->m->label() == j->m->label()) {
	break;
      }
    }
    if(!i) {
      // Molecule represented by j is not present in new export
      if(j->e) j->e->breakEdge();
    }
  }
  
  // Merge new and original list of exported molecules
  for(i=exported.first(); i; i=exported.next()) {
    for(j=exported_.first(); j; j=exported_.next()) {
      if(i->m->label() == j->m->label()) {
	i->e = j->e;
	break;
      }
    }
  }

  exported_ = exported;
  exported_.setAutoDelete(true);
}




SubnetworkEdgeItem::SubnetworkEdgeItem(QWidget *p)
  : EdgeBaseItem(p)
{}

SubnetworkEdgeItem::SubnetworkEdgeItem(const SubnetworkEdgeItem &i)
  : EdgeBaseItem(i)
{}

SubnetworkEdgeItem::~SubnetworkEdgeItem() {
  //puts("~SubnetworkEdgeItem");
  breakEdge();
}

void
SubnetworkEdgeItem::breakEdge() {
  //puts("breakEdge()");
  MoleculeItem   *mol = molecule();
  SubnetworkItem *sn  = subnetwork();
  
  if(mol) mol->removeEdge(this);
  if(sn)  sn->removeEdge(this);

  ipoints_.clear();
}

PathwayCanvasItem *
SubnetworkEdgeItem::clone() const {
  return new SubnetworkEdgeItem(*this);
}

void
SubnetworkEdgeItem::save(QDomDocument &doc, QDomElement &n) const {
  if(!subnetwork() || !molecule()) {
    pneDebug("Incomplete reaction edge ignored during save");
    return;
  }
  
  QDomElement e, e2;
  QDomText t;

  QDomElement sne = doc.createElement("SubnetworkEdge");
  n.appendChild(sne);

  e = doc.createElement("Id");
  t = doc.createTextNode(QObject::tr("%1").arg(id_));
  sne.appendChild(e); e.appendChild(t);

  {
    e = doc.createElement("SubnetworkMolecule");
    sne.appendChild(e);

    e2 = doc.createElement("Label");
    t = doc.createTextNode(subnetworkMolecule()->label());
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Id");
    t = doc.createTextNode(QObject::tr("%1").arg(subnetworkMolecule()->id()));
    e.appendChild(e2); e2.appendChild(t);
  }

  {
    e = doc.createElement("Molecule");
    sne.appendChild(e);

    e2 = doc.createElement("Label");
    t = doc.createTextNode(molecule()->label());
    e.appendChild(e2); e2.appendChild(t);

    e2 = doc.createElement("Id");
    t = doc.createTextNode(QObject::tr("%1").arg(molecule()->id()));
    e.appendChild(e2); e2.appendChild(t);
  }

  QListIterator<MolRxnBaseItem> i(ipoints_);
  if(i.count() > 2) {
    e = doc.createElement("InternalNodes");
    sne.appendChild(e);
    for(; i.current(); ++i) {
      if(isNode(i.current())) {
	i.current()->save(doc, e);
      }
    }
  }
}

void
SubnetworkEdgeItem::appendNode(PathwayCanvasItem *p) {
  if(isMolRxn(p)) {
    MolRxnBaseItem *q = (MolRxnBaseItem*)p;
    ipoints_.append(q);
    q->addEdge(this);
  } else if(isNode(p)) {
    InternalNodeItem *q = (InternalNodeItem*)p;
    ipoints_.append(q);
    q->setEdge(this);
  } else if(isSubnetwork(p)) {
    fprintf(stderr, "Use appendPoint(Subnetwork *, const QPoint &) instead for subnetworks\n");
    abort();
  } else {
    abort();
  }
}

bool
SubnetworkEdgeItem::appendNode(SubnetworkItem *p, const QPoint &pt) {
  if(ipoints_.isEmpty()) {
    MoleculeItem *m = p->exportedMolecule(pt);
    //printf("Identified molecule: %p %s\n", m, m ? (const char*)m->label() : "NULL");
    if(m && p->addEdge(this, m)) {
      ipoints_.append(p);
      return true;
    }
  }
  return false;
}

bool
SubnetworkEdgeItem::appendNode(SubnetworkItem *p, const QString &str) {
  if(ipoints_.isEmpty()) {
    MoleculeItem *m = p->exportedMolecule(str);
    //printf("Identified molecule: %p %s\n", m, m ? (const char*)m->label() : "NULL");
    if(m && p->addEdge(this, m)) {
      ipoints_.append(p);
      return true;
    }
  }
  return false;
}

MolRxnBaseItem *
SubnetworkEdgeItem::addPoint(MolRxnBaseItem *p) {
  if(isSubnetwork(p)) {
    fprintf(stderr, "Use addPoint(Subnetwork *, const QPoint &) instead for subnetworks\n");
    abort();
  }
  if(ipoints_.isEmpty()) {
    abort();
  } else {
    if(isMolecule(p)) {
      PathwayCanvasItem *p2 = ipoints_.last();
      ipoints_.removeLast();
      if(p2) delete p2;
      ipoints_.append(p);
      p->addEdge(this);
      return NULL;
    }
  }
  InternalNodeItem *in = new InternalNodeItem(parent_);
  in->setEdge(this);
  ipoints_.append(in);
  return in;
}

MolRxnBaseItem *
SubnetworkEdgeItem::addPoint(SubnetworkItem *p, const QPoint &pt) {
  InternalNodeItem *in = NULL;
  if(appendNode(p, pt)) {
    in = new InternalNodeItem(parent_);
    in->setEdge(this);
    ipoints_.append(in);
  }
  return in;
}

MolRxnBaseItem *
SubnetworkEdgeItem::addPoint(SubnetworkItem *p, const QString &str) {
  InternalNodeItem *in = NULL;
  if(appendNode(p, str)) {
    in = new InternalNodeItem(parent_);
    in->setEdge(this);
    ipoints_.append(in);
  }
  return in;
}

bool
SubnetworkEdgeItem::contains(QPoint p) {
  p /= sf_;
  
  //puts("Checking...");
  MolRxnBaseItem *i = ipoints_.first();
  MolRxnBaseItem *j = ipoints_.next();
  if(i && j) {
    QRect bb = ((SubnetworkItem*)i)->boundingRect();
    int x1 = ((SubnetworkItem*)i)->ex(this), y1 = ((SubnetworkItem*)i)->ey(this);
    int x3 = j->x(), y3 = j->y();
    int x2 = (x3<x1)?bb.x()-5:bb.x()+bb.width()+5, y2 = y1;
      
    if(distanceToLine(QPoint(x1, y1),
		      QPoint(x2, y2),
		      p) < 4.0) return true;
    if(distanceToLine(QPoint(x2, y2),
		      QPoint(x3, y3),
		      p) < 4.0) return true;
  }
  i = j;
  j = ipoints_.next();
  for(; j; i=j, j=ipoints_.next()) {
    if(distanceToLine(QPoint(i->x(), i->y()),
		      QPoint(j->x(), j->y()),
		      p) < 4.0) return true;
  }
  //puts("Failed to find a point...");
  return false;
}

void
SubnetworkEdgeItem::draw(QPainter *p) {
  p->save();
  p->scale(sf_, sf_);
#ifdef GUI
  p->setFont(font_);
#endif
  if(selected_) {
    p->setPen(QPen(Qt::red, 4));
    p->setBrush(Qt::red);
  } else {
    p->setPen(Qt::black);
    p->setBrush(Qt::black);
  }
  MolRxnBaseItem *i = ipoints_.first();
  MolRxnBaseItem *j = ipoints_.next();
  if(i && j) {
    QRect bb = ((SubnetworkItem*)i)->boundingRect();
    int x1 = ((SubnetworkItem*)i)->ex(this), y1 = ((SubnetworkItem*)i)->ey(this);
    int x3 = j->x(), y3 = j->y();
    int x2 = (x3<x1)?bb.x()-5:bb.x()+bb.width()+5, y2 = y1;
    int r = 2;
      
    p->drawLine(x1, y1, x2, y2);
    if(y2 != y3) {
      p->drawPie(x2-r, y2-r, r*2, r*2, 0, 5760);
    }
    p->drawLine(x2, y2, x3, y3);
  }
  i = j;
  j = ipoints_.next();
  for(; j; i=j, j=ipoints_.next()) {
    p->drawLine(i->x(), i->y(), j->x(), j->y());
  }

  for(i=ipoints_.first(); i; i=ipoints_.next()) {
    if(isNode(i)) {
      i->draw(p);
    }
  }

  p->restore();
}

int
SubnetworkEdgeItem::rtti() const {
  return SubnetworkEdgeItem::RTTI;
}

MoleculeItem *
SubnetworkEdgeItem::molecule() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toLast();
  if(p && isMolecule(p)) return (MoleculeItem*)p;
  return NULL;
}

SubnetworkItem *
SubnetworkEdgeItem::subnetwork() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toFirst();
  if(p && isSubnetwork(p)) return (SubnetworkItem*)p;
  return NULL;
}

MoleculeItem *
SubnetworkEdgeItem::subnetworkMolecule() const {
  QListIterator<MolRxnBaseItem> i(ipoints_);
  const PathwayCanvasItem *p = i.toFirst();
  if(p && isSubnetwork(p)) return ((SubnetworkItem*)p)->exportedMolecule(this);
  return NULL;
}
