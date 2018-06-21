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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwidget.h>

#include "pathway_editor.h"
#include "pathway_canvas_items.h"
#include "pathway_load.h"
#include "editor.h"
#include "simulation.h"
#include "report_dialog.h"
#include "matrix.h"
#include "calculate.h"
#include "concentration_dialog.h"
#include "editor_params_dialog.h"
#include "simulation_params_dialog.h"
#include "dynamic_dialog.h"
#include "evolve_thread.h"
#include "version.h"

#ifdef WIN32
#define random rand
#endif

static const int n_menu_names = 10;
static const char *menu_names[n_menu_names] = {
  "Mass-Action Reaction",
  "---",
  "DNA Molecule",
  "Enzyme Molecule",
  "Metabolite Molecule",
  "Environment Molecule",
  "Transcription Fractor Molecule",
  "Complex Molecule",
  "---",
  "Square Wave Control Variable"
};

PETip::PETip(QWidget *parent)
  : QToolTip(parent)
{}

void
PETip::maybeTip(const QPoint &p) {
  if(!parentWidget()->inherits("PathwayEditor"))
    return;

  QRect r;
  QString str;
  
  ((PathwayEditor*)parentWidget())->tip(p, r, str);
  if(!r.isValid()) return;
  
  tip(r, str);
}

PathwayEditor::PathwayEditor(QWidget *p, const char *name, WFlags f)
  : QScrollView(p, name, f),
    filename_(""),
    version_(0),
    changed_(false),
    single_selection_(false),
    window_selection_(false),
    dragging_(false),
    cv_count_(0),
    mol_count_(0),
    rxn_count_(0),
    lb_com_(-1),
    moving_(NULL),
    edge_(NULL),
    cd_(NULL),
    dd_(NULL),
    layout_was_active_(false),
    evolve_traces_(NULL)
{
  resizeContents(500, 500);

  viewport()->setBackgroundMode(NoBackground);
  //viewport()->setBackgroundMode(FixedColor);
  //viewport()->setBackgroundColor(gray);
  setFocusPolicy(QWidget::StrongFocus);
  setFocus();
  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  viewport()->setMouseTracking(true);

  insert_menu_ = new QPopupMenu(this);
  for(int i=0; i<n_menu_names; i++) {
    if(strcmp(menu_names[i], "---") == 0) {
      insert_menu_->insertSeparator();
    } else {
      insert_menu_->insertItem(menu_names[i], i);
    }
  }

  connect(insert_menu_, SIGNAL(activated(int)), SLOT(insertMenu(int)));
  connect(this, SIGNAL(changed()), SLOT(setChanged()));
  connect(this, SIGNAL(topologyChanged()), SLOT(setChanged()));
  
  setPathwayCanvasItemsFont(font());

  items_.setAutoDelete(true);

  layout_timer_ = new QTimer(this);
  connect(layout_timer_, SIGNAL(timeout()), SLOT(layoutStep()));

  evolve_thread_ = new EvolveThread(this);
  
  pet_ = new PETip(this);
}

PathwayEditor::~PathwayEditor() {
  delete pet_;
  if(evolve_thread_) delete evolve_thread_;
}

void
PathwayEditor::customEvent(QCustomEvent *e) {
  if(e->type() == QEvent::User + 100) {
    //EvolveEvent *ee = (EvolveEvent*)e;
    evolveGetBest();
  }
}

void
PathwayEditor::editUndo() {
  puts("Undo");
#if 0
  popUndo();
#endif
}

void
PathwayEditor::editRedo() {
  puts("Redo");

#if 0
  if(undo_editor_.current() != undo_editor_.last() && undo_editor_.count() > 0) {
    editor_ = *undo_editor_.next();
    simulation_ = *undo_simulation_.next();
    items_ = *undo_items_.next();
    puts("Redo successful");
  }
#endif
}

void
PathwayEditor::editCut() {
}

void
PathwayEditor::editCopy() {
}

void
PathwayEditor::editPaste() {
}

void
PathwayEditor::editSelectAll() {
  selectAll();
  viewport()->update();
}

void
PathwayEditor::editLayoutGo(bool s) {
  layoutGo(!layout_timer_->isActive());
  //layoutGo(s);
}

void
PathwayEditor::editLayoutRandom() {
  layoutRandom();
}

void
PathwayEditor::optimizationEvolveGo(bool s) {
  evolveGo(s);
}

bool
PathwayEditor::open(const QString &filename) {
  if(!pathwayLoad((const char *)filename, editor_,
		  simulation_, items_, version_, viewport())) {
    return false;
  }

  changed_ = false;
  filename_ = filename;

  setCaption(filename);
  resizeContents(500, 500);
  
  viewport()->update();
  
  return true;
}

bool
PathwayEditor::insertSubNetwork(const QString &filename) {
  if(filename.isEmpty()) return true;

  Editor sub_editor;
  Simulation sub_simulation;
  QList<PathwayCanvasItem> sub_items;
  if(!pathwayLoad((const char *)filename, sub_editor,
		  sub_simulation, sub_items, viewport())) {
    return false;
  }

  QFileInfo fi(filename);
  QString basename = fi.baseName();
  bool ok;
  QString text = QInputDialog::getText(appname, "Enter basename for sub-network:", QLineEdit::Normal, basename, &ok, this);
  if(!ok || text.isEmpty()) {
    return false;
  } else {
    basename = text;
  }

  unselectAll();
  
  QListIterator<PathwayCanvasItem> i(sub_items);
  for(; i.current(); ++i) {
    if(i.current()->rtti() == MoleculeItem::RTTI ||
       i.current()->rtti() == ReactionItem::RTTI) {
      ((MolRxnBaseItem*)i.current())->setLabel(basename + "::" + ((MolRxnBaseItem*)i.current())->label());
    }
    if(i.current()->rtti() == MoleculeItem::RTTI &&
       ((MoleculeItem*)i.current())->exported()) {
      i.current()->setSelected(true);
    }
    if(i.current()->rtti() == MoleculeItem::RTTI &&
       i.current()->sub_rtti() == ControlVariableItem::sub_RTTI) {
      ((ControlVariableItem*)i.current())->setNumEnvironment(simulation_.numEnvironment());
      ((ControlVariableItem*)i.current())->setCurEnvironment(simulation_.curEnvironment());
    }
    items_.append(*i);
  }

  emit topologyChanged();

  viewport()->update();

  return true;
}

