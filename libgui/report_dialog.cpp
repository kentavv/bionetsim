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

#include <qdialog.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextedit.h>

#include "calculate.h"
#include "line_graph_view.h"
#include "pathway_canvas_items.h"
#include "pathway_canvas_utils.h"
#include "pathway_matrix.h"
#include "pathway_reports.h"
#include "report_dialog.h"
#include "result.h"
#include "simulation.h"

ReportDialog::ReportDialog(QWidget *parent, const char *name, bool modal)
  : QDialog(parent, name, modal, Qt::WStyle_MinMax)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);
  
  setMinimumSize(400, 200);
  setCaption("Report Dialog");

  text_ = new QTextEdit(this);
  text_->setReadOnly(true);
  lg_ = new LineGraphView(this);
  lg_->setFixedHeight(180);
  
  QHBox *hbox = new QHBox(this);
  QPushButton *save = new QPushButton("Save", hbox);
  connect(save, SIGNAL(clicked()), SLOT(save()));

  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

void
ReportDialog::setup(const Simulation &sim, const QList<PathwayCanvasItem> &items) {
  QString text;
  PathwayCanvasItem *i;
  QListIterator<PathwayCanvasItem> it(items);
  int nmol=0, nrxn=0;
  
  text += sim.genReport();

  for(; it.current(); ++it) {
    i = it.current();
    if(isMolecule(i)) {
      nmol++;
      if(((MoleculeItem*)i)->decays()) {
	nrxn++;
      }
    } else if(isReaction(i)) {
      nrxn++;
      if(((ReactionItem*)i)->reversible()) {
	nrxn++;
      }
    } else if(isSubnetwork(i)) {
      nmol += ((SubnetworkItem*)i)->numMolecules() - ((SubnetworkItem*)i)->numBoundMolecules();
      nrxn += ((SubnetworkItem*)i)->numReactions();
    }
  }
  
  text += tr("\n\nMolecules (%1):\n").arg(nmol);

  buildMoleculeList(items, text, 2);
  
  text += "\n\n";
  text += tr("Reactions (%1):\n").arg(nrxn);

  buildReactionList(items, text, 2);

  text += "\n\n";
  text += tr("Equations (%1):\n").arg(nmol);

  buildMoleculeEquations(items, text, 2);

  text += "\n\n";
  text += tr("Reduced CVODE Equations (%1):\n").arg(nmol);

  buildMoleculeReducedEquationsCVODE(items, text, 0);

  text += "\n\n";
  text += tr("Reduced Mathematica Equations (%1):\n").arg(nmol);

  buildMoleculeReducedEquationsMathematica(items, text, 0);

  text_->setText(text);

  buildControlVariableGraph(sim, items);
}

void
ReportDialog::buildControlVariableGraph(const Simulation &sim, const QList<PathwayCanvasItem> &items) {
  QList<PathwayCanvasItem> cvs;
  QStringList labels;
  cvs.setAutoDelete(false);

  buildControlVariableList(items, "", cvs, labels);
  
  Editor editor;
  PathwayMatrix pem;
  pem.setup(editor, sim, cvs);
  pem.simulation.setSimulationMethod(0);
  
  Result results;
  if(run_simulation(pem, results)) {
    lg_->setData(results);
    lg_->setTitle("Control Variables");
    lg_->setAllVisible(true);
  }
}

void
ReportDialog::save() {
  QString s(QFileDialog::getSaveFileName(QString::null, "Text Files (*.txt);;All Files (*)", NULL));
  if(!s.isEmpty()) {
    FILE *file = fopen((const char *)s, "w");
    if(!file) {
      QMessageBox::warning(NULL, "Report Dialog", "Unable to open file " + s);
      return;
    }
    fprintf(file, "%s\n", (const char*)text_->text());
    fclose(file);
  }
}
