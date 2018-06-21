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

#ifndef _PATHWAY_EDITOR_H_
#define _PATHWAY_EDITOR_H_

#include <qscrollview.h>
#include <qstring.h>
#include <qlist.h>
#include <qtooltip.h>

#include "editor.h"
#include "simulation.h"
#include "ode_gen.h"

class QPopupMenu;
class QTimer;
class MoleculeItem;
class ReactionItem;
class ReactionEdgeItem;
class PathwayCanvasItem;
class MolRxnBaseItem;
class ConcentrationDialog;
class DynamicDialog;
class EvolveThread;

class PETip : public QToolTip {
public:
  PETip(QWidget *parent);
  
protected:
  void maybeTip(const QPoint &);
};

class PathwayEditor : public QScrollView {
  Q_OBJECT

public:
  PathwayEditor(QWidget *, const char *, WFlags);
  ~PathwayEditor();

  void customEvent(QCustomEvent *);

  bool open(const QString &);
  bool insertSubNetwork(const QString &);
  bool save();
  bool saveAs(const QString &);
  bool saveSnapshot(const QString &);
  bool exportAs(const QString &, ode_format_t);
  bool exportTimeSeries(const QString &) const;
  QString filename() const;
  bool hasChanged() const;

  void draw(QPainter*);

  void editEditorParams();
  void editSimulationParams();
  void runSimulation();
  void viewReport();
  
  void tip(QPoint p, QRect &r, QString &str) const;
  
signals:
  void statusMessage(const QString &, int);
  void changed();
  void topologyChanged();
    
public slots:
  bool fileInsertSubNetwork();
  bool fileSave();
  bool fileSaveAs();
  void editUndo();
  void editRedo();
  void editSelectAll();
  void editLayoutGo(bool);
  void editLayoutRandom();  
  void optimizationEvolveGo(bool);  
  void displayConcentrationDialog();
  void displayEvolveTracesDialog();
  void displayDynamicDialog();
  void displayParameterScanDialog();
  void setChanged();

protected slots:
  void editCut();
  void editCopy();
  void editPaste();
  void insertMenu(int);

  void layoutStep();
  void evolveGetBest();

protected:
  Editor editor_;
  Simulation simulation_;
  QList<PathwayCanvasItem> items_;
  
  QString filename_;
  int version_;
  bool changed_;

  bool single_selection_;
  bool window_selection_;
  bool dragging_;
  QPoint m_start_, m_cur_, m_prev_;
  
  int cv_count_, mol_count_, rxn_count_;
  int lb_com_;

  Matrix results_;
  MolRxnBaseItem *moving_;
  ReactionEdgeItem *edge_;

  ConcentrationDialog *cd_;
  DynamicDialog *dd_;

  QPopupMenu *insert_menu_;

  PETip *pet_;
  
  void viewportPaintEvent(QPaintEvent*);
  void drawContents(QPainter*, int, int, int, int);
  void keyPressEvent(QKeyEvent*);
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);
  void contentsMouseReleaseEvent(QMouseEvent*);
  void contentsMouseDoubleClickEvent(QMouseEvent*);
  void closeEvent(QCloseEvent*);
  
  void insertItem(const QPoint&);
  void deleteEdge(ReactionEdgeItem*);
  bool collision(MolRxnBaseItem*);
  void unselectAll();
  void selectAll();
  void moveSelected(int, bool);
  void moveSelected(int, int);

  void pushUndo();
  void popUndo();
  QList<Editor> undo_editor_;
  QList<Simulation> undo_simulation_;
  QList<QList<PathwayCanvasItem> > undo_items_;

  QString subNetworkName(const PathwayCanvasItem*) const;
  bool isItemOnLayer(const PathwayCanvasItem*) const;

  QString nextControlVariableName();
  QString nextMoleculeName();
  QString nextReactionName();

  QTimer *layout_timer_;
  bool layout_was_active_;
  QArray<double> x_, y_, dx_, dy_, ddx_, ddy_;
  QArray<int> em_;
  void layoutGo(bool);
  void layoutRandom();
  void layoutInit();
  
  EvolveThread *evolve_thread_;
  Matrix evolve_stats_;
  ConcentrationDialog *evolve_traces_;
  void evolveGo(bool);
}; 

#endif