bool
PathwayEditor::fileInsertSubNetwork() {
  QString fn = QFileDialog::getOpenFileName(QString::null, "Network Files (*.net);;All Files (*)", this);
  if(fn.isEmpty()) return false;

  if(!insertSubNetwork(fn)) {
    QMessageBox::critical(NULL, appname, QString("Unable to import ") + fn);
    return false;
  }

  return true;
}

bool
PathwayEditor::fileSave() {
  if(filename_.isEmpty()) {
    return fileSaveAs();
  } else {
    if(!save()) {
      QMessageBox::critical(NULL, appname, QString("Unable to save ") + filename_);
      return false;
    }
  }
  return true;
}

bool
PathwayEditor::fileSaveAs() {
  QString fn = QFileDialog::getSaveFileName(QString::null, "Network Files (*.net);;All Files (*)", this);
  if(fn.isEmpty()) return false;

  if(!saveAs(fn)) {
    QMessageBox::critical(NULL, appname, QString("Unable to save as ") + fn);
    return false;
  }
  return true;
}

bool
PathwayEditor::save() {
  return saveAs(filename_);
}

bool
PathwayEditor::saveAs(const QString &filename) {
  version_++;
  
  if(!pathwaySave((const char *)filename, appname, appversion, editor_,
		  simulation_, items_, version_)) {
    return false;
  }
  
  changed_ = false;
  filename_ = filename;

  setCaption(filename);

  return true;
}

bool
PathwayEditor::saveSnapshot(const QString &filename) {
  QPixmap pix(contentsWidth(), contentsHeight());
  QPainter p(&pix);
  draw(&p);
  p.end();
  QImage img = pix.convertToImage();
  return img.save(filename, "PNG", 0);
}

bool
PathwayEditor::exportAs(const QString &filename, ode_format_t format) {
  PathwayMatrix pem;
  pem.setup(editor_, simulation_, items_);
  return gen_ode((const char*)filename, pem, format);
}

bool
PathwayEditor::exportTimeSeries(const QString &filename) const {
  FILE *fp = fopen((const char*)filename, "w");
  if(!fp) return false;

  fprintf(fp, "Time");
  QListIterator<PathwayCanvasItem> i(items_);
  for(; i.current(); ++i) {
    if(i.current()->rtti() == MoleculeItem::RTTI) {
      fprintf(fp, "\t%s", (const char*)((MoleculeItem*)(i.current()))->label());
    }
  }
  fputc('\n', fp);

  results_.print(fp);
  
  fclose(fp);

  return true;
}

QString
PathwayEditor::filename() const {
  return filename_;
}

bool
PathwayEditor::hasChanged() const {
  return changed_;
}

void
PathwayEditor::setChanged() {
  changed_ = true;
}

void
PathwayEditor::editEditorParams() {
  QStringList subnetworks;
  subnetworks.append("toplevel");
  QListIterator<PathwayCanvasItem> i(items_);
  for(; i.current(); ++i) {
    if(i.current()->rtti() == MoleculeItem::RTTI ||
       i.current()->rtti() == ReactionItem::RTTI) {
      QString label = ((MolRxnBaseItem*)i.current())->label();
      int pos = label.findRev("::", -2);
      if(pos >= 0) {
	label.truncate(pos);
	if(!subnetworks.contains(label)) {
	  subnetworks.append(label);
	}
      }
    }
  }
  if(!subnetworks.contains(editor_.cur_subnetwork_)) {
    subnetworks.append(editor_.cur_subnetwork_);
  }
  
  EditorParametersDialog d(NULL, editor_, subnetworks);
  if(d.exec() == EditorParametersDialog::Accepted) {
    //pushUndo();

    d.getValues(editor_);
    resizeContents(500, 500);
    unselectAll();
    viewport()->update();
  }
}

void
PathwayEditor::editSimulationParams() {
  SimulationParametersDialog d(NULL, simulation_);
  if(d.exec() == SimulationParametersDialog::Accepted) {
    //pushUndo();

    d.getValues(simulation_);

    PathwayCanvasItem *item;
    for(item=items_.first(); item; item=items_.next()) {
      if(item->rtti() == MoleculeItem::RTTI && item->sub_rtti() == ControlVariableItem::sub_RTTI) {
	((ControlVariableItem*)item)->setNumEnvironment(simulation_.numEnvironment());
	((ControlVariableItem*)item)->setCurEnvironment(simulation_.curEnvironment());
      }
    }

    viewport()->update();
  }
}

