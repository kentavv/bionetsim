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

#ifndef _PATHWAY_CANVAS_ITEMS_H_
#define _PATHWAY_CANVAS_ITEMS_H_

#include <qcolor.h>
#include <qdom.h>
#include <qlist.h>
#include <qpoint.h>
#include <qpointarray.h>

#include "matrix.h"
#include "simulation.h"

class QFont;
class QPainter;
class QWidget;
class PathwayCanvasItem;
class MoleculeItem;
class ReactionItem;
class EdgeBaseItem;
class ReactionEdgeItem;
class SubnetworkEdgeItem;
class SubnetworkItem;

typedef QList<PathwayCanvasItem> PathwayCanvasItemList;

QFont pathwayCanvasItemsFont();
void setPathwayCanvasItemsFont(QFont);

double pathwayCanvasItemsScalingFactor();
void setPathwayCanvasItemsScalingFactor(double);

void loadPathwayCanvasItemsIcons();

class PathwayCanvasItem {
public:
  PathwayCanvasItem(QWidget *);
  PathwayCanvasItem(const PathwayCanvasItem &);
  virtual ~PathwayCanvasItem();

  virtual PathwayCanvasItem *clone() const = 0;
  
  int id() const;
  void setId(int);

  virtual QString tip() const;
  
  bool selected() const;
  virtual void setSelected(bool);
  void setParent(QWidget*);

  virtual int rtti() const = 0;
  virtual int sub_rtti() const;
  virtual QWidget *propertiesDialog(QWidget *, const Simulation &) = 0;
  virtual void updatePropertiesDialog(const Simulation &);
  virtual void updatePropertiesFromDialog() = 0;
  virtual void draw(QPainter *) = 0;
  virtual bool contains(QPoint) = 0;
  virtual bool collidesWith(const PathwayCanvasItem*) = 0;
  virtual QRect boundingRect() const;

  virtual void save(QDomDocument&, QDomElement&) const = 0;

protected:
  QWidget *parent_;
  QWidget *pd_;
  QPointArray points_;
  bool selected_;
  //QFont *font_;
  int id_;
};

class MolRxnBaseItem : public PathwayCanvasItem {
public:
  MolRxnBaseItem(QWidget *);
  MolRxnBaseItem(const MolRxnBaseItem &);
  ~MolRxnBaseItem();

  void move(int dx, int dy);
  void moveTo(int x, int y);
  int x() const;
  int y() const;

  bool initialized() const;
  void setInitialized(bool);
  
  QString label() const;
  virtual void setLabel(QString);
  QString caption() const;
  virtual void snugUp();

  void addEdge(EdgeBaseItem *);
  virtual void removeEdge(EdgeBaseItem *);
  int numEdges() const;
  QList<EdgeBaseItem> edges() const;
  
  void draw(QPainter *);
  virtual QPen pen() const;
  bool contains(QPoint);
  bool collidesWith(const PathwayCanvasItem*);
  QPoint intersection(const QPoint&, const QPoint&);
  QRect boundingRect() const;

  QString notes() const;
  void setNotes(const QString&);
  
  void setColor(QColor);
  void defaultColor();
  QColor color() const;
  
  //void setFont(QFont *);
  //QFont *font() const;
  void setDrawIcon(bool);
  bool drawIcon() const;
  void setDrawLabel(bool);
  bool drawLabel() const;
  
protected:
  QColor bg_, def_bg_;
  QString label_, cap_;
  int text_x_, text_y_;
  int x_, y_;
  QList<EdgeBaseItem> edges_;
  bool draw_icon_;
  bool draw_label_;
  bool icon_avail_;
  bool user_initialized_;
  QString notes_;
};

class MoleculeItem : public MolRxnBaseItem {
public:
  MoleculeItem(QWidget *);
  MoleculeItem(const MoleculeItem &);
  ~MoleculeItem();

  //PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesFromDialog();
  
  void save(QDomDocument&, QDomElement&) const;

  void draw(QPainter *);
  QPen pen() const;

  bool exported() const;
  void setExported(bool);
  
  double initialConcentration() const;
  void setInitialConcentration(double);

  bool decays() const;
  void setDecays(bool);
  double decayRate() const;
  void setDecayRate(double);

  bool plotted() const;
  void setPlotted(bool);

  double concentration() const;
  void setConcentration(double);

  QString tip() const;
 
  int rtti() const;
  static int RTTI;

protected:
  double initial_con_;
  double decay_rate_;
  double con_;
  bool decays_, exported_, plotted_;
  QString tag_;
};

