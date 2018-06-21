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

#include <qcheckbox.h>
#include <qdialog.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qmultilineedit.h> // Use QTextEdit when Qt 3 is available
#include <qpushbutton.h>
#include <qtable.h>

#include "calculate.h"
#include "interpolated_cv_dialog.h"
#include "line_graph_view.h"
#include "matrix.h"
#include "pathway_canvas_items.h"
#include "version.h"

InterpolatedControlVariableDialog::InterpolatedControlVariableDialog(QWidget *parent,
								     const Simulation &simulation,
								     InterpolatedControlVariableItem &cv)
  : QDialog(parent, NULL, false, Qt::WStyle_MinMax),
    cv_(cv),
    simulation_(simulation)
{
  QVBoxLayout *vbox = new QVBoxLayout(this, 8);
  vbox->setAutoAdd(true);

  QHBox *hbox;

  hbox = new QHBox(this);
  new QLabel("Control Variable Label: ", hbox);
  label_ = new QLineEdit(hbox);

  
  exported_ = new QCheckBox("Exported", this);
  plotted_ = new QCheckBox("Plotted", this);

  QGroupBox *group;

  group = new QGroupBox(2, Vertical, "Parameters", this);

  values_ = new QTable(group);

  QPushButton *import = new QPushButton("Import", group);
  
  lg_ = new LineGraphView(this);
  lg_->setFixedHeight(200);
  
  new QLabel("Notes:", this);
  notes_ = new QMultiLineEdit(this);

  hbox = new QHBox(this);
  QPushButton *cancel = new QPushButton("Cancel", hbox);
  apply_ = new QPushButton("Apply", hbox);
  QPushButton *ok = new QPushButton("OK", hbox);
  ok->setDefault(true);
  apply_->setEnabled(false);

  updateDialog(simulation);
  
  connect(label_, SIGNAL(textChanged(const QString &)), SLOT(setChanged()));
  connect(exported_, SIGNAL(clicked()), SLOT(setChanged()));
  connect(plotted_,  SIGNAL(clicked()), SLOT(setTopologyChanged()));
  connect(values_,   SIGNAL(valueChanged(int, int)), SLOT(setTopologyChanged()));
  connect(notes_,    SIGNAL(textChanged()), SLOT(setChanged()));

  connect(label_,           SIGNAL(returnPressed()), SLOT(apply()));
  connect(apply_, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(apply()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  connect(import, SIGNAL(clicked()), SLOT(import()));
  
  connect(values_,     SIGNAL(valueChanged(int, int)), SLOT(updateGraph()));

  updateGraph();
}

InterpolatedControlVariableDialog::~InterpolatedControlVariableDialog() {
}

void
InterpolatedControlVariableDialog::updateItem() {
  cv_.setLabel(label_->text());
  cv_.setExported(exported_->isChecked());
  cv_.setPlotted(plotted_->isChecked());
  
  int nr = values_->numRows();
  int nc = values_->numCols();
  Matrix values(nr, nc);
  for(int i=0; i<nr; i++) {
    for(int j=0; j<nc; j++) {
      values(i, j) = values_->text(i, j).toDouble();
    }
  }
  
  cv_.setValues(values);
  
  cv_.setNotes(notes_->text());
}

void
InterpolatedControlVariableDialog::updateDialog(const Simulation &simulation) {
  simulation_ = simulation;

  setCaption(cv_.caption());
  label_->setText(cv_.label());
  
  exported_->setChecked(cv_.exported());
  plotted_->setChecked(cv_.plotted());

  Matrix values;
  cv_.values(values);
  setValues(values);

  notes_->setText(cv_.notes());

  updateGraph();
}

void
InterpolatedControlVariableDialog::import() {
  QString fn = QFileDialog::getOpenFileName(QString::null,
					    "Text (*.txt)",
					    this,
					    "Import data"
					    "Import data" );

  if(!fn.isNull()) {
    Matrix values;
    FILE *fp = fopen((const char*)fn, "r");
    if(!fp) {
      QMessageBox::critical(this, appname, "Unable to load " + fn, QMessageBox::Ok, QMessageBox::NoButton);
      return;
    }
    values.load(fp);
    // Ensure that values matrix does not containue fewer (or nore) environments
    // than needed by the simulation
    values.resize(values.nrows(), simulation_.numEnvironment()+1, 0.0, true);
    fclose(fp);
    setValues(values);
    updateGraph();
  }
}

void
InterpolatedControlVariableDialog::setValues(const Matrix &values) {
  int nr = values.nrows();
  int nc = values.ncolumns();

  //values_->clear();
  values_->setNumRows(nr);
  values_->setNumCols(nc);
  
  for(int i=0; i<nr; i++) {
    for(int j=0; j<nc; j++) {
      QString str;
      str.setNum(values(i, j));
      values_->setText(i, j, str);
    }
  }

  QStringList env_desc = simulation_.environmentDescriptions();

  values_->horizontalHeader()->setLabel(0, "Time");
  for(int j=1; j<nc; j++) {
    values_->horizontalHeader()->setLabel(j, env_desc[j-1]);
  }

  setTopologyChanged();
}

void
InterpolatedControlVariableDialog::updateGraph() {
  simulation_.setCurEnvironment(0);

  int nr = values_->numRows();
  Matrix values(nr, 2);

  for(int i=0; i<nr; i++) {
    values(i, 0) = values_->text(i, 0).toDouble();
  }

  QList<PathwayCanvasItem> items;
  items.setAutoDelete(true);

  int ne = simulation_.numEnvironment();
  for(int j=0; j<ne; j++) {
    for(int i=0; i<nr; i++) {
      values(i, 1) = values_->text(i, j+1).toDouble();
    }

    InterpolatedControlVariableItem *cv = new InterpolatedControlVariableItem(NULL);
    cv->setValues(values);
    items.append(cv);
  }

  Editor editor;
  PathwayMatrix pem;
  pem.setup(editor, simulation_, items);
  pem.simulation.setSimulationMethod(0);
  
  Result result;
  if(run_simulation(pem, result)) {
    lg_->setData(result);
    lg_->setGraphVisible(0, true);
  }
}

void
InterpolatedControlVariableDialog::apply() {
  if(topology_changed_) {
    cv_.updatePropertiesFromDialog();
    emit topologyChanged();
  } else if(changed_) {
    cv_.updatePropertiesFromDialog();
    emit changed();
  }
  changed_ = false;
  topology_changed_ = false;
  apply_->setEnabled(false);
}

void
InterpolatedControlVariableDialog::setChanged() {
  changed_ = true;
  apply_->setEnabled(true);
}

void
InterpolatedControlVariableDialog::setTopologyChanged() {
  topology_changed_ = true;
  apply_->setEnabled(true);
}
