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
#include "version.h"

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
  "Square Wave Control Variable",
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
    dd_(NULL)
{
  resizeContents(editor_.cw_, editor_.ch_);

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

  pet_ = new PETip(this);
}

PathwayEditor::~PathwayEditor() {
  delete pet_;
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

bool
PathwayEditor::open(const QString &filename) {
  if(!pathwayLoad((const char *)filename, editor_,
		  simulation_, items_, version_, viewport())) {
    return false;
  }

  changed_ = false;
  filename_ = filename;

  setCaption(filename);
  resizeContents(editor_.cw_, editor_.ch_);
  
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
  editor_.cw_ = contentsWidth();
  editor_.ch_ = contentsHeight();
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
    resizeContents(editor_.cw_, editor_.ch_);
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

#include "scaled_matrix_view.h"

extern "C" bool run_gal4_simulation(const Matrix &kc, Matrix &results);

#ifndef WIN32
#include <dlfcn.h>
#endif

void
PathwayEditor::displayParameterScanDialog() {
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
#if 1
	if(run_simulation(pem, results)) {
	  fitness = calculate_fitness(pem, results);
	}
#else
	if(run_gal4_simulation(pem.kc, results)) {
	  fitness = calculate_fitness(pem, results);
	}
#endif
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

#if 1
#if 1
  qt2.start();
  if(run_simulation(pem, results_)) {
    fitness = calculate_fitness(pem, results_);
  }
#else
  qt2.start();
  if(run_gal4_simulation(pem.kc, results_)) {
    fitness = calculate_fitness(pem, results_);
  }
#endif
#else
  void *handle = dlopen("/home/kent/projects/pathway_editor/test.so", RTLD_NOW);
  if(!handle) {
    fprintf(stderr, "%s\n", dlerror());
    abort();
  }
  void *func = dlsym(handle, "run_gal4_simulation");
  const char *error = dlerror();
  if(error) {
    fprintf(stderr, "%s\n", error);
    abort();
  }
  qt2.start();
  if(((bool (*)(const Matrix &, Matrix &))func)(pem.kc, results_)) {
    fitness = calculate_fitness(pem, results_);
  }
#endif
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
    p->moveTo(pt.x(), pt.y());

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
  
  if(e->pos().x() < contentsX() || contentsWidth()  <= e->pos().x() ||
     e->pos().y() < contentsY() || contentsHeight() <= e->pos().y()) {
    return;
  }

  //pushUndo();

  PathwayCanvasItem *i;

  if(e->button() == Qt::LeftButton) {
    if(edge_) return;
    if(lb_com_ != -1) {
      insertItem(e->pos());
      emit topologyChanged();
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
  if(e->button() == Qt::RightButton) {
    if(edge_) {
      for(i=items_.first(); i; i=items_.next()) {
	if((i->rtti() == MoleculeItem::RTTI && edge_->reaction() ||
	    i->rtti() == ReactionItem::RTTI && edge_->molecule() &&
	    ((ReactionItem*)i)->numEdges() == 0) &&
	   i->contains(e->pos()) &&
	   isItemOnLayer(i)) {
	  viewport()->unsetCursor();
	  edge_->addPoint((MolRxnBaseItem*)i);
	  edge_ = NULL;
	  emit topologyChanged();
	  break;
	}
      }
      
      if(!i && !collision(moving_)) {
	items_.append(moving_);
	moving_ = edge_->addPoint(moving_);
	moving_->moveTo(e->pos().x(), e->pos().y());
	emit changed();
      }
    } else {
      lb_com_ = -1;
      viewport()->unsetCursor();
      moving_ = NULL;
      for(i=items_.first(); i; i=items_.next()) {
	if((i->rtti() == MoleculeItem::RTTI ||
	    (i->rtti() == ReactionItem::RTTI)) &&
	   i->contains(e->pos()) &&
	   isItemOnLayer(i)) {
	  viewport()->setCursor(crossCursor);
	  edge_ = new ReactionEdgeItem(viewport());
	  moving_ = edge_->addPoint((MolRxnBaseItem*)i);
	  moving_->moveTo(e->pos().x(), e->pos().y());
	  items_.append(edge_);
	  emit topologyChanged();
	  break;
	}
      }
      
      if(!i) {
	viewport()->unsetCursor();
	insert_menu_->popup(QCursor::pos());
      }
    }
    viewport()->update();
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
}

void
PathwayEditor::contentsMouseDoubleClickEvent(QMouseEvent *e) {
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
	    if(i->propertiesDialog(simulation_)) {
	      emit topologyChanged();
	    }
	  }
	  return;
	}
      }
    }
  }
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