void
PathwayEditor::displayConcentrationDialog() {
  QStringList labels;
  PathwayCanvasItem *item;
  for(item=items_.first(); item; item=items_.next()) {
    if(item->rtti() == MoleculeItem::RTTI && item->sub_rtti() == ControlVariableItem::sub_RTTI) {
      labels.append(((MolRxnBaseItem*)item)->label());
    }
  }
  for(item=items_.first(); item; item=items_.next()) {
    if(item->rtti() == MoleculeItem::RTTI && item->sub_rtti() != ControlVariableItem::sub_RTTI) {
      labels.append(((MolRxnBaseItem*)item)->label());
    }
  }

  if(!cd_) {
    cd_ = new ConcentrationDialog(this);
    cd_->resize(450, 400);
    connect(this, SIGNAL(topologyChanged()), cd_, SLOT(hide()));
  }
  cd_->setData(results_, labels);
  cd_->show(); 
}

void
PathwayEditor::displayEvolveTracesDialog() {
  QStringList labels;
  labels.append("Best");
  labels.append("Average");
  labels.append("Worst");
  int nc = evolve_stats_.ncolumns();
  for(int i=4; i<nc; i++) {
    QString str = tr("E%1").arg(i-3);
    labels.append(str);
  }
  if(!evolve_traces_) {
    evolve_traces_ = new ConcentrationDialog(this);
    evolve_traces_->resize(450, 400);
    connect(this, SIGNAL(topologyChanged()), evolve_traces_, SLOT(hide()));
  }
  evolve_traces_->setData(evolve_stats_, labels);
  evolve_traces_->show(); 
}

void
PathwayEditor::displayDynamicDialog() {
  QStringList labels;
  PathwayCanvasItem *item;
  for(item=items_.first(); item; item=items_.next()) {
    if(item->rtti() == MoleculeItem::RTTI) {
      labels.append(((MolRxnBaseItem*)item)->label());
    }
  }

  if(!dd_) {
    dd_ = new DynamicDialog(this);
    connect(this, SIGNAL(topologyChanged()), dd_, SLOT(hide()));
  }
  dd_->setData(results_, labels, &items_, viewport());
  dd_->show();  
}

//#include "scaled_matrix_view.h"

void
PathwayEditor::displayParameterScanDialog() {
#if 0
  PathwayMatrix pem;
  pem.setup(editor_, simulation_, items_);
  //parameterScanDialog ps(pem);
  //ps.exec();

  {
    int p1 = 1;
    int p2 = 2;

    double min = 1.0;
    double max = 100.0;
    int step = 10;
    int i, j;

    QTime qt1;
    qt1.start();
    int n = 0;
    double min_fit = DBL_MAX, avg_fit = 0.0, max_fit = -DBL_MAX;

    Matrix m(step+1, step+1);
    
    for(i=0; i<=step; i++) {
      double y = i/(double)step * (max - min) + min;
      pem.kc(p1, 0) = y;
      for(j=0; j<=step; j++) {
	double x = j/(double)step * (max - min) + min;
	pem.kc(p2, 0) = x;
	double fitness = -1.0;
	Matrix results;
	if(run_simulation(pem, results)) {
	  fitness = calculate_fitness(pem, results);
	}
	printf("%f %f %f\n", y, x, fitness);
	m(i, j) = fitness;
	
	if(fitness < min_fit) min_fit = fitness;
	if(fitness > max_fit) max_fit = fitness;
	avg_fit += fitness;
	n++;
      }
    }

    avg_fit /= n;
    
    double t1 = qt1.elapsed() / 1000.0;
    
    QString str;
    str.sprintf("Parameter scan complete - "
		"Fitness: Min:%f Avg:%f Max:%f  "
		"Time Requied: Total:%f Avg:%f (%d calculations)",
		min_fit, avg_fit, max_fit, t1, t1/n, n);
    
    emit statusMessage(str, 30000);

    ScaledMatrixView *mv = new ScaledMatrixView(this);
    mv->setMatrix(m);
    mv->setScale(10);
    mv->resize(200, 200);
    mv->show();
  }
#endif
}

void
PathwayEditor::runSimulation() {
  double fitness = 1.0;
  PathwayMatrix pem;
  QTime qt1, qt2;
  double t1, t2;
  
  qt1.start();
  pem.setup(editor_, simulation_, items_);
  t1 = qt1.elapsed() / 1000.0;
  
  //pem.print();

  qt2.start();
  if(run_simulation(pem, results_)) {
    fitness = calculate_fitness(pem, results_);
  }
  t2 = qt2.elapsed() / 1000.0;
  
  QString str, str2;
  str2.sprintf("Setup:%0.3fs Sim:%0.3fs", t1, t2);

  str.sprintf("Simulation complete - Fitness: %f", fitness);
  str += "  Time Required: " + str2;

  emit statusMessage(str, 30000);

  if(cd_ && cd_->isVisible()) displayConcentrationDialog();
  if(dd_ && dd_->isVisible()) displayDynamicDialog();
}

void
PathwayEditor::viewReport() {
  ReportDialog rd(NULL, NULL, true);
  rd.setup(simulation_, items_);  
  rd.resize(400, 400);
  rd.exec();
}

QString
PathwayEditor::nextControlVariableName() {
  cv_count_++;
  QString str;
  if(editor_.cur_subnetwork_ == "toplevel") {
    str = QString("CV %1").arg(cv_count_);
  } else {
    str = QString("%1::CV %2").arg(editor_.cur_subnetwork_).arg(cv_count_);
  }
  return str;
}

QString
PathwayEditor::nextMoleculeName() {
  mol_count_++;
  QString str;
  if(editor_.cur_subnetwork_ == "toplevel") {
    str = QString("Mol %1").arg(mol_count_);
  } else {
    str = QString("%1::Mol %2").arg(editor_.cur_subnetwork_).arg(mol_count_);
  }
  return str;
}