class DNAMoleculeItem : public MoleculeItem {
public:
  DNAMoleculeItem(QWidget *);
  DNAMoleculeItem(const DNAMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class mRNAMoleculeItem : public MoleculeItem {
public:
  mRNAMoleculeItem(QWidget *);
  mRNAMoleculeItem(const mRNAMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class EnzymeMoleculeItem : public MoleculeItem {
public:
  EnzymeMoleculeItem(QWidget *);
  EnzymeMoleculeItem(const EnzymeMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class MetaboliteMoleculeItem : public MoleculeItem {
public:
  MetaboliteMoleculeItem(QWidget *);
  MetaboliteMoleculeItem(const MetaboliteMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class TranscriptionFactorMoleculeItem : public MoleculeItem {
public:
  TranscriptionFactorMoleculeItem(QWidget *);
  TranscriptionFactorMoleculeItem(const TranscriptionFactorMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class ComplexMoleculeItem : public MoleculeItem {
public:
  ComplexMoleculeItem(QWidget *);
  ComplexMoleculeItem(const ComplexMoleculeItem &);

  PathwayCanvasItem *clone() const;
};

class AnnotationItem : public MolRxnBaseItem {
public:
  AnnotationItem(QWidget *);
  AnnotationItem(const AnnotationItem &);

  PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesFromDialog();
  
  void save(QDomDocument&, QDomElement&) const;

  int rtti() const;
  static int RTTI;
};

class ControlVariableItem : public MoleculeItem {
public:
  ControlVariableItem(QWidget *);
  ControlVariableItem(const ControlVariableItem &);

  virtual double concentration(double) const = 0;

  virtual void setCurEnvironment(int);
  virtual void setNumEnvironment(int);
  
  virtual void init();

  int sub_rtti() const;
  static int sub_RTTI;

protected:
  int cur_env_;
  int num_env_;
};

class SquareWaveControlVariableItem : public ControlVariableItem {
public:
  SquareWaveControlVariableItem(QWidget *);
  SquareWaveControlVariableItem(const SquareWaveControlVariableItem &);
  ~SquareWaveControlVariableItem();

  PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesDialog(const Simulation &);
  void updatePropertiesFromDialog();
  
  void save(QDomDocument&, QDomElement&) const;

  double concentration(double) const;

  void setCurEnvironment(int);
  void setNumEnvironment(int);

  void setValues(double s1, double s2);
  void values(double s[2]);

  void setDuration(double s1, double s2);
  void duration(double s[2]);

  void setTransitionTime(double s1, double s2);
  void transitionTime(double s[2]);

  void setPhaseShift(double s);
  double phaseShift();

  void setNoise(double s);
  double noise();

  void setParameters(const double *values, const double *duration,
		     const double *transition, const double *phase_shift,
		     const double *noise);
  void parameters(double *values, double *duration,
		  double *transition, double *phase_shift, double *noise) const;

  void init();

  QString tip() const;

protected:
  double *values_;
  double *c_values_;
  double *duration_;
  double *transition_;
  double *phase_shift_;
  double *noise_;
};

class InterpolatedControlVariableItem : public ControlVariableItem {
public:
  InterpolatedControlVariableItem(QWidget *);
  InterpolatedControlVariableItem(const InterpolatedControlVariableItem &);
  ~InterpolatedControlVariableItem();

  PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesDialog(const Simulation &);
  void updatePropertiesFromDialog();
  
  void save(QDomDocument&, QDomElement&) const;

  double concentration(double) const;

  void setCurEnvironment(int);
  void setNumEnvironment(int);

  void setValues(const Matrix &);
  void values(Matrix &);

  QString tip() const;

protected:
  Matrix values_;
};

class ReactionItem : public MolRxnBaseItem {
public:
  ReactionItem(QWidget *);
  ReactionItem(const ReactionItem &);
  ~ReactionItem();

  virtual void equation(QString &lhs, QString &rhs, bool include_k=true) const;
  virtual QString equation() const;

  bool reversible() const;
  void setReversible(bool);

  int rtti() const;
  static int RTTI;

protected:
  bool reversible_;
};

class MassActionReactionItem : public ReactionItem {
public:
  MassActionReactionItem(QWidget *);
  MassActionReactionItem(const MassActionReactionItem &);
  ~MassActionReactionItem();

  void equation(QString &lhs, QString &rhs, bool include_k=true) const;
  QString equation() const;

  PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesDialog(const Simulation &);
  void updatePropertiesFromDialog();
  
  void save(QDomDocument&, QDomElement&) const;

  void setKf(double);
  double Kf() const;

  void setKr(double);
  double Kr() const;

  QString tip() const;

protected:
  double kf_, kr_;
};

class InternalNodeItem : public MolRxnBaseItem {
public:
  InternalNodeItem(QWidget *);
  InternalNodeItem(const InternalNodeItem &);
  ~InternalNodeItem();

  PathwayCanvasItem *clone() const;

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesFromDialog();

  void save(QDomDocument&, QDomElement&) const;

  const EdgeBaseItem *edge() const;
  void setEdge(const EdgeBaseItem *);
  
  void draw(QPainter *);

  int rtti() const;
  static int RTTI;

protected:
  int r_;
  const EdgeBaseItem *edge_;
};

class EdgeBaseItem : public PathwayCanvasItem {
public:
  EdgeBaseItem(QWidget *);
  EdgeBaseItem(const EdgeBaseItem &);
  ~EdgeBaseItem();

  void setSelected(bool);

  void move(MolRxnBaseItem *);

  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesFromDialog();
  
  bool contains(QPoint);
  bool collidesWith(const PathwayCanvasItem*);

  QList<MolRxnBaseItem> nodes() const;
  
  virtual void appendNode(PathwayCanvasItem *) = 0;
  virtual MolRxnBaseItem *addPoint(MolRxnBaseItem *) = 0;
  MolRxnBaseItem *insertPoint(InternalNodeItem *);
  void removeNode(MolRxnBaseItem *);
  
  QRect boundingRect() const;

protected:
  QList<MolRxnBaseItem> ipoints_;

  static double distanceToLine(const QPoint &, const QPoint &, const QPoint &);
};

class ReactionEdgeItem : public EdgeBaseItem {
public:
  ReactionEdgeItem(QWidget *);
  ReactionEdgeItem(const ReactionEdgeItem &);
  ~ReactionEdgeItem();

  PathwayCanvasItem *clone() const;

  void save(QDomDocument&, QDomElement&) const;

  void draw(QPainter *);

  MoleculeItem *molecule() const;
  ReactionItem *reaction() const;
  bool moleculeIsReactant() const;
  bool moleculeIsProduct() const;
  int coefficient() const;
  void setCoefficient(int);
  
  void appendNode(PathwayCanvasItem *);
  MolRxnBaseItem *addPoint(MolRxnBaseItem *);
  
  int rtti() const;
  static int RTTI;

protected:
  int coefficient_;

  void drawArrowHead(QPainter *, MolRxnBaseItem *, MolRxnBaseItem *);
};

class SE_t {
public:
  SE_t();
  SE_t(MoleculeItem *, SubnetworkEdgeItem *);
  
protected:
  MoleculeItem *m;
  SubnetworkEdgeItem *e;

  friend class SubnetworkItem;
};

class SubnetworkItem : public MolRxnBaseItem {
public:
  SubnetworkItem(QWidget *);
  SubnetworkItem(const SubnetworkItem &);
  ~SubnetworkItem();

  PathwayCanvasItem *clone() const;

  void snugUp();

  bool addEdge(SubnetworkEdgeItem *, MoleculeItem *);
  void removeEdge(SubnetworkEdgeItem *);
  MoleculeItem *exportedMolecule(QPoint);
  MoleculeItem *exportedMolecule(const QString &);
  MoleculeItem *exportedMolecule(const SubnetworkEdgeItem *);
  int ex(const SubnetworkEdgeItem *) const;
  int ey(const SubnetworkEdgeItem *) const;

  int numMolecules() const;
  int numBoundMolecules() const;
  int numReactions() const;
  
  QWidget *propertiesDialog(QWidget *, const Simulation &);
  void updatePropertiesFromDialog();

  void draw(QPainter *);
  bool contains(QPoint);
  QRect tightBoundingRect() const;

  void save(QDomDocument&, QDomElement&) const;

  QString tip() const;
  QString equation() const;

  QList<PathwayCanvasItem> items() const;    // Returns a copy
  QList<PathwayCanvasItem> rawItems() const; // Returns the underlying items
  void setItems(QList<PathwayCanvasItem> &);

  int rtti() const;
  static int RTTI;

protected:
  QList<PathwayCanvasItem> items_;
  QList<SE_t> exported_;
  QPointArray tpoints_;

  void buildExportList(QList<SE_t> &exported);
  void updateExportList();
};

class SubnetworkEdgeItem : public EdgeBaseItem {
public:
  SubnetworkEdgeItem(QWidget *);
  SubnetworkEdgeItem(const SubnetworkEdgeItem &);
  ~SubnetworkEdgeItem();

  void breakEdge();

  PathwayCanvasItem *clone() const;

  void move(MolRxnBaseItem *);
  
  void save(QDomDocument&, QDomElement&) const;

  bool contains(QPoint);

  void draw(QPainter *);

  MoleculeItem *molecule() const;
  SubnetworkItem *subnetwork() const;
  MoleculeItem *subnetworkMolecule() const;
  
  void appendNode(PathwayCanvasItem *);
  bool appendNode(SubnetworkItem *, const QPoint &);
  bool appendNode(SubnetworkItem *, const QString &);
  MolRxnBaseItem *addPoint(MolRxnBaseItem *);
  MolRxnBaseItem *addPoint(SubnetworkItem *, const QPoint &);
  MolRxnBaseItem *addPoint(SubnetworkItem *, const QString &);
  
  int rtti() const;
  static int RTTI;

protected:
  void drawArrowHead(QPainter *, MolRxnBaseItem *, MolRxnBaseItem *);
};

#endif
