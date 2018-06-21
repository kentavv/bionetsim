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

#ifndef _NETWORK_EDITOR_H_
#define _NETWORK_EDITOR_H_

#include <qdatetime.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qtooltip.h>

#include "editor.h"
#include "fitness.h"
#include "simulation.h"
#include "ode_gen.h"
#include "pathway_matrix.h"
#include "result.h"

class QPopupMenu;
class QProgressBar;
class QSocketDevice;
class QSocketNotifier;
class QTimer;
class ConcentrationDialog;
class DynamicDialog;
class EdgeBaseItem;
class EditorParametersDialog;
class EnvironmentsDialog;
class FitnessParametersDialog;
class HistogramDialog;
class MoleculeItem;
class MolRxnBaseItem;
class PathwayCanvasItem;
class ReactionEdgeItem;
class SubnetworkEdgeItem;
class ReactionItem;
class SimulationParametersDialog;
class SliderDialog;
class ReactionEntryDialog;

class PETip : public QToolTip {
public:
  PETip(QWidget *parent);
  
protected:
  void maybeTip(const QPoint &);
};

class PEState {
public:
  PEState();
  PEState(PEState &);
  ~PEState();
  PEState &operator = (PEState &pe);

  Editor editor_;
  Fitness fitness_;
  Simulation simulation_;
  QList<PathwayCanvasItem> items_;

  int lb_com_;
};
  
class NetworkEditor : public QScrollView {
  Q_OBJECT

public:
  NetworkEditor(QWidget *, const char *, WFlags);
  ~NetworkEditor();

  bool open(const QString &);
  bool save();
  bool saveAs(const QString &);
  bool saveSnapshot(const QString &);
  bool saveSVG(const QString &);
  bool exportAs(const QString &, ode_format_t);
  bool exportTimeSeries(const QString &) const;
  QString filename() const;
  bool hasChanged() const;

  void draw(QPainter*);

  void editEditorParams();
  void editEnvironments();
  void editSimulationParams();
  void runSimulation();
  void viewReport();
  void sliders();
	void reactionEntry();
  
  void tip(QPoint p, QRect &r, QString &str) const;

  void setProgressBar(QProgressBar *);

  Simulation simulation() const;
  
signals:
  void statusMessage(const QString &, int);
  void changed();
  void parametersChanged();
  void topologyChanged();
    
public slots:
  bool fileSave();
  bool fileSaveAs();
  void editUndo();
  void editCut(QList<PathwayCanvasItem>&);
  void editCopy(QList<PathwayCanvasItem>&);
  void copyToClipboard();
  void editPaste(QList<PathwayCanvasItem>&);
  void editClear();
  void editSelectAll();
  void editSelectNone();
  void editLayoutGo(bool);
  void editLayoutRandom();  
  void editSnapToGrid();
  void editTrimBorder();
  void displayConcentrationDialog();
  void displayDynamicDialog();
  void displayParameterScanDialog();
  void displayParameterScan2DDialog();
  void setChanged();
  void setParametersChanged();
  void setTopologyChanged();
  void editorParametersChanged();
  void environmentsChanged();
  void simulationParametersChanged();
  void fitnessCalculate();
  void fitnessParams();
  void fitnessParametersChanged();
  
  void copyCanvas(QList<PathwayCanvasItem>&);
  void replaceCanvas(QList<PathwayCanvasItem>&);

  void setCurEnvironment(int);
  
protected slots:
  void insertMenu(int);
  void libraryMenu(int); 
  void libraryMenuAboutToShow();

  void layoutStep();

  void remoteCPUShutdown();
  void remoteCPUReadyRead();

	void getItemsFromReactionEntryDialog();

protected:
  PEState state_;
  PathwayMatrix pem_;
  QValueList<Result> results_;
  
  QString filename_;
  int version_;
  bool changed_;
  bool is_running_;

  bool single_selection_;
  bool window_selection_;
  bool dragging_;
  QPoint m_start_, m_cur_, m_prev_;
  
  MolRxnBaseItem *moving_;
  EdgeBaseItem *edge_;

  EditorParametersDialog *epd_;
  EnvironmentsDialog *ed_;
  FitnessParametersDialog *fpd_;
  SimulationParametersDialog *spd_;
  ConcentrationDialog *cd_;
  DynamicDialog *dd_;
  HistogramDialog *hd_;
  SliderDialog *sl_;
  ReactionEntryDialog *rxned_;

  SubnetworkItem *sn_;
  
  QPopupMenu *insert_menu_;
  QPopupMenu *library_menu_;

  PETip *pet_;

  QProgressBar *progress_;
  
  void viewportPaintEvent(QPaintEvent*);
  void drawContents(QPainter*, int, int, int, int);
  void keyPressEvent(QKeyEvent*);
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);
  void contentsMouseReleaseEvent(QMouseEvent*);
  void contentsMouseDoubleClickEvent(QMouseEvent*);
  void closeEvent(QCloseEvent*);
  
  void insertItem(const QPoint&);
  void deleteEdge(EdgeBaseItem*);
  void deleteSelected();
  void unselectAll();
  void selectAll();
  void moveSelected(int, bool);
  void moveSelected(int, int);

  void pushUndo();
  void popUndo();
  QList<PEState> undo_;

  bool uniqueLabel(const QString&) const;
  QString randomLabel() const;
  QString nextItemName(const QString&) const;
  QString nextControlVariableName() const;
  QString nextMoleculeName() const;
  QString nextReactionName() const;
  QString nextSubnetworkName() const;

  void enforceBorder(bool trim_top_left=false);
  void viewportResizeEvent(QResizeEvent*);
  void snapToNearestGridPoint(int&, int&);

  QTimer *layout_timer_;
  bool layout_was_active_;
  QArray<double> x_, y_, dx_, dy_, ddx_, ddy_;
  QArray<int> em_;
  void layoutGo(bool);
  void layoutRandom();
  void layoutInit();  

  QSocketDevice *remote_cpu_socket_;
  QSocketNotifier *remote_cpu_socket_notifier_;
  QTime remote_cpu_qt_;
  int remote_cpu_rep_;
  int remote_cpu_net_id_;

	void clearResults();
};

#endif