QString
PathwayEditor::nextReactionName() {
  rxn_count_++;
  QString str;
  if(editor_.cur_subnetwork_ == "toplevel") {
    str = QString("Rxn %1").arg(rxn_count_);
  } else {
    str = QString("%1::Rxn %2").arg(editor_.cur_subnetwork_).arg(rxn_count_);
  }
  return str;
}

void
PathwayEditor::viewportPaintEvent(QPaintEvent *pe) {
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

bool
PathwayEditor::isItemOnLayer(const PathwayCanvasItem *i) const {
  return (subNetworkName(i) == editor_.cur_subnetwork_ ||
	  (i->rtti() == MoleculeItem::RTTI && ((MoleculeItem*)i)->exported()));
}

QString
PathwayEditor::subNetworkName(const PathwayCanvasItem *i) const {
  QString label;
  
  if(i->rtti() == ReactionEdgeItem::RTTI) {
    if(!((ReactionEdgeItem*)i)->molecule() ||
       !((ReactionEdgeItem*)i)->reaction() ||
       subNetworkName(((ReactionEdgeItem*)i)->molecule()) ==
       editor_.cur_subnetwork_ ||
       subNetworkName(((ReactionEdgeItem*)i)->reaction()) ==
       editor_.cur_subnetwork_) {
      return editor_.cur_subnetwork_;
    } else {
      label = ((ReactionEdgeItem*)i)->molecule()->label();
    }
  } else if(i->rtti() == InternalNodeItem::RTTI) {
    if(!((InternalNodeItem*)i)->edge() ||
       !((InternalNodeItem*)i)->edge()->molecule() ||
       !((InternalNodeItem*)i)->edge()->reaction() ||
       subNetworkName(((InternalNodeItem*)i)->edge()->molecule()) ==
       editor_.cur_subnetwork_ ||
       subNetworkName(((InternalNodeItem*)i)->edge()->reaction()) ==
       editor_.cur_subnetwork_) {
      return editor_.cur_subnetwork_;
    } else {
      label = ((InternalNodeItem*)i)->edge()->molecule()->label();
    }
  } else if(i->rtti() == ReactionItem::RTTI) {
    label = ((ReactionItem*)i)->label();
  } else if(i->rtti() == MoleculeItem::RTTI) {
    label = ((MoleculeItem*)i)->label();
  }

  int pos = label.findRev("::", -2);
  if(pos >= 0) {
    label.truncate(pos);
  } else {
    label = "toplevel";
  }
  
  return label;
}

void
PathwayEditor::draw(QPainter *p) {
  drawContents(p, 0, 0, contentsWidth(), contentsHeight());
}

void
PathwayEditor::drawContents(QPainter *p, int cx, int cy, int cw, int ch) {
  p->fillRect(cx, cy, cw, ch, white);
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if((i->rtti() == ReactionEdgeItem::RTTI ||
	i->rtti() == InternalNodeItem::RTTI) &&
       isItemOnLayer(i)) {
      //i->save(stdout);
      //puts("========================");
      //fflush(NULL);
      i->draw(p/*, editor_.cur_subnetwork_*/);
    }
  }
  for(i=items_.last(); i; i=items_.prev()) {
    if((i->rtti() != ReactionEdgeItem::RTTI &&
	i->rtti() != InternalNodeItem::RTTI) &&
       isItemOnLayer(i)) {
      //i->save(stdout);
      //puts("========================");
      //fflush(NULL);
      i->draw(p/*, editor_.cur_subnetwork_*/);
    }
  }

  if(window_selection_) {
    p->setPen(QPen(black, 0, DotLine));
    p->drawRect(QRect(m_start_, m_cur_));
  }
}

bool
PathwayEditor::collision(MolRxnBaseItem *p) {
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(i->rtti() != ReactionEdgeItem::RTTI && isItemOnLayer(i)) {
      if(p->collidesWith(i)) {
	emit statusMessage("Collision - Please place item elsewhere", 2000);
	return true;
      }
    }
  }
  return false;
}

void
PathwayEditor::insertItem(const QPoint &pt) {
  //printf("New %d %d\n", pt.x(), pt.y());
  MolRxnBaseItem *p = NULL;
  switch(lb_com_) {
  case 0:
    p = new MassActionReactionItem(viewport());
    break;
  case 2:
    p = new DNAMoleculeItem(viewport());
    break;
  case 3:
    p = new EnzymeMoleculeItem(viewport());
    break;
  case 4:
    p = new MetaboliteMoleculeItem(viewport());
    break;
  case 5:
    p = new EnvironmentMoleculeItem(viewport());
    break;
  case 6:
    p = new TranscriptionFactorMoleculeItem(viewport());
    break;
  case 7:
    p = new ComplexMoleculeItem(viewport());
    break;
  case 9:
    p = new SquareWaveControlVariableItem(viewport());
    break;
  }
  if(p) {
    if(p->rtti() == MoleculeItem::RTTI) {
      if(p->sub_rtti() == ControlVariableItem::sub_RTTI) {
	p->setLabel(nextControlVariableName());
      } else {
	p->setLabel(nextMoleculeName());
      }
    } else if(p->rtti() == ReactionItem::RTTI) {
      p->setLabel(nextReactionName());
    }
    if(editor_.use_grid_) {
      p->moveTo((pt.x()/editor_.gsx_)*editor_.gsx_,
		(pt.y()/editor_.gsy_)*editor_.gsy_);
    } else {
      p->moveTo(pt.x(), pt.y());
    }

    if(collision(p)) {
      delete p;
    } else {
      items_.append(p);
      viewport()->update();
    }
  }
}

void
PathwayEditor::unselectAll() {
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    i->setSelected(false);
  }
}

void
PathwayEditor::selectAll() {
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(isItemOnLayer(i)) {
      i->setSelected(true);
    }
  }
}

void
PathwayEditor::contentsMousePressEvent(QMouseEvent *e) {
  //printf("press event  %d %d %d\n", single_selection_, window_selection_, dragging_);

  if(layout_timer_->isActive()) {
    layout_timer_->stop();
    layout_was_active_ = true;
  }
  
  if(e->pos().x() < contentsX() || contentsWidth()  <= e->pos().x() ||
     e->pos().y() < contentsY() || contentsHeight() <= e->pos().y()) {
    return;
  }

  //pushUndo();

  PathwayCanvasItem *i;

  if(e->button() == Qt::LeftButton) {
    if(edge_) return;
    if(lb_com_ != -1) {
      //insertItem(e->pos());
      //emit topologyChanged();
    }
    if(lb_com_ == -1) {
      bool found = false;
      for(i=items_.first(); i; i=items_.next()) {
	if(i->contains(e->pos()) && isItemOnLayer(i)) {
	  found = true;
	  break;
	}
      }
      
      if(found) {
	if(!i->selected()) {
	  if(!(e->state() & ShiftButton)) unselectAll();
	  //i->setSelected(true);
	  emit changed();
	}	      
	m_start_ = m_cur_ = m_prev_ = e->pos();
	single_selection_ = true;
	dragging_ = true;
	return;
      } else {
	if(!(e->state() & ShiftButton)) {
	  unselectAll();
	  emit changed();
	}	
	m_start_ = m_cur_ = m_prev_ = e->pos();
	window_selection_ = true;
	return;
      }
    }
  }
}

//----------------------------------------------------------------------

void
PathwayEditor::contentsMouseMoveEvent(QMouseEvent *e) {
  //printf("move event  %d %d %d %p\n", single_selection_, window_selection_, dragging_, edge_);

  if(edge_) {
    int x = e->pos().x();
    int y = e->pos().y();
    if(editor_.use_grid_) {
      x = (x / editor_.gsx_) * editor_.gsx_;
      y = (y / editor_.gsy_) * editor_.gsy_;
    }
    QRect bb = moving_->boundingRect();
    int min_x = bb.width()/2;
    int min_y = bb.height()/2;
    int max_x = contentsWidth() - min_x;
    int max_y = contentsHeight() - min_y;
    if(x < min_x) {
      x = min_x;
    } else if(x > max_x) {
      x = max_x;
    }
    if(y < min_y) {
      y = min_y;
    } else if(y > max_y) {
      y = max_y;
    }

    moving_->moveTo(x, y);
    ensureVisible(x, y, 10, 10);
    viewport()->update();
    return;
  } else {
    if(e->state() & Qt::LeftButton) {
      if(window_selection_) {
	m_cur_ = e->pos();
	viewport()->update();
      } else {
	if(dragging_) {
	  if(single_selection_) {
	    bool found = false;
	    PathwayCanvasItem *i;
	    for(i=items_.first(); i; i=items_.next()) {
	      if(i->rtti() != ReactionEdgeItem::RTTI &&
		 i->contains(m_start_) &&
		 isItemOnLayer(i)) {
		i->setSelected(true);
		found = true;
		break;
	      }
	    }
	    if(!found) {
	      for(i=items_.first(); i; i=items_.next()) {
		if(i->rtti() == ReactionEdgeItem::RTTI &&
		   i->contains(m_start_) &&
		   isItemOnLayer(i)) {
		  i->setSelected(true);
		  found = true;
		  break;
		}
	      }
	    }
	    if(found) viewport()->update();
	    single_selection_ = false;
	  }
	  m_cur_ = e->pos();

	  int dx = m_cur_.x() - m_prev_.x();
	  int dy = m_cur_.y() - m_prev_.y();
	  if(editor_.use_grid_ && !(e->state() & ControlButton)) {
	    dx = (dx / editor_.gsx_) * editor_.gsx_;
	    dy = (dy / editor_.gsy_) * editor_.gsy_;
	  }

	  if(dx != 0 || dy != 0) {
	    moveSelected(dx, dy);
	    m_prev_ += QPoint(dx, dy);
	  }
	} else {
	}
      }
    } else {
    }
    return;
  }
}

//----------------------------------------------------------------------

void
PathwayEditor::contentsMouseReleaseEvent(QMouseEvent *e) {
  //printf("release event  %d %d %d\n", single_selection_, window_selection_, dragging_);

  if(e->state() & Qt::LeftButton) {
    if(single_selection_) {
      if(m_start_ == e->pos()) {
	bool found = false;
	PathwayCanvasItem *i;
	for(i=items_.first(); i; i=items_.next()) {
	  if(i->rtti() != ReactionEdgeItem::RTTI &&
	     i->contains(m_start_) &&
	     isItemOnLayer(i)) {
	    found = true;
	    break;
	  }
	}
	if(!found) {
	  for(i=items_.first(); i; i=items_.next()) {
	    if(i->rtti() == ReactionEdgeItem::RTTI &&
	       i->contains(m_start_) &&
	       isItemOnLayer(i)) {
	      break;
	    }
	  }
	}

	if(i) {
	  if(e->state() & ShiftButton) {
	    i->setSelected(!i->selected());
	  } else {
	    unselectAll();
	    i->setSelected(true);
	  }
	  viewport()->update();
	}
      }
    } else if(window_selection_) {
      m_cur_ = e->pos();
      QRect win(m_start_, m_cur_);
      win = win.normalize();
      PathwayCanvasItem *i;
      for(i=items_.first(); i; i=items_.next()) {
	if(win.contains(i->boundingRect()) && isItemOnLayer(i)) {
	  i->setSelected(true);
	}
      }
      viewport()->update();      
    }
  }
  
  single_selection_ = false;
  window_selection_ = false;
  dragging_ = false;
  
  if(!edge_) {
    moving_ = NULL;
  }

  if(layout_was_active_) {
    layout_was_active_ = false;
    layoutInit();
    layout_timer_->start(100);
  }  
}

void
PathwayEditor::contentsMouseDoubleClickEvent(QMouseEvent *e) {
#if 0
  puts("double click event");

  if(e->button() == Qt::LeftButton) {
    if(!edge_) {
      //pushUndo();
      
      PathwayCanvasItem *i;
      for(i=items_.first(); i; i=items_.next()) {
	if(i->contains(e->pos()) && isItemOnLayer(i)) {
	  if(i->rtti() == ReactionEdgeItem::RTTI) {
	    InternalNodeItem *p = new InternalNodeItem(viewport());
	    p->moveTo(e->pos().x(), e->pos().y());
	    ((ReactionEdgeItem*)i)->insertPoint(p);
	    items_.append(p);
	    unselectAll();
	    p->setSelected(true);
	    viewport()->update();
	    emit changed();
	  } else {
	    if(i->propertiesDialog(this, simulation_)) {
	      emit topologyChanged();
	    }
	  }
	  return;
	}
      }
    }
  }
#endif
}

void
PathwayEditor::closeEvent(QCloseEvent *e) {
  if(hasChanged()) {
    setFocus();
    showMaximized();
    switch(QMessageBox::warning(this, appname,
				"Current pathway is unsaved.\n"
				"Do you wish to save before quiting?\n",
				"Yes", "No", "Cancel", 0, 2)) {
    case 0:
      if(fileSave()) e->accept();
      else e->ignore();
      break;
    case 1:
      e->accept();
      break;
    case 2:
      e->ignore();
      break;
    }
  } else {
    e->accept();
  }
}

void
PathwayEditor::insertMenu(int id) {
  viewport()->setCursor(pointingHandCursor);
  lb_com_ = id;
  QString str;
  str.sprintf("Placing %s", menu_names[id]);
  emit statusMessage(str, 5000);
}

void
PathwayEditor::deleteEdge(ReactionEdgeItem *edge) {
  QList<MolRxnBaseItem> nodes = edge->nodes();
  QListIterator<MolRxnBaseItem> i(nodes);
  for(; i.current();) {
    MolRxnBaseItem *p = (MolRxnBaseItem*)i.current();
    if(p->rtti() == InternalNodeItem::RTTI) {
      edge->removeNode(p);
      nodes.remove(p);
      items_.remove(p);
    } else {
      ++i;
    }
  }
  items_.remove(edge);
}

#define UP 0x01
#define DOWN 0x02
#define LEFT 0x04
#define RIGHT 0x08

void
PathwayEditor::keyPressEvent(QKeyEvent *e) {
  switch(e->key()) {
  case Key_Up:
    if(e->state() & (ShiftButton | ControlButton)) {
      moveSelected(UP, bool(e->state() & ShiftButton));
    } else {
      scrollBy(0, -visibleHeight()/5);
    }
    break;
  case Key_Down:
    if(e->state() & (ShiftButton | ControlButton)) {
      moveSelected(DOWN, bool(e->state() & ShiftButton));
    } else {
      scrollBy(0, visibleHeight()/5);
    }
    break;
  case Key_Left:
    if(e->state() & (ShiftButton | ControlButton)) {
      moveSelected(LEFT, bool(e->state() & ShiftButton));
    } else {
      scrollBy(-visibleWidth()/5, 0);
    }
    break;
  case Key_Right:
    if(e->state() & (ShiftButton | ControlButton)) {
      moveSelected(RIGHT, bool(e->state() & ShiftButton));
    } else {
      scrollBy(visibleWidth()/5, 0);
    }
    break;
  case Key_PageUp:
    scrollBy(0, -visibleHeight());
    break;
  case Key_PageDown:
    scrollBy(0, visibleHeight());
    break;
  case Key_Escape:
    viewport()->unsetCursor();
    lb_com_ = -1;
    if(edge_) {
      deleteEdge(edge_);
      edge_ = NULL;
      viewport()->update();
      emit topologyChanged();
    }
    break;
#if 0
  case Key_Delete:
    //pushUndo();
    
    {
      QListIterator<PathwayCanvasItem> i(items_), j(items_);

      // Remove selected internal nodes, molecules, and rxns
      for(; i.current();) {
	if(i.current()->selected() &&
	   i.current()->rtti() != ReactionEdgeItem::RTTI) {
	  // Remove selected node from any edges
	  j.toFirst();
	  for(; j.current(); ++j) {
	    if(j.current()->rtti() == ReactionEdgeItem::RTTI) {
	      ((ReactionEdgeItem*)j.current())->removeNode((MolRxnBaseItem*)(i.current()));
	    }
	  }
	  // Actually delete selected node
	  items_.remove(i.current());
	} else {
	  ++i;
	}
      }

      // Remove selected and invalid edges (includes current edge_)
      i.toFirst();
      for(; i.current();) {
	ReactionEdgeItem *p = (ReactionEdgeItem*)i.current();
	if(p->rtti() == ReactionEdgeItem::RTTI &&
	   (p->selected() || !p->molecule() || !p->reaction())) {
	  deleteEdge(p);
	} else {
	  ++i;
	}
      }
      if(edge_) viewport()->unsetCursor();
      edge_ = NULL;
      moving_ = NULL;
      viewport()->update();
      emit topologyChanged();
    }
    break;
#endif
  default:
    e->ignore();
  }  
}

void
PathwayEditor::tip(QPoint p, QRect &r, QString &str) const {
  p += QPoint(contentsX(), contentsY());
  
  QListIterator<PathwayCanvasItem> i(items_);
  for(; i.current(); ++i) {
    if((i.current()->rtti() == MoleculeItem::RTTI ||
	i.current()->rtti() == ReactionItem::RTTI) &&
       i.current()->contains(p) &&
       isItemOnLayer(i.current())) {
      r = ((MolRxnBaseItem*)i.current())->boundingRect();
      r.moveBy(-contentsX(), -contentsY());
      str = ((MolRxnBaseItem*)i.current())->tip();
    }
  }
}

void
PathwayEditor::pushUndo() {
  if(undo_editor_.current() != undo_editor_.last()) {
    puts("Clean off redo");
    // Clean off redo list
  }
  undo_editor_.append(new Editor(editor_));
  undo_simulation_.append(new Simulation(simulation_));
  undo_items_.append(new QList<PathwayCanvasItem>(items_));
}

void
PathwayEditor::popUndo() {
  if(undo_editor_.count() > 0) {
    puts("Popping..");
    editor_ = *undo_editor_.last();
    simulation_ = *undo_simulation_.last();
    items_ = *undo_items_.last();

    undo_editor_.prev();
    undo_simulation_.prev();
    undo_items_.prev();
    
    //undo_editor_.removeLast();
    //undo_simulation_.removeLast();
    //undo_items_.removeLast();
  }
}

void
PathwayEditor::moveSelected(int dir, bool big_step) {
  int dx=1, dy=1;

  if(big_step) {
    dx = editor_.gsx_;
    dy = editor_.gsy_;
  }

  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(i->selected() &&
       (i->rtti() == MoleculeItem::RTTI ||
	i->rtti() == ReactionItem::RTTI ||
	i->rtti() == InternalNodeItem::RTTI)) {
      MolRxnBaseItem *p = (MolRxnBaseItem*)i;
      //if(i->rtti() == MoleculeItem::RTTI || i->rtti() == ReactionItem::RTTI) printf("Moved1: %s\n", (const char*)((MolRxnBaseItem*)i)->label());
      if(dir & UP)    p->move(0, -dy);
      if(dir & DOWN)  p->move(0, +dy);
      if(dir & LEFT)  p->move(-dx, 0);
      if(dir & RIGHT) p->move(+dx, 0);
    }
  }
  viewport()->update();
}

void
PathwayEditor::moveSelected(int dx, int dy) {
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(i->selected() &&
       (i->rtti() == MoleculeItem::RTTI ||
	i->rtti() == ReactionItem::RTTI ||
	i->rtti() == InternalNodeItem::RTTI)) {
      MolRxnBaseItem *p = (MolRxnBaseItem*)i;
      //if(i->rtti() == MoleculeItem::RTTI || i->rtti() == ReactionItem::RTTI) printf("Moved2: %s\n", (const char*)((MolRxnBaseItem*)i)->label());
      p->move(dx, dy);
    }
  }
  viewport()->update();
}

void
PathwayEditor::layoutGo(bool s) {
  if(s && !layout_timer_->isActive()) {
    layoutInit();
    layout_timer_->start(100);
  } else {
    layout_timer_->stop();
  } 
}

void
PathwayEditor::layoutRandom() {
  int w = contentsWidth();
  int h = contentsHeight();
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(i->rtti() == MoleculeItem::RTTI ||
       i->rtti() == ReactionItem::RTTI ||
       i->rtti() == InternalNodeItem::RTTI) {
      MolRxnBaseItem *p = (MolRxnBaseItem*)i;
      int xi = random()%(w-20);
      int yi = random()%(h-15);
      p->moveTo(xi, yi);
    }
  }
  if(layout_timer_->isActive()) layoutInit();
  viewport()->update();
}

void
PathwayEditor::layoutInit() {
  int ni = 0, j, k;
  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    if(i->rtti() == MoleculeItem::RTTI ||
       i->rtti() == ReactionItem::RTTI ||
       i->rtti() == InternalNodeItem::RTTI) {
      i->setId(ni++);
    }
  }

  x_.resize(ni),  y_.resize(ni);
  dx_.resize(ni),  dy_.resize(ni);
  ddx_.resize(ni), ddy_.resize(ni);
  for(j=0; j<ni; j++) {
    dx_[j] = 0.0;
    dy_[j] = 0.0;
  }

  int w = contentsWidth();
  int h = contentsHeight();
  for(i=items_.first(), j=0; i; i=items_.next()) {
    if(i->rtti() == MoleculeItem::RTTI ||
       i->rtti() == ReactionItem::RTTI ||
       i->rtti() == InternalNodeItem::RTTI) {
      MolRxnBaseItem *p = (MolRxnBaseItem*)i;
      x_[j] = p->x()/(double)w;
      y_[j] = p->y()/(double)h;
      j++;
    }
  }

  em_.resize(ni * ni);
  for(j=0; j<ni-1; j++) {
    for(k=j+1; k<ni; k++) {
      em_[j*ni + k] = -1;
    }
  }

  int ne = 0;
  for(i=items_.first(), j=0; i; i=items_.next()) {
    if(i->rtti() == ReactionEdgeItem::RTTI) {
      QList<MolRxnBaseItem> nodes = ((ReactionEdgeItem*)i)->nodes();
      
      PathwayCanvasItem *p, *q;
      for(p=nodes.first(), q=nodes.next(); q; q=nodes.next()) {
	if(p->id() <= q->id()) {
	  em_[p->id() * ni + q->id()] = ne++;
	} else {
	  em_[q->id() * ni + p->id()] = ne++;
	}
      }
    }
  }
}

void
PathwayEditor::layoutStep() {
  int w = contentsWidth();
  int h = contentsHeight();
  int i, j;

  const double cc = 0.0001;
  const double dampening = 0.85;
  const double nsl = 100.0 / (w > h ? w : h); // normalized spring length
  const double marginX = 20.0 / w; 
  const double marginY = 15.0 / h;  

  double rv_ = 0.25;
  double hc_ = 0.11;

  int n_ = x_.count();
  
  for(i=0; i<n_; i++) {
    ddx_[i] = 0.0;
    ddy_[i] = 0.0;
  }

  for(i=0; i<n_-1; i++) {
    for(j=i+1; j<n_; j++) {
      double dx = x_[i] - x_[j];
      double dx1 = dx;
      double dx2 = x_[i] + 1.0 - x_[j];
      double dx3 = -(1.0 - x_[i]) - x_[j];
      if(fabs(dx) > fabs(dx2)) dx = dx2;
      if(fabs(dx) > fabs(dx3)) dx = dx3;

      double dy = y_[i] - y_[j];
      double dy1 = dy;
      double dy2 = y_[i] + 1.0 - y_[j];
      double dy3 = -(1.0 - y_[i]) - y_[j];
      if(fabs(dy) > fabs(dy2)) dy = dy2;
      if(fabs(dy) > fabs(dy3)) dy = dy3;
      
      double ds = dx*dx + dy*dy;
      if(ds < 1e-6) ds = 1e-6;
      
      double fm_r = rv_ * cc / ds;

      double fm_s;
      if(em_[i * n_ + j] != -1) {
	double ds_s = dx1*dx1 + dy1*dy1;
	if(ds_s < 1e-6) ds_s = 1e-6;
	double dt = sqrt(ds_s);
	fm_s = hc_ * (nsl - dt);
      } else {
	fm_s = 0.0;
      }
      
      double dcx = dx * fm_r + dx1 * fm_s;
      ddx_[i] += dcx;
      ddx_[j] -= dcx;
      double dcy = dy * fm_r + dy1 * fm_s;
      ddy_[i] += dcy;
      ddy_[j] -= dcy;
    }
  }

  for(i=0; i<n_; i++) {
    dx_[i] = dx_[i] * dampening + ddx_[i];
    dy_[i] = dy_[i] * dampening + ddy_[i];
    
    if(dx_[i] > .125) dx_[i] = .125;
    else if(dx_[i] < -.125) dx_[i] = -.125;
    if(dy_[i] > .125) dy_[i] = .125;
    else if(dy_[i] < -.125) dy_[i] = -.125;

    x_[i] = x_[i] + dx_[i];
    if(x_[i] < 0.0) {
      x_[i] = 0.0;
      dx_[i] = fabs(dx_[i]);
    } else if(x_[i] > 1.0 - marginX) {
      x_[i] = 1.0 - marginX;
      dx_[i] = -fabs(dx_[i]);
    }
    y_[i] = y_[i] + dy_[i];
    if(y_[i] < 0.0) {
      y_[i] = 0.0;
      dy_[i] = fabs(dy_[i]);
    } else if(y_[i] > 1.0 - marginY) {
      y_[i] = 1.0 - marginY;
      dy_[i] = -fabs(dy_[i]);
    }
  }

  {
    int j;
    PathwayCanvasItem *i;
    for(i=items_.first(), j=0; i; i=items_.next()) {
      if(i->rtti() == MoleculeItem::RTTI ||
	 i->rtti() == ReactionItem::RTTI ||
	 i->rtti() == InternalNodeItem::RTTI) {
	MolRxnBaseItem *p = (MolRxnBaseItem*)i;
	p->moveTo(int(x_[j]*w+0.5), int(y_[j]*h+0.5));
	j++;
      }
    }
  }
  
#if 1
  viewport()->update();
  //updateContents(lx1_, ly1_, lx2_ - lx1_ + 1, ly2_ - ly1_ + 1);
  //viewport()->update(lx1_, ly1_, lx2_ - lx1_ + 1, ly2_ - ly1_ + 1);
#else
  {
    static int count = 0;
    if((count % 10) == 0) {
      canvas()->update();
    }
    count++;
    printf("%d\n", count);
  }
#endif
  //static int count = 0;
  //count++;
  //printf("%d\n", count);
}

void
PathwayEditor::evolveGo(bool s) {
  evolve_thread_->start();
}

void
PathwayEditor::evolveGetBest() {
  int gen;
  double fitness;
  Matrix stats;
  evolve_thread_->best(items_, simulation_, gen, fitness, stats);

  int nr = evolve_stats_.nrows();
  int nc = stats.ncolumns() + 1;
  {
    int i, j;
    Matrix tmp;
    tmp.resize(nr+1, nc);
    for(i=0; i<nr; i++) {
      for(j=0; j<nc; j++) {
	tmp(i, j) = evolve_stats_(i, j);
      }
    }
    tmp(nr, 0) = gen;
    for(j=1; j<nc; j++) {
      tmp(nr, j) = stats(0, j-1);
    }
    evolve_stats_ = tmp;
  }

  PathwayCanvasItem *i;
  for(i=items_.first(); i; i=items_.next()) {
    i->setParent(viewport());
  }
  layoutRandom();
  if(layout_timer_->isActive()) layoutInit();
  viewport()->update();
  runSimulation();
  //viewReport();
  if(!cd_ || cd_->isVisible()) displayConcentrationDialog();
  if(!evolve_traces_ || evolve_traces_->isVisible()) displayEvolveTracesDialog();
  QString str = tr("Generation %1, fitness %2").arg(gen).arg(fitness);
  emit statusMessage(str, 30000);
}

